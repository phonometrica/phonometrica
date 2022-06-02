// This is a ported version of the matlab code for the YAAPT algorithm.
// See http://www.ws.binghamton.edu/zahorian/yaapt.htm
// C++ translation by Julien Eychenne <jeychenne@gmail.com>

#include <fftw3.h>
#include <phon/analysis/yaapt.hpp>
#include <phon/analysis/signal_processing.hpp>
#include <phon/analysis/statistics.hpp>
#include <phon/third_party/FIR-filter-class/filt.h>

#if 0

namespace phonometrica {

static intptr_t fix(double x)
{
	return intptr_t((x < 0) ? std::ceil(x) : std::floor(x));
}


//---------------------------------------------------------------------------------------------------------------------

static
double
nonlinear(const Array<double> &DataA, double Fs, const PitchOptions &Prm, Array<double> &DataB, Array<double> &DataC, Array<double> &DataD)
{
	//NONLINEAR Create the nonlinear processed signal
	//
	//   [DataB, DataC, DataD, newFs] = nonlinear(DataA, Fs, Prm) creates
	//   nonlinear signal (squared signal) and applys filtering.
	//
	// INPUTS:
	//   DataA: The original signal from read_audio.m
	//   Fs:    The sampling rate for the signal
	//   Prm:   Array of parameters used to control performance
	//
	// OUTPUTS:
	//   DataB: The original signal, bandpass filtered with F1.
	//   DataC: The nonlinear signal, no filtering
	//   DataD: The nonlinear signal, bandpass filtered with F1.
	//   newFs: The sampling rate of the new signal

	//   Creation date:  Jun. 30, 2006
	//   Programers:     Hongbing Hu, Princy, Zahorian

	////////////////////////////////////////////////////////////////////////////////
	//     This file is a part of the YAAPT program, designed for a fundamental
	//   frequency tracking algorithm that is extremely robust for both high quality
	//   and telephone speech.
	//     The YAAPT program was created by the Speech Communication Laboratory of
	//   the state university of New York at Binghamton. The program is available
	//   at http://www.ws.binghamton.edu/zahorian as free software. Further
	//   information about the program could be found at "A spectral/temporal
	//   method for robust fundamental frequency tracking," J.Acosut.Soc.Am. 123(6),
	//   June 2008.
	////////////////////////////////////////////////////////////////////////////////

	const double Fs_min = 1000;    // Do not decimate if Fs less than this

	//  Parameters for filtering original signal, with a broader band
	auto Filter_order = Prm.bp_forder;
	double F_hp = Prm.bp_low;
	double F_lp = Prm.bp_high;

	int dec_factor = (Fs > Fs_min) ? Prm.dec_factor : 1;

	// Creates the bandpass filters
	auto lenDataA = DataA.size();

	// filter F1
	double w1 = (F_hp / (Fs / 2));
	double w2 = (F_lp / (Fs / 2));
	Filter filter(BPF, Filter_order, Fs, w1, w2);

	//filtering the original data with the bandpass filter,
	// DataA   is original acoustic signal
	Array<double> tempData;
	tempData.reserve(DataA.size());
	for (auto sample : DataA)
	{
		tempData.append(filter.do_sample(sample));
	}

	// Original signal filtered with F1
	for (intptr_t i = 1; i <= lenDataA; i += dec_factor)
	{
		DataB.append(tempData[i]);
	}

	//   Create nonlinear version of signal
	//   Squared value of the signal
	DataC = Array<double>(lenDataA, 0.0);
	for (intptr_t i = 1; i <= lenDataA; i++)
	{
		auto value = DataA[i];
		DataC[i] = value * value;
	}


	//   Nonlinear version filtered with F1
	tempData.clear();
	for (auto sample : DataC)
	{
		tempData.append(filter.do_sample(sample));
	}
	auto LenData_dec = fix(double(lenDataA + dec_factor - 1) / dec_factor);
	DataD.reserve(LenData_dec);
	for (intptr_t i = 1; i <= lenDataA; i += dec_factor)
	{
		DataD.append(tempData[i]);
	}
//	DataD = tempData(1:dec_factor:lenDataA);

	auto newFs = Fs / dec_factor;

	return newFs;
}


//---------------------------------------------------------------------------------------------------------------------

static
void nlfer(const Array<double> &Data, double Fs, const PitchOptions &Prm, Array<double> &Energy, Array<bool> &VUVEnergy)
{
	// NLFER  Normalized Low Frequency Energy Ratio
	//
	//   [Energy, VUVEnergy]= nlfer(Data, Fs, Prm) computes the
	//   nromlized low frequency energy ratio.
	//
	// INPUTS:
	//   Data:  Nonlinear, filtered signal for NLFER caculation
	//   Fs:    The sampling frequency.
	//   Prm:   Array of parameters used to control performance
	//
	// OUTPUTS:
	//   Energy:     NLFER Energy of the data
	//   VUVEnergy:  Voiced/Unvoiced decisions (optimum??)

	//   Creation date:  Oct 17, 2006, July 13, 2007
	//   Programers:     Hongbing Hu, S. Zahorian

	//////////////////////////////////////////////////////////////////////////////////
	//     This file is a part of the YAAPT program, designed for a fundamental
	//   frequency tracking algorithm that is extermely robust for both high quality
	//   and telephone speech.
	//     The YAAPT program was created by the Speech Communication Laboratory of
	//   the state university of New York at Binghamton. The program is available
	//   at http://www.ws.binghamton.edu/zahorian as free software. Further
	//   information about the program could be found at "A spectral/temporal
	//   method for robust fundamental frequency tracking," J.Acosut.Soc.Am. 123(6),
	//   June 2008.
	//////////////////////////////////////////////////////////////////////////////////

	//-- PARAMETERS: set up all of these parameters --------------------------------
	auto nfftlength = Prm.fft_length; // FFT length
	auto nframesize = fix(Prm.frame_length * Fs / 1000);
	auto nframejump = fix(Prm.frame_space * Fs / 1000);

	// If normalized low-frequency energy is below this, assume unvoiced frame
	auto nlfer_thersh1 = Prm.nlfer_thresh1;

	// Low frequency range for NLFER
	auto N_F0_min = intptr_t(round((Prm.f0_min * 2 / Fs) * nfftlength));
	auto N_F0_max = intptr_t(round((Prm.f0_max / Fs) * nfftlength));

	//-- MAIN ROUTINE --------------------------------------------------------------

	//  Spectrogram of the data
	auto SpecData = speech::specgram(Data, nfftlength, (nframesize - nframejump), nframesize);
	// Compute normalized low-frequency energy ratio
	Array<double> FrmEnergy(SpecData.ncol(), 0.0);
	double total_energy = 0.0;


	for (intptr_t j = 1; j <= SpecData.ncol(); j++)
	{
		double sum = 0.0;

		for (intptr_t i = N_F0_min; i <= N_F0_max; i++) {
			sum += std::abs(SpecData(i,j));
		}
		FrmEnergy[j] = sum;
	}
//	FrmEnergy = sum(abs(SpecData(N_F0_min:N_F0_max,:)));
	double avgEnergy = stats::mean(FrmEnergy);
	Energy = Array<double>(SpecData.ncol(), 0.0);

	for (intptr_t j = 1; j <= Energy.ncol(); j++) {
		Energy[j] = FrmEnergy[j] / avgEnergy;
	}

	// The frame is voiced if NLFER enery > threshold, otherwise is unvoiced.
	VUVEnergy = Array<bool> (Energy.ncol(), false);

	for (intptr_t j = 1; j <= Energy.ncol(); j++) {
		VUVEnergy[j] = (Energy[j] > nlfer_thersh1);
	}
}


//---------------------------------------------------------------------------------------------------------------------

static
void peaks(const Array<double> &Data, double delta, int maxpeaks, const PitchOptions &Prm, Array<double> &Pitch, Array<double> &Merit)
{

}


//---------------------------------------------------------------------------------------------------------------------

static
Array<double> get_minimum(const Array<double> &data, Array<intptr_t> &indexes)
{
	Array<double> result(data.ncol(), 0.0);
	indexes.resize(data.ncol());

	for (intptr_t j = 1; j <= data.ncol(); j++)
	{
		double value = data(1,j);
		intptr_t index = 1;

		for (intptr_t i = 2; i <= data.nrow(); i++)
		{
			auto cand = data(i,j);
			if (cand < value)
			{
				value = cand;
				index = i;
			}
		}
		result[j] = value;
		indexes[j] = index;
	}

	return result;
}

//---------------------------------------------------------------------------------------------------------------------

static
void spec_track(Array<double> &Data, double Fs, const Array<double> &VUVEnergy, const PitchOptions &Prm, Array<double> &SPitch,
				Array<double> &VUVSPitch, double pAvg, double pStd)
{
	//SPEC_TRK  Spectral pitch tracking for YAAPT pitch tracking
	//
	//   [SPitch, VUVSPitch, pAvg, pStd]=spec_trk(Data, Fs, VUVEnergy, Prm)
	//   computes estimates of pitch  using nonlinearly processed
	//   speech (typically square or absolute value) and frequency domain processing
	//   Search for frequencies which have energy at multiplies of that frequency
	//
	//INPUTS:
	//   Data:      Nonlinearly processed signal, filtered with F1. (~50 to 1500Hz)
	//   Fs:        The sampling frequency.
	//   VUVEnergy: Voiced/ unvoiced decision, depending on nlfer
	//   Prm:       Parameters
	//
	//OUTPUTS:
	//   SPitch:    The spectral Pitch track, with the unvoiced regions filled using interpolation.
	//   VUVSPitch: The spectral Pitch track, with  unvoiced regions set at zero
	//   pAvg:      The average of the Pitch track value.
	//   pStd:      The standard deviation in the track.

	//   Creation date:  Feb 20, 2006
	//   Revision dates: Feb 22, 2006,  March 11, 2006, April 5, 2006,
	//                   Jun 25, 2006,  June 13, 2007
	//   Programers:     Hongbing Hu, Princy, Zahorian

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//     This file is a part of the YAAPT program, designed for a fundamental
	//   frequency tracking algorithm that is extermely robust for both high quality
	//   and telephone speech.
	//     The YAAPT program was created by the Speech Communication Laboratory of
	//   the state university of New York at Binghamton. The program is available
	//   at http://www.ws.binghamton.edu/zahorian as free software. Further
	//   information about the program could be found at "A spectral/temporal
	//   method for robust fundamental frequency tracking," J.Acosut.Soc.Am. 123(6),
	//   June 2008.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	using namespace speech;

	//-- PARAMETERS: set up all of these parameters --------------------------------
	auto nframesize = fix(Prm.frame_length*Fs/1000);
	auto nframejump = fix(Prm.frame_space*Fs/1000);
	auto noverlap = nframesize - nframejump; // overlap width in sample
	auto numframes = fix(double(Data.size() - noverlap) / nframejump);
	nframesize = nframesize * 2;

	// Max number of peak candidates found
	auto maxpeaks = Prm.shc_maxpeaks;
	auto nfftlength = Prm.fft_length; // FFT length
	// Resolution of spectrum
	auto delta = Fs / nfftlength;
	// Window width in sample
	auto window_length = fix(Prm.shc_window/delta);
	if (window_length % 2 == 0) {
		window_length = window_length + 1;
	}
	// Half window width
	auto half_winlen = fix(Prm.shc_window/(delta*2));
	// Max range of SHC
	auto max_SHC = fix((Prm.f0_max+Prm.shc_pwidth*2)/delta);
	// Min range of SHC
	auto min_SHC = intptr_t(ceil(Prm.f0_min/delta));
	// Number of harmomics considered
	auto numharmonics = Prm.shc_numharms;


	//-- INITIALIZATION -----------------------------------------------------------
	Array<double> CandsPitch(maxpeaks, numframes, 0.0);
	Array<double> CandsMerit(maxpeaks, numframes, 1.0);
	// Zero padding
	auto extra = (numframes-1) * nframejump + nframesize;
	Data.resize(Data.size() + extra); // will automatically zero out extra samples.

	//-- MAIN ROUTINE --------------------------------------------------------------
	// Compute SHC for voiced frame
	auto Kaiser_window = create_window(nframesize, nframesize, WindowType::Kaiser);
	Array<double> SHC(max_SHC, 0.0);

	// TODO: remove these index arrays
	Array<intptr_t> winix(numharmonics+1, window_length, 0);
	for (intptr_t j = 1; j <= window_length; j++)
	{
		for (intptr_t i = 1; i <= winix.nrow(); i++) {
			winix(i,j) = j;
		}
	}
	Array<intptr_t>rowix(numharmonics+1, window_length, 0);
	for (intptr_t j = 1; j <= window_length; j++)
	{
		for (intptr_t i = 1; i <= rowix.nrow(); i++) {
			rowix(i,j) = i;
		}
	}

	auto Magnit1_len = fix((numharmonics+2)*Prm.f0_max/delta) + window_length;
	speech::FFT fft(nfftlength);

	for (intptr_t frame = 1; frame <= numframes; frame++)
	{
		if (VUVEnergy[frame] > 0)
		{
			auto firstp = 1 + (frame - 1) * (nframejump);
			auto Signal = Data.slice(firstp, firstp + nframesize - 1);
			auto signal_length = Signal.size();

			assert(signal_length == Kaiser_window.size());
			for (intptr_t i = 1; i <= signal_length; i++) {
				Signal[i] *= Kaiser_window[i];
			}
			auto signal_mean = stats::mean(Signal);
			for (auto &value : Signal) {
				value -= signal_mean;
			}

			Array<double> Magnit(half_winlen+nfftlength, 0.0);
			auto &fft_data = fft.process(Signal);
			for (intptr_t i = 1; i <= nfftlength; i++) {
				Magnit[half_winlen+1] = std::abs(fft_data[i]);
			}

			auto Magnit1 = Magnit.slice(1, Magnit1_len);
			auto magnit_max = stats::maximum(Magnit1);

			for (auto &value : Magnit1) {
				value /= magnit_max;
			}

			// Compute SHC (Spectral Harmonic Correlation)
			for (intptr_t k = min_SHC; k <= max_SHC; k++)
			{
				double shc = 0.0;

				for (intptr_t j = 1; j <= winix.ncol(); j++)
				{
					double prod = 1.0;
					for (intptr_t i = 1; i <= winix.nrow(); i++)
					{
						auto index = winix(i,j) + k * rowix(i,j);
						prod *= Magnit1[index];
					}
					shc += prod;
				}
				SHC[k] = shc;
			}

			auto a = stats::maximum(SHC);

			if (a > 1)
			{
				for (auto &value : SHC) {
					value /= a;
				}
			}

			Array<double> tmp_cand, tmp_merit;
			peaks(SHC, delta, maxpeaks, Prm, tmp_cand, tmp_merit);

			for (intptr_t i = 1; i <= CandsPitch.nrow(); i++)
			{
				CandsPitch(i, frame) = tmp_cand[i];
				CandsMerit(i, frame) = tmp_merit[i];
			}
		}
		else
		{
			// if energy is low, let frame be considered as unvoiced
			for (intptr_t i = 1; i <= CandsPitch.nrow(); i++)
			{
				CandsPitch(i, frame) = 0.0;
				CandsMerit(i, frame) = 1.0;
			}
		}
	}

	// Extract the Pitch candidates of voiced frames for the future Pitch selection
	SPitch = CandsPitch.slice(1, 1, 1, CandsPitch.ncol());
	Array<bool> Idx_voiced(SPitch.size(), false);
	intptr_t Num_VCands = 0;
	for (intptr_t j = 1; j <= SPitch.size(); j++)
	{
		bool value =  (SPitch[j] > 0);
		Idx_voiced[j] = value;
		Num_VCands += int(Num_VCands);
	}
	Array<double> VCandsPitch(CandsPitch.nrow(), Num_VCands);
	Array<double> VCandsMerit(CandsMerit.nrow(), Num_VCands);
	intptr_t cand = 1;
	for (intptr_t n = 1; n <= Idx_voiced.size(); n++)
	{
		if (Idx_voiced[n])
		{
			for (intptr_t i = 1; i <= CandsPitch.nrow(); i++)
			{
				VCandsPitch(i, cand) = CandsPitch(i, n);
				VCandsMerit(i, cand) = CandsMerit(i, n);
			}
			cand++;
		}
	}

	// Average, STD of the first choice candidates
	double avg_voiced = 0.0;
	for (intptr_t j = 1; j <= VCandsPitch.ncol(); j++) {
		avg_voiced += VCandsPitch(1,j);
	}
	avg_voiced /= VCandsPitch.ncol();

	double std_voiced = 0.0;
	for (intptr_t j = 1; j <= VCandsPitch.ncol(); j++)
	{
		auto value = VCandsPitch(1,j) - avg_voiced;
		std_voiced += value * value;
	}
	std_voiced = sqrt(std_voiced / double(VCandsPitch.ncol() - 1));

	// Weight the deltas, so that higher merit candidates are considered
	// more favorably
	Array<double> delta1(VCandsPitch.nrow(), VCandsPitch.ncol());
	for (intptr_t i = 1; i <= delta1.nrow(); i++)
	{
		for (intptr_t j = 1; j <= delta1.ncol(); j++)
		{
			delta1(i,j) = std::abs((VCandsPitch(i,j) - 0.8 * avg_voiced)) * (3 - VCandsMerit(i,j));
		}
	}

	// Interpolation of the weigthed candidates
	Array<intptr_t> Idx;
	auto VR = get_minimum(delta1, Idx);
	Array<double> VPeak_minmrt(Idx.size(), 0.0);
	Array<double> VMerit_minmrt(Idx.size(), 0.0);
	for (intptr_t n = 1; n <= Idx.size(); n++)
	{
		VPeak_minmrt[n]  = VCandsPitch(Idx[n], n);
		VMerit_minmrt[n] = VCandsMerit(Idx[n], n);
	}

	VPeak_minmrt = medfilt1(VPeak_minmrt, (std::max)(1, Prm.median_value-2));
	// Replace the lowest merit candidates by the median smoothed ones
	// computed from highest merit peaks above
	for (intptr_t n = 1; n <= Idx.size(); n++)
	{
		VCandsPitch(Idx[n], n) = VPeak_minmrt[n];
		// Assign merit for the smoothed peaks
		VCandsMerit(Idx[n], n) = VMerit_minmrt[n];
	}

	// Use dynamic programming to find best overal path among pitch candidates
	// Dynamic weight for transition costs
	// balance between local and transition costs
	auto weight_trans = Prm.dp5_k1 * std_voiced / avg_voiced;
	Array<double> VPitch;

	if (Num_VCands > 2)
	{
		VPitch = dynamic5(VCandsPitch, VCandsMerit, weight_trans);
		VPitch = medfilt1(VPitch, (std::max)(Prm.median_value-2, 1));
	}
	else if (Num_VCands > 0)
	{
		for (intptr_t i = 1; i <= Num_VCands; i++)
		{   // assume at least 1 voiced candidate
			VPitch[i] = 150;   // Not really correct,  but should prevent hang up if only  0,1,2,3 voiced candidates
		}
	}
	//   This should only occur for very short pitch tracks, and even then rarely
	else
	{
		VPitch.append(150.);    // to handle when no real voiced candidates
		Idx_voiced[1] = true;
	}

	// Computing some statistics from the voiced frames
	pAvg = stats::mean(VPitch);
	pStd = stats::stdev(VPitch);

	// Stretching out the smoothed pitch track
	SPitch.reserve(Idx_voiced.size());
	for (intptr_t j = 1; j <= Idx_voiced.size(); j++)
	{
		auto it = VPitch.begin();
		if (Idx_voiced[j]) {
			SPitch.append(*it++);
		}
		else {
			SPitch.append(0.0);
		}
	}

	// Interpolating thru unvoiced frames
	if (SPitch.first() < pAvg/2) {
		SPitch.first() = pAvg;
	}
	if (SPitch.last() < pAvg/2) {
		SPitch.last() = pAvg;
	}

	[Indrows, Indcols, Vals] = find(SPitch);
	SPitch = interp1(Indcols, Vals, [1:numframes], 'pchip');

	constexpr int FILTER_ORDER = 3;
	Array<double> flt_b(FILTER_ORDER, 1/FILTER_ORDER);
	constexpr double flt_a = 1.0;
	SPitch = filter(flt_b, flt_a, SPitch);

	//  above messes up  first few values of SPitch  ---  simple fix up
	//  Note--   this fix up should be based on above filter order

	SPitch[1] = SPitch[3];
	SPitch[2] = SPitch[4];

	// Create pitch track with Voiced/Unvoiced decision
	VUVSPitch = SPitch;
	for (intptr_t i = 1; i <= VUVSPitch.size(); i++)
	{
		if (VUVEnergy[i] == 0) {
			VUVSPitch[i] = 0;
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------

//Pitch, numfrms, frmrate] = ;
Array<double> yaapt(const Array<double> &Data, double Fs, bool VU, const PitchOptions &Prm, int speed)
{
	// YAAPT Fundamental Frequency (Pitch) tracking
	//
	// [Pitch, numfrms, frmrate] = yaapt(Data, Fs, VU, ExtrPrm, fig, speed)
	//   , the function is to check input parameters and invoke a number of associated routines
	//   for the YAAPT pitch tracking.
	//
	// INPUTS:
	//   Data:       Input speech raw data
	//   Fs:         Sampling rate of the input data
	//   VU:         Whether to use voiced/unvoiced decision with 1 for True and 0 for
	//               False.The default is 1.
	//   Prm:        parameters for performance control.
	//               See available parameters defined in yaapt.m
	//               e.g.,
	//               ExtrPrm.f0_min = 60;         // Change minimum search F0 to 60Hz
	//               ExtrmPrm.fft_length = 8192;  // Change FFT length to 8192
	//   speed:      choose processing speed of the YAPPT. This version of YAPPT
	//               has three processing speeds. 1 means the complete version
	//               of YAPPT, which has highest overall accuracy but slowest processing speed
	//               2 is a version only use linear signal in time domain
	//               processing, which has a balanced performance in both
	//               accuracy and processing speed. 3 is the spectral only
	//               version YAPPT, which only use frequency domain processing.
	//               This version is the fastest, but has lower accuracy. The
	//               default is 2.
	// OUTPUTS:
	//   Pitch:      Final pitch track in Hz. Unvoiced frames are assigned to 0s.
	//   numfrms:    Total number of calculated frames, or the length of
	//               output pitch track
	//   frmrate:    Frame rate of output pitch track in ms

	//  Creation Date:  June 2000
	//  Revision date:  Jan 2, 2002 , Jan 13, 2002 Feb 19, 2002, Mar 3, 2002
	//                  June 11, 2002, Jun 30, 2006, July 27, 2007
	//                  May 20, 2012: Add the VU parameter for whether to use
	//                  voiced/unvoiced decision.
	//  Authors:        Hongbing Hu, Stephen A.Zahorian

	////////////////////////////////////////////////////////////////////////////////
	//     This file is a part of the YAAPT program, designed for a fundamental
	//   frequency tracking algorithm that is extermely robust for both high quality
	//   and telephone speech.
	//     The YAAPT program was created by the Speech Communication Laboratory of
	//   the state university of New York at Binghamton. The program is available
	//   at http://www.ws.binghamton.edu/zahorian as free software. Further
	//   information about the program can be found at "A spectral/temporal
	//   method for robust fundamental frequency tracking," J.Acosut.Soc.Am. 123(6),
	//   June 2008.
	////////////////////////////////////////////////////////////////////////////////

	//  Step 1. Preprocessing
	//  Create the squared or absolute values of filtered speech data
	Array<double> DataB, DataC, DataD;
	auto nFs = nonlinear(Data, Fs, Prm, DataB, DataC, DataD);

	//  Check frame size, frame jump and the number of frames for nonlinear signal
	auto nframesize = fix(Prm.frame_length*nFs/1000);
	if (nframesize < 15) {
		throw error("Frame length value % is too short", Prm.frame_length);
	}
	if (nframesize > 2048) {
		throw error("Frame length value % exceeds the limit", Prm.frame_length);
	}

	//  Step 2. Spectral pitch tracking
	//  Calculate NLFER and determine voiced/unvoiced frames with NLFER
	Array<double> Energy;
	Array<bool> VUVEnergy;
	nlfer(DataB, nFs, Prm, Energy, VUVEnergy);

	//  Calculate an approximate pitch track from the spectrum.
	//  At this point, SPitch is best estimate of pitch track from spectrum
	[SPitch, VUVSPitch, pAvg, pStd]= spec_track(DataD, nFs, VUVEnergy, Prm);


	//  Step 3. Temporal pitch tracking based on NCCF
	//  Calculate a pitch track based on time-domain processing
	//  Pitch tracking from the filterd original signal
	[TPitch1, TMerit1] = tm_trk(DataB, nFs, SPitch, pStd, pAvg, Prm);

	//  Pitch tracking from the filterd nonlinear signal
	// [TPitch2, TMerit2] = tm_trk(DataD, nFs, SPitch, pStd, pAvg, Prm);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// this step is inserted for unifying numframe of both temporal and spectral
	// track
	lenspectral = length(SPitch);
	lentemporal = length(TPitch1);

	if lentemporal < lenspectral
	TPitch1 = [TPitch1, zeros(3,lenspectral-lentemporal)];
	TMerit1 = [TMerit1, zeros(3,lenspectral-lentemporal)];
	end
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Refine pitch candidates
	[RPitch, Merit] = refine(TPitch1, TMerit1, TPitch1, TMerit1, SPitch, ...
	Energy, VUVEnergy, Prm);

	// Step 5. Use dyanamic programming to determine the final pitch
	Pitch  = dynamic(RPitch, Merit, Energy, Prm);
	numfrms = length(Pitch);
	frmrate = Prm.frame_space;

	// Replace above VU = 0 section for new ptch_fix() function
	if (VU==0)
		Pitch = ptch_fix(Pitch);
	end


	//     Pitch = SPitch;   // to see how good spectral track is

	//== FIGURE ====================================================================
	//  Several plots to illustrate processing and performance
	if (GraphPar)
		pt_figs(DataB, DataD, nFs, SPitch, Energy, VUVEnergy, RPitch, Pitch, Prm);
	end
	case 3
		// Overwrite parameters if they are passed from command line(ExtrPar)
		if ((nargin > 3) && isstruct(ExtrPrm))
			fdNames = fieldnames(ExtrPrm);
	for n = 1:length(fdNames)
	pName = char(fdNames(n));
	Prm.(pName) = ExtrPrm.(pName);
	//message('PT:det', 'Parameter //s replaced: //d', pName,Prm.(pName));
	end
			end
	//Prm

	// Whether to plot pitch tracks, spectrum, engergy, etc.
	GraphPar = 0;
	if (nargin > 4 && ~isempty(fig))
		GraphPar = fig;
	end

			//-- MAIN ROUTINE --------------------------------------------------------------
			//  Step 1. Preprocessing
			//  Create the squared or absolute values of filtered speech data
	[DataB, DataC, DataD, nFs] = nonlinear(Data, Fs, Prm);

	//  Check frame size, frame jump and the number of frames for nonlinear singal
	nframesize = fix(Prm.frame_length*nFs/1000);
	if (nframesize < 15)
		error('Frame length value //d is too short', Prm.frame_length);
	end
	if (nframesize > 2048)
		error('Frame length value //d exceeds the limit', Prm.frame_length);
	end

			//  Step 2. Spectral pitch tracking
			//  Calculate NLFER and determine voiced/unvoiced frames with NLFER
	[Energy, VUVEnergy]= nlfer(DataB, nFs, Prm);


	//  Calculate an approximate pitch track from the spectrum.
	//  At this point, SPitch is best estimate of pitch track from spectrum
	[SPitch, VUVSPitch, pAvg, pStd,m2,m3,m4]= spec_trk2(DataD, nFs, VUVEnergy, Prm);

	// Step 5. Use dyanamic programming to determine the final pitch
	// Pitch  = dynamic(RPitch, Merit, Energy, Prm);
	numfrms = length(SPitch);
	frmrate = Prm.frame_space;



	Pitch = VUVSPitch;   // to see how good spectral track is
	if (VU==0)
		Pitch = ptch_fix(Pitch);
	end
	//== FIGURE ====================================================================
	//  Several plots to illustrate processing and performance
	if (GraphPar)
		pt_figs(DataB, DataD, nFs, SPitch, Energy, VUVEnergy, RPitch,Pitch, Prm);
	end
			otherwise

	// Overwrite parameters if they are passed from command line(ExtrPar)
	if ((nargin > 3) && isstruct(ExtrPrm))
		fdNames = fieldnames(ExtrPrm);
	for n = 1:length(fdNames)
	pName = char(fdNames(n));
	Prm.(pName) = ExtrPrm.(pName);
	//message('PT:det', 'Parameter //s replaced: //d', pName,Prm.(pName));
	end
			end
	//Prm

	// Whether to plot pitch tracks, spectrum, engergy, etc.
	GraphPar = 0;
	if (nargin > 4 && ~isempty(fig))
		GraphPar = fig;
	end

			//-- MAIN ROUTINE --------------------------------------------------------------
			//  Step 1. Preprocessing
			//  Create the squared or absolute values of filtered speech data
	[DataB, DataC, DataD, nFs] = nonlinear(Data, Fs, Prm);

	//  Check frame size, frame jump and the number of frames for nonlinear singal
	nframesize = fix(Prm.frame_length*nFs/1000);
	if (nframesize < 15)
		error('Frame length value //d is too short', Prm.frame_length);
	end
	if (nframesize > 2048)
		error('Frame length value //d exceeds the limit', Prm.frame_length);
	end

			//  Step 2. Spectral pitch tracking
			//  Calculate NLFER and determine voiced/unvoiced frames with NLFER
	[Energy, VUVEnergy]= nlfer(DataB, nFs, Prm);


	//  Calculate an approximate pitch track from the spectrum.
	//  At this point, SPitch is best estimate of pitch track from spectrum
	[SPitch, VUVSPitch, pAvg, pStd]= spec_trk(DataD, nFs, VUVEnergy, Prm);


	//  Step 3. Temporal pitch tracking based on NCCF
	//  Calculate a pitch track based on time-domain processing
	//  Pitch tracking from the filterd original signal
	[TPitch1, TMerit1] = tm_trk(DataB, nFs, SPitch, pStd, pAvg, Prm);

	//  Pitch tracking from the filterd nonlinear signal
	[TPitch2, TMerit2] = tm_trk(DataD, nFs, SPitch, pStd, pAvg, Prm);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// this step is inserted for unifying numframe of both temporal and spectral
	// track
	lenspectral = length(SPitch);
	lentemporal = length(TPitch1);


	if lentemporal < lenspectral
	TPitch1 = [TPitch1, zeros(3,lenspectral-lentemporal)];
	TPitch2 = [TPitch2, zeros(3,lenspectral-lentemporal)];
	TMerit1 = [TMerit1, zeros(3,lenspectral-lentemporal)];
	TMerit2 = [TMerit2, zeros(3,lenspectral-lentemporal)];
	end
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Refine pitch candidates
	[RPitch, Merit] = refine(TPitch1, TMerit1, TPitch2, TMerit2, SPitch, ...
	Energy, VUVEnergy, Prm);

	// Step 5. Use dyanamic programming to determine the final pitch
	Pitch  = dynamic(RPitch, Merit, Energy, Prm);
	numfrms = length(Pitch);
	frmrate = Prm.frame_space;

	// Replace above VU = 0 section for new ptch_fix() function
	if (!VU) {
		Pitch = ptch_fix(Pitch);
	}
}

} // namespace phonometrica

#endif // if 0