// This is a ported version of the matlab code for the YAAPT algorithm.
// See http://www.ws.binghamton.edu/zahorian/yaapt.htm
// C++ translation by Julien Eychenne <jeychenne@gmail.com>

#ifndef PHONOMETRICA_YAAPT_HPP
#define PHONOMETRICA_YAAPT_HPP

#include <utility>
#include <phon/array.hpp>

namespace phonometrica {


struct PitchOptions
{
	// Default values for the tracking with voiced/unvoiced decision
	int frame_length     = 35;   // Length of each analysis frame (ms)
	int frame_lengtht    = 35;   // Length of each analysis frame in the time domain (ms)
	int frame_space      = 10;   // Spacing between analysis frame (ms)
	int f0_min           = 60;   // Minimum F0 searched (Hz)
	int f0_max           = 400;  // Maximum F0 searached (Hz)
	int fft_length       = 8192; // FFT length
	int bp_forder        = 150;  // Order of bandpass filter
	int bp_low           = 50;   // Low frequency of filter passband (Hz)
	int bp_high          = 1500; // High frequency of filter passband (Hz)
	double nlfer_thresh1 = 0.75; // NLFER boundary for voiced/unvoiced decisions
	double nlfer_thresh2 = 0.1;  // Threshold for NLFER definitely unvocied
	int shc_numharms     = 3;    // Number of harmonics in SHC calculation
	int shc_window       = 40;   // SHC window length (Hz)
	int shc_maxpeaks     = 4;    // Maximum number of SHC peaks to be found
	int shc_pwidth       = 50;   // Window width in SHC peak picking (Hz)
	double shc_thresh1   = 5.0;  // Threshold 1 for SHC peak picking
	double shc_thresh2   = 1.25; // Threshold 2 for SHC peak picking
	int f0_double        = 150;  // F0 doubling decision threshold (Hz)
	int f0_half          = 150;  // F0 halving decision threshold (Hz)
	int dp5_k1           = 11;   // Weight used in dynamic program
	int dec_factor       = 1;    // Factor for signal resampling
	double nccf_thresh1  = 0.25; // Threshold for considering a peak in NCCF
	double nccf_thresh2  = 0.9;  // Threshold for terminating serach in NCCF
	int nccf_maxcands    = 3;    // Maximum number of candidates found
	int nccf_pwidth      = 5;    // Window width in NCCF peak picking
	int median_value     = 7;    // Order of medial filter
	double merit_boost   = 0.20; // Boost merit
	double merit_pivot   = 0.99; // Merit assigned to unvoiced candidates in definitely unvoiced frames
	double merit_extra   = 0.4;  // Merit assigned to extra candidates in reducing F0 doubling/halving errors
	double dp_w1         = 0.15; // DP weight factor for V-V transitions
	double dp_w2         = 0.5;  // DP weight factor for V-UV or UV-V transitions
	double dp_w3         = 0.1;  // DP weight factor of UV-UV transitions
	double dp_w4         = 0.9;  // Weight factor for local costs
};

} // namespace phonometrica

#endif // PHONOMETRICA_YAAPT_HPP
