/*
 * This software has been licensed to the Centre of Speech Technology, KTH
 * by Microsoft Corp. with the terms in the accompanying file BSD.txt,
 * which is a BSD style license.
 *
 *    "Copyright (c) 1990-1996 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * Written by:  Derek Lin
 * Checked by:
 * Revised by:  David Talkin
 *
 * Brief description:  Estimates F0 using normalized cross correlation and
 *   dynamic programming.
 *
 */
/* ----------------------------------------------------------------- */
/*             The Speech Signal Processing Toolkit (SPTK)           */
/*             developed by SPTK Working Group                       */
/*             http://sp-tk.sourceforge.net/                         */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 1984-2007  Tokyo Institute of Technology           */
/*                           Interdisciplinary Graduate School of    */
/*                           Science and Engineering                 */
/*                                                                   */
/*                1996-2017  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the SPTK working group nor the names of its */
/*   contributors may be used to endorse or promote products derived */
/*   from this software without specific prior written permission.   */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

// Note: This code was ported to C++ and cleaned up for Phonometrica by Julien Eychenne

/****************************************************************

    $Id$

*****************************************************************/


#include <cmath>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <cstdio>
#include <exception>
#include <limits>

#ifndef TRUE
# define TRUE 1
# define FALSE 0
#endif
#ifndef DBL_MAX
# define DBL_MAX std::numeric_limits<double>::max()
#endif
#ifndef M_PI
# define M_PI (3.1415926536f)
#endif

#include "jkGetF0.h"

int debug_level = 0;

void free_dp_f0();

struct float_list
{
	double f;
	float_list *next;
};


static
void get_cand(Cross *cross, double *peak, int *loc, int nlags, int *ncand, double cand_thresh);

static
int
downsamp(double *in, double *out, int samples, int *outsamps, int state_idx, int decimate, int ncoef, double fc[], int init);

static
void
do_ffir(double *buf, int in_samps, double *bufo, int *out_samps, int idx, int ncoef, double *fc, int invert, int skip, int init);

static
int lc_lin_fir(double fc, int *nf, double *coef);

static
void peak(double *y, double *xp, double *yp);

static
int eround(double flnum);

static
Frame *alloc_frame(int nlags, int ncands);

static
int retrieve_windstat(double *rho, int order, double *err, double *rms);

static
Stat *get_stationarity(double *fdata, double freq, int buff_size, int nframes, int frame_step, int first_time);

static
char *getmem(const size_t leng, const size_t size)
{
	char *p = nullptr;

	if ((p = (char *) calloc(leng, size)) == nullptr)
	{
		throw std::bad_alloc();
	}
	return (p);
}

static
double *fgetmem(const int leng)
{
	return ((double *) getmem((size_t) leng, sizeof(double)));
}

static
double rnd(unsigned long *next)
{
	double r;

	*next = *next * 1103515245L + 12345;
	r = (*next / 65536L) % 32768L;

	return (r / RAND_MAX);
}

static
double nrandom(unsigned long *next)
{
	static int sw = 0;
	static double r1, r2, s;

	if (sw == 0)
	{
		sw = 1;
		do
		{
			r1 = 2 * rnd(next) - 1;
			r2 = 2 * rnd(next) - 1;
			s = r1 * r1 + r2 * r2;
		} while (s > 1 || s == 0);
		s = sqrt(-2 * log(s) / s);
		return (r1 * s);
	}
	else
	{
		sw = 0;
		return (r2 * s);
	}
}


/*
 * Some consistency checks on parameter values.
 * Return a positive integer if any errors detected, 0 if none.
 */

static int
check_f0_params(F0_params *par, double sample_freq)
{
	int error = 0;
	double dstep;

	if ((par->max_f0 <= par->min_f0) || (par->max_f0 >= (sample_freq / 2.0)) ||
		(par->min_f0 < (sample_freq / 10000.0)))
	{
		fprintf(stderr, "ERROR: min(max)_f0 parameter inconsistent with sampling frequency.\n");
		error++;
	}
	dstep = ((double) ((int) (0.5 + (sample_freq * par->frame_step)))) / sample_freq;
	if (dstep != par->frame_step)
	{
		par->frame_step = (double) dstep;
	}
	if ((par->frame_step > 0.1) || (par->frame_step < (1.0 / sample_freq)))
	{
		fprintf(stderr, "ERROR: frame_step parameter must be between [1/sampling rate, 0.1].\n");
		error++;
	}

	return (error);
}


/* ----------------------------------------------------------------------- */
void
get_fast_cands(double *fdata, double *fdsdata, int ind, int step, int size, int dec, int start, int nlags, double *engref,
			   int *maxloc, double *maxval, Cross *cp, double *peaks, int *locs, int *ncand, F0_params *par)
{
	int decind, decstart, decnlags, decsize, i, j, *lp;
	double *corp, xp, yp, lag_wt;
	double *pe;

	lag_wt = par->lag_weight / nlags;
	decnlags = 1 + (nlags / dec);
	if ((decstart = start / dec) < 1) decstart = 1;
	decind = (ind * step) / dec;
	decsize = 1 + (size / dec);
	corp = cp->correl;

	crossf(fdsdata + decind, decsize, decstart, decnlags, engref, maxloc, maxval, corp);
	cp->maxloc = *maxloc;    /* location of maximum in correlation */
	cp->maxval = *maxval;    /* max. correlation value (found at maxloc) */
	cp->rms = (double) sqrt(*engref / size); /* rms in reference window */
	cp->firstlag = decstart;

	get_cand(cp, peaks, locs, decnlags, ncand, par->cand_thresh); /* return high peaks in xcorr */

	/* Interpolate to estimate peak locations and values at high sample rate. */
	for (i = *ncand, lp = locs, pe = peaks; i--; pe++, lp++)
	{
		j = *lp - decstart - 1;
		peak(&corp[j], &xp, &yp);
		*lp = (*lp * dec) + (int) (0.5 + (xp * dec)); /* refined lag */
		*pe = yp * (1.0f - (lag_wt * *lp)); /* refined amplitude */
	}

	if (*ncand >= par->n_cands)
	{    /* need to prune candidates? */
		int *loc, *locm, lt;
		double smaxval, *pem;
		int outer, inner, lim;
		for (outer = 0, lim = par->n_cands - 1; outer < lim; outer++)
			for (inner = *ncand - 1 - outer,
				 pe = peaks + (*ncand) - 1, pem = pe - 1,
				 loc = locs + (*ncand) - 1, locm = loc - 1;
				 inner--;
				 pe--, pem--, loc--, locm--)
				if ((smaxval = *pe) > *pem)
				{
					*pe = *pem;
					*pem = smaxval;
					lt = *loc;
					*loc = *locm;
					*locm = lt;
				}
		*ncand = par->n_cands - 1;  /* leave room for the unvoiced hypothesis */
	}
	crossfi(fdata + (ind * step), size, start, nlags, 7, engref, maxloc,
			maxval, corp, locs, *ncand);

	cp->maxloc = *maxloc;    /* location of maximum in correlation */
	cp->maxval = *maxval;    /* max. correlation value (found at maxloc) */
	cp->rms = (double) sqrt(*engref / size); /* rms in reference window */
	cp->firstlag = start;
	get_cand(cp, peaks, locs, nlags, ncand, par->cand_thresh); /* return high peaks in xcorr */
	if (*ncand >= par->n_cands)
	{    /* need to prune candidates again? */
		int *loc, *locm, lt;
		double smaxval, *pe, *pem;
		int outer, inner, lim;
		for (outer = 0, lim = par->n_cands - 1; outer < lim; outer++)
			for (inner = *ncand - 1 - outer,
				 pe = peaks + (*ncand) - 1, pem = pe - 1,
				 loc = locs + (*ncand) - 1, locm = loc - 1;
				 inner--;
				 pe--, pem--, loc--, locm--)
				if ((smaxval = *pe) > *pem)
				{
					*pe = *pem;
					*pem = smaxval;
					lt = *loc;
					*loc = *locm;
					*locm = lt;
				}
		*ncand = par->n_cands - 1;  /* leave room for the unvoiced hypothesis */
	}
}

/* ----------------------------------------------------------------------- */
double *
downsample(double *input, int samsin, int state_idx, double freq, int *samsout, int decimate, int first_time, int last_time)
{
	static double b[2048];
	static double *foutput = nullptr;
	double beta = 0.0f;
	static int ncoeff = 127, ncoefft = 0;
	int init;

	if (input && (samsin > 0) && (decimate > 0) && *samsout)
	{
		if (decimate == 1)
		{
			return (input);
		}

		if (first_time)
		{
			int nbuff = (samsin / decimate) + (2 * ncoeff);

			ncoeff = ((int) (freq * .005)) | 1;
			beta = .5f / decimate;
			foutput = (double *) realloc((void *) foutput, sizeof(double) * nbuff);

			for (; nbuff > 0;)
				foutput[--nbuff] = 0.0;

			if (!lc_lin_fir(beta, &ncoeff, b))
			{
				fprintf(stderr, "\nProblems computing interpolation filter\n");
				free((void *) foutput);

				return (nullptr);
			}
			ncoefft = (ncoeff / 2) + 1;
		}            /*  endif new coefficients need to be computed */

		if (first_time) init = 1;
		else if (last_time) init = 2;
		else init = 0;

		if (downsamp(input, foutput, samsin, samsout, state_idx, decimate, ncoefft, b, init))
		{
			return (foutput);
		}
		else
			Fprintf(stderr, "Problems in downsamp() in downsample()\n");
	}

	return (nullptr);
}

/* ----------------------------------------------------------------------- */
/* Get likely candidates for F0 peaks. */
void get_cand(Cross *cross, double *peak, int *loc, int nlags, int *ncand, double cand_thresh)
{
	int i, lastl, *t;
	double o, p, q, *r, *s, clip;
	int start, ncan;

	clip = (double) (cand_thresh * cross->maxval);
	lastl = nlags - 2;
	start = cross->firstlag;

	r = cross->correl;
	o = *r++;            /* first point */
	q = *r++;                    /* middle point */
	p = *r++;
	s = peak;
	t = loc;
	ncan = 0;
	for (i = 1; i < lastl; i++, o = q, q = p, p = *r++)
	{
		if ((q > clip) &&        /* is this a high enough value? */
			(q >= p) && (q >= o))
		{ /* NOTE: this finds SHOLDERS and PLATEAUS
				      as well as peaks (is this a good idea?) */
			*s++ = q;        /* record the peak value */
			*t++ = i + start;    /* and its location */
			ncan++;            /* count number of peaks found */
		}
	}
/*
  o = q;
  q = p;
  if( (q > clip) && (q >=0)){
    *s++ = q;
    *t++ = i+start;
    ncan++;
  }
*/
	*ncand = ncan;
}

/* ----------------------------------------------------------------------- */
/* buffer-to-buffer downsample operation */
/* This is STRICTLY a decimator! (no upsample) */
int
downsamp(double *in, double *out, int samples, int *outsamps, int state_idx, int decimate, int ncoef, double fc[], int init)
{
	if (in && out)
	{
		do_ffir(in, samples, out, outsamps, state_idx, ncoef, fc, 0, decimate, init);
		return (TRUE);
	}
	else
		printf("Bad signal(s) passed to downsamp()\n");
	return (FALSE);
}

/*      ----------------------------------------------------------      */
void
do_ffir(double *buf, int in_samps, double *bufo, int *out_samps, int idx, int ncoef, double *fc, int invert, int skip, int init)
/* fc contains 1/2 the coefficients of a symmetric FIR filter with unity
    passband gain.  This filter is convolved with the signal in buf.
    The output is placed in buf2.  If(invert), the filter magnitude
    response will be inverted.  If(init&1), beginning of signal is in buf;
    if(init&2), end of signal is in buf.  out_samps is set to the number of
    output points placed in bufo. */

{
	double *dp1, *dp2, *dp3, sum, integral;
	static double *co = nullptr, *mem = nullptr;
	static double state[1000];
	static int fsize = 0, resid = 0;
	int i, j, k, l;
	double *sp;
	double *buf1;

	buf1 = buf;
	if (ncoef > fsize)
	{/*allocate memory for full coeff. array and filter memory */    fsize = 0;
		i = (ncoef + 1) * 2;

		if (!((co = (double *) realloc((void *) co, sizeof(double) * i))
			  && (mem = (double *) realloc((void *) mem, sizeof(double) * i))))
		{
			throw std::bad_alloc();
		}
		fsize = ncoef;
	}

	/* fill 2nd half with data */
	for (i = ncoef, dp1 = mem + ncoef - 1; i-- > 0;) *dp1++ = *buf++;

	if (init & 1)
	{    /* Is the beginning of the signal in buf? */
		/* Copy the half-filter and its mirror image into the coefficient array. */
		for (i = ncoef - 1, dp3 = fc + ncoef - 1, dp2 = co, dp1 = co + ((ncoef - 1) * 2),
			 integral = 0.0; i-- > 0;)
			if (!invert) *dp1-- = *dp2++ = *dp3--;
			else
			{
				integral += (sum = *dp3--);
				*dp1-- = *dp2++ = -sum;
			}
		if (!invert) *dp1 = *dp3;    /* point of symmetry */
		else
		{
			integral *= 2;
			integral += *dp3;
			*dp1 = integral - *dp3;
		}

		for (i = ncoef - 1, dp1 = mem; i-- > 0;) *dp1++ = 0;
	}
	else
		for (i = ncoef - 1, dp1 = mem, sp = state; i-- > 0;) *dp1++ = *sp++;

	i = in_samps;
	resid = 0;

	k = (ncoef << 1) - 1;    /* inner-product loop limit */

	if (skip <= 1)
	{       /* never used */
/*    *out_samps = i;	
    for( ; i-- > 0; ) {	
      for(j=k, dp1=mem, dp2=co, dp3=mem+1, sum = 0.0; j-- > 0;
	  *dp1++ = *dp3++ )
	sum += *dp2++ * *dp1;

      *--dp1 = *buf++;	
      *bufo++ = (sum < 0.0)? sum -0.5 : sum +0.5; 
    }
    if(init & 2) {	
      for(i=ncoef; i-- > 0; ) {
	for(j=k, dp1=mem, dp2=co, dp3=mem+1, sum = 0.0; j-- > 0;
	    *dp1++ = *dp3++ )
	  sum += *dp2++ * *dp1;
	*--dp1 = 0.0;
	*bufo++ = (sum < 0)? sum -0.5 : sum +0.5; 
      }
      *out_samps += ncoef;
    }
    return;
*/
	}
	else
	{            /* skip points (e.g. for downsampling) */
		/* the buffer end is padded with (ncoef-1) data points */
		for (l = 0; l < *out_samps; l++)
		{
			for (j = k - skip, dp1 = mem, dp2 = co, dp3 = mem + skip, sum = 0.0; j-- > 0;
				 *dp1++ = *dp3++)
				sum += *dp2++ * *dp1;
			for (j = skip; j-- > 0; *dp1++ = *buf++) /* new data to memory */
				sum += *dp2++ * *dp1;
			*bufo++ = (sum < 0.0) ? sum - 0.5f : sum + 0.5f;
		}
		if (init & 2)
		{
			resid = in_samps - *out_samps * skip;
			for (l = resid / skip; l-- > 0;)
			{
				for (j = k - skip, dp1 = mem, dp2 = co, dp3 = mem + skip, sum = 0.0; j-- > 0;
					 *dp1++ = *dp3++)
					sum += *dp2++ * *dp1;
				for (j = skip; j-- > 0; *dp1++ = 0.0)
					sum += *dp2++ * *dp1;
				*bufo++ = (sum < 0.0) ? sum - 0.5f : sum + 0.5f;
				(*out_samps)++;
			}
		}
		else
			for (dp3 = buf1 + idx - ncoef + 1, l = ncoef - 1, sp = state; l-- > 0;) *sp++ = *dp3++;
	}
}

/*      ----------------------------------------------------------      */
int lc_lin_fir(double fc, int *nf, double *coef)
/* create the coefficients for a symmetric FIR lowpass filter using the
   window technique with a Hanning window. */
{
	int i, n;
	double twopi, fn, c;

	if (((*nf % 2) != 1))
		*nf = *nf + 1;
	n = (*nf + 1) / 2;

	/*  Compute part of the ideal impulse response (the sin(x)/x kernel). */
	twopi = M_PI * 2.0;
	coef[0] = (double) (2.0 * fc);
	c = M_PI;
	fn = twopi * fc;
	for (i = 1; i < n; i++) coef[i] = (double) (sin(i * fn) / (c * i));

	/* Now apply a Hanning window to the (infinite) impulse response. */
	/* (Probably should use a better window, like Kaiser...) */
	fn = twopi / (double) (*nf);
	for (i = 0; i < n; i++)
		coef[n - i - 1] *= (double) ((.5 - (.5 * cos(fn * ((double) i + 0.5)))));

	return (TRUE);
}


/* ----------------------------------------------------------------------- */
/* Use parabolic interpolation over the three points defining the peak
 * vicinity to estimate the "true" peak. */
void peak(double *y, double *xp, double *yp)
/*y: vector of length 3 defining peak */
/*xp, yp: x,y values of parabolic peak fitting the input points. */
{
	double a, c;

	a = (double) ((y[2] - y[1]) + (.5 * (y[0] - y[2])));
	if (fabs(a) > .000001)
	{
		*xp = c = (double) ((y[0] - y[2]) / (4.0 * a));
		*yp = y[1] - (a * c * c);
	}
	else
	{
		*xp = 0.0;
		*yp = y[1];
	}
}

/* A fundamental frequency estimation algorithm using the normalized
   cross correlation function and dynamic programming.  The algorithm
   implemented here is similar to that presented by B. Secrest and
   G. Doddington, "An integrated pitch tracking algorithm for speech
   systems", Proc. ICASSP-83, pp.1352-1355.  It is fully described
   by D. Talkin, "A robust algorithm for ptich tracking (RAPT)", in
   W. B. Kleijn & K. K. Paliwal (eds.) Speech Coding and Synthesis,
   (New York: Elsevier, 1995). */

/* For each frame, up to par->n_cands cross correlation peaks are
   considered as F0 intervals.  Each is scored according to its within-
   frame properties (relative amplitude, relative location), and
   according to its connectivity with each of the candidates in the
   previous frame.  An unvoiced hypothesis is also generated at each
   frame and is considered in the light of voicing state change cost,
   the quality of the cross correlation peak, and frequency continuity. */

/* At each frame, each candidate has associated with it the following
   items:
	its peak value
	its peak value modified by its within-frame properties
	its location
	the candidate # in the previous frame yielding the min. err.
		(this is the optimum path pointer!)
	its cumulative cost: (local cost + connectivity cost +
		cumulative cost of its best-previous-frame-match). */

/* Dynamic programming is then used to pick the best F0 trajectory and voicing
   state given the local and transition costs for the entire utterance. */

/* To avoid the necessity of computing the full crosscorrelation at
   the input sample rate, the signal is downsampled; a full ccf is
   computed at the lower frequency; interpolation is used to estimate the
   location of the peaks at the higher sample rate; and the fine-grained
   ccf is computed only in the vicinity of these estimated peak
   locations. */


/*
 * READ_SIZE: length of input data frame in sec to read
 * DP_CIRCULAR: determines the initial size of DP circular buffer in sec
 * DP_HIST: stored frame history in second before checking for common path 
 *      DP_CIRCULAR > READ_SIZE, DP_CIRCULAR at least 2 times of DP_HIST 
 * DP_LIMIT: in case no convergence is found, DP frames of DP_LIMIT secs
 *      are kept before output is forced by simply picking the lowest cost
 *      path
 */

#define READ_SIZE 0.2
#define DP_CIRCULAR 1.5
#define DP_HIST 0.5
#define DP_LIMIT 1.0

/* 
 * stationarity parameters -
 * STAT_WSIZE: window size in sec used in measuring frame energy/stationarity
 * STAT_AINT: analysis interval in sec in measuring frame energy/stationarity
 */
#define STAT_WSIZE 0.030
#define STAT_AINT 0.020

/*
 * headF points to current frame in the circular buffer, 
 * tailF points to the frame where tracks start
 * cmpthF points to starting frame of converged path to backtrack
 */

static Frame *headF = nullptr, *tailF = nullptr, *cmpthF = nullptr;

static int *pcands = nullptr;    /* array for backtracking in convergence check */
static int cir_buff_growth_count = 0;

static int size_cir_buffer,    /* # of frames in circular DP buffer */
size_frame_hist,    /* # of frames required before convergence test */
size_frame_out,    /* # of frames before forcing output */
num_active_frames,    /* # of frames from tailF to headF */
output_buf_size;    /* # of frames allocated to output buffers */

/* 
 * DP parameters
 */
static double tcost, tfact_a, tfact_s, frame_int, vbias, fdouble, wdur, ln2,
		freqwt, lagwt;
static int step, size, nlags, start, stop, ncomp, *locs = nullptr;
static short maxpeaks;

static int wReuse = 0;  /* number of windows seen before resued */
static Windstat *windstat = nullptr;

static double *f0p = nullptr, *vuvp = nullptr, *rms_speech = nullptr,
		*acpkp = nullptr, *peaks = nullptr;
static int first_time = 1, pad;


/*--------------------------------------------------------------------*/
int
get_Nframes(long buffsize, int pad, int step)
{
	if (buffsize < pad)
		return (0);
	else
		return int((buffsize - pad) / step);
}


/*--------------------------------------------------------------------*/
int
init_dp_f0(double freq, F0_params *par, long *buffsize, long *sdstep)
{
	int nframes;
	int i;
	int stat_wsize, agap, ind, downpatch;

/*
 * reassigning some constants 
 */

	tcost = par->trans_cost;
	tfact_a = par->trans_amp;
	tfact_s = par->trans_spec;
	vbias = par->voice_bias;
	fdouble = par->double_cost;
	frame_int = par->frame_step;

	step = eround(frame_int * freq);
	size = eround(par->wind_dur * freq);
	frame_int = (double) (((double) step) / freq);
	wdur = (double) (((double) size) / freq);
	start = eround(freq / par->max_f0);
	stop = eround(freq / par->min_f0);
	nlags = stop - start + 1;
	ncomp = size + stop + 1; /* # of samples required by xcorr
			      comp. per fr. */
	maxpeaks = 2 + (nlags / 2);    /* maximum number of "peaks" findable in ccf */
	ln2 = (double) log(2.0);
	size_frame_hist = (int) (DP_HIST / frame_int);
	size_frame_out = (int) (DP_LIMIT / frame_int);

/*
 * SET UP THE D.P. WEIGHTING FACTORS:
 *      The intent is to make the effectiveness of the various fudge factors
 *      independent of frame rate or sampling frequency.                
 */

	/* Lag-dependent weighting factor to emphasize early peaks (higher freqs)*/
	lagwt = par->lag_weight / stop;

	/* Penalty for a frequency skip in F0 per frame */
	freqwt = par->freq_weight / frame_int;

	i = (int) (READ_SIZE * freq);
	if (ncomp >= step) nframes = ((i - ncomp) / step) + 1;
	else nframes = i / step;

	/* *buffsize is the number of samples needed to make F0 computation
	   of nframes DP frames possible.  The last DP frame is patched with
	   enough points so that F0 computation on it can be carried.  F0
	   computaion on each frame needs enough points to do

	   1) xcross or cross correlation measure:
			 enough points to do xcross - ncomp

	   2) stationarity measure:
			 enough to make 30 msec windowing possible - ind

	   3) downsampling:
			 enough to make filtering possible -- downpatch

	   So there are nframes whole DP frames, padded with pad points
	   to make the last frame F0 computation ok.

	*/

	/* last point in data frame needs points of 1/2 downsampler filter length
	   long, 0.005 is the filter length used in downsampler */
	downpatch = (((int) (freq * 0.005)) + 1) / 2;

	stat_wsize = (int) (STAT_WSIZE * freq);
	agap = (int) (STAT_AINT * freq);
	ind = (agap - stat_wsize) / 2;
	i = stat_wsize + ind;
	pad = downpatch + ((i > ncomp) ? i : ncomp);
	*buffsize = nframes * step + pad;
	*sdstep = nframes * step;

	/* Allocate space for the DP storage circularly linked data structure */

	size_cir_buffer = (int) (DP_CIRCULAR / frame_int);

	/* creating circularly linked data structures */
	tailF = alloc_frame(nlags, par->n_cands);
	headF = tailF;

	/* link them up */
	for (i = 1; i < size_cir_buffer; i++)
	{
		headF->next = alloc_frame(nlags, par->n_cands);
		headF->next->prev = headF;
		headF = headF->next;
	}
	headF->next = tailF;
	tailF->prev = headF;

	headF = tailF;

	/* Allocate sscratch array to use during backtrack convergence test. */
	if (!pcands)
	{
		pcands = (int *) malloc(par->n_cands * sizeof(int));
	}

	/* Allocate arrays to return F0 and related signals. */

	/* Note: remember to compare *vecsize with size_frame_out, because
	   size_cir_buffer is not constant */
	output_buf_size = size_cir_buffer;
	rms_speech = (double *) fgetmem(output_buf_size);
	f0p = (double *) fgetmem(output_buf_size);
	vuvp = (double *) fgetmem(output_buf_size);
	acpkp = (double *) fgetmem(output_buf_size);

	/* Allocate space for peak location and amplitude scratch arrays. */
	peaks = (double *) fgetmem(maxpeaks);
	locs = (int *) malloc(sizeof(int) * maxpeaks);

	/* Initialise the retrieval/saving scheme of window statistic measures */
	wReuse = agap / step;
	if (wReuse)
	{
		windstat = (Windstat *) malloc(wReuse * sizeof(Windstat));

		for (i = 0; i < wReuse; i++)
		{
			windstat[i].err = 0;
			windstat[i].rms = 0;
		}
	}

	if (debug_level)
	{
		Fprintf(stderr, "done with initialization:\n");
		Fprintf(stderr,
				" size_cir_buffer:%d  xcorr frame size:%d start lag:%d nlags:%d\n",
				size_cir_buffer, size, start, nlags);
	}

	num_active_frames = 0;
	first_time = 1;

	return (0);
}

/*--------------------------------------------------------------------*/
int
dp_f0(double *fdata, int buff_size, int sdstep, double freq,
	  F0_params *par, double **f0p_pt, double **vuvp_pt, double **rms_speech_pt, double **acpkp_pt, int *vecsize, int last_time)
{
	double maxval, engref, *sta, *rms_ratio, *dsdata;
	double ttemp, ftemp, ft1, ferr, err, errmin;
	int i, j, k, loc1, loc2;
	int nframes, maxloc, ncand, ncandp, minloc,
			decimate, samsds;

	Stat *stat = nullptr;

	nframes = get_Nframes((long) buff_size, pad, step); /* # of whole frames */

	if (debug_level)
		Fprintf(stderr,
				"******* Computing %d dp frames ******** from %d points\n", nframes, buff_size);

	/* Now downsample the signal for coarse peak estimates. */

	decimate = (int) (freq / 2000.0);    /* downsample to about 2kHz */
	if (decimate <= 1)
		dsdata = fdata;
	else
	{
		samsds = ((nframes - 1) * step + ncomp) / decimate;
		/* skip final frame if it is too small */
		if (samsds < 1)
			return 1;

		dsdata = downsample(fdata, buff_size, sdstep, freq, &samsds, decimate,
							first_time, last_time);
		if (!dsdata)
		{
			Fprintf(stderr, "can't get downsampled data.\n");
			return 1;
		}
	}

	/* Get a function of the "stationarity" of the speech signal. */

	stat = get_stationarity(fdata, freq, buff_size, nframes, step, first_time);
	if (!stat)
	{
		Fprintf(stderr, "can't get stationarity\n");
		return (1);
	}
	sta = stat->stat;
	rms_ratio = stat->rms_ratio;

	/***********************************************************************/
	/* MAIN FUNDAMENTAL FREQUENCY ESTIMATION LOOP */
	/***********************************************************************/
	if (!first_time && nframes > 0) headF = headF->next;

	for (i = 0; i < nframes; i++)
	{

		/* NOTE: This buffer growth provision is probably not necessary.
		   It was put in (with errors) by Derek Lin and apparently never
		   tested.  My tests and analysis suggest it is completely
		   superfluous. DT 9/5/96 */
		/* Dynamically allocating more space for the circular buffer */
		if (headF == tailF->prev)
		{
			Frame *frm;

			if (cir_buff_growth_count > 5)
			{
				Fprintf(stderr,
						"too many requests (%d) for dynamically allocating space.\n   There may be a problem in finding converged path.\n", cir_buff_growth_count);
				return (1);
			}
			if (debug_level)
				Fprintf(stderr, "allocating %d more frames for DP circ. buffer.\n", size_cir_buffer);
			frm = alloc_frame(nlags, par->n_cands);
			headF->next = frm;
			frm->prev = headF;
			for (k = 1; k < size_cir_buffer; k++)
			{
				frm->next = alloc_frame(nlags, par->n_cands);
				frm->next->prev = frm;
				frm = frm->next;
			}
			frm->next = tailF;
			tailF->prev = frm;
			cir_buff_growth_count++;
		}

		headF->rms = stat->rms[i];
		get_fast_cands(fdata, dsdata, i, step, size, decimate, start,
					   nlags, &engref, &maxloc,
					   &maxval, headF->cp, peaks, locs, &ncand, par);

		/*    Move the peak value and location arrays into the dp structure */
		{
			double *ftp1, *ftp2;
			short *sp1;
			int *sp2;

			for (ftp1 = headF->dp->pvals, ftp2 = peaks,
				 sp1 = headF->dp->locs, sp2 = locs, j = ncand; j--;)
			{
				*ftp1++ = *ftp2++;
				*sp1++ = *sp2++;
			}
			*sp1 = -1;        /* distinguish the UNVOICED candidate */
			*ftp1 = maxval;
			headF->dp->mpvals[ncand] = vbias + maxval; /* (high cost if cor. is high)*/
		}

		/* Apply a lag-dependent weight to the peaks to encourage the selection
		   of the first major peak.  Translate the modified peak values into
		   costs (high peak ==> low cost). */
		for (j = 0; j < ncand; j++)
		{
			ftemp = 1.0f - ((double) locs[j] * lagwt);
			headF->dp->mpvals[j] = 1.0f - (peaks[j] * ftemp);
		}
		ncand++;            /* include the unvoiced candidate */
		headF->dp->ncands = ncand;

		/*********************************************************************/
		/*    COMPUTE THE DISTANCE MEASURES AND ACCUMULATE THE COSTS.       */
		/*********************************************************************/

		ncandp = headF->prev->dp->ncands;
		for (k = 0; k < ncand; k++)
		{    /* for each of the current candidates... */
			minloc = 0;
			errmin = DBL_MAX;
			if ((loc2 = headF->dp->locs[k]) > 0)
			{ /* current cand. is voiced */
				for (j = 0; j < ncandp; j++)
				{ /* for each PREVIOUS candidate... */
					/*    Get cost due to inter-frame period change. */
					loc1 = headF->prev->dp->locs[j];
					if (loc1 > 0)
					{ /* prev. was voiced */
						ftemp = (double) log(((double) loc2) / loc1);
						ttemp = (double) fabs(ftemp);
						ft1 = (double) (fdouble + fabs(ftemp + ln2));
						if (ttemp > ft1)
							ttemp = ft1;
						ft1 = (double) (fdouble + fabs(ftemp - ln2));
						if (ttemp > ft1)
							ttemp = ft1;
						ferr = ttemp * freqwt;
					}
					else
					{        /* prev. was unvoiced */
						ferr = tcost + (tfact_s * sta[i]) + (tfact_a / rms_ratio[i]);
					}
					/*    Add in cumulative cost associated with previous peak. */
					err = ferr + headF->prev->dp->dpvals[j];
					if (err < errmin)
					{    /* find min. cost */
						errmin = err;
						minloc = j;
					}
				}
			}
			else
			{            /* this is the unvoiced candidate */
				for (j = 0; j < ncandp; j++)
				{ /* for each PREVIOUS candidate... */

					/*    Get voicing transition cost. */
					if (headF->prev->dp->locs[j] > 0)
					{ /* previous was voiced */
						ferr = tcost + (tfact_s * sta[i]) + (tfact_a * rms_ratio[i]);
					}
					else
						ferr = 0.0;
					/*    Add in cumulative cost associated with previous peak. */
					err = ferr + headF->prev->dp->dpvals[j];
					if (err < errmin)
					{    /* find min. cost */
						errmin = err;
						minloc = j;
					}
				}
			}
			/* Now have found the best path from this cand. to prev. frame */
			if (first_time && i == 0)
			{        /* this is the first frame */
				headF->dp->dpvals[k] = headF->dp->mpvals[k];
				headF->dp->prept[k] = 0;
			}
			else
			{
				headF->dp->dpvals[k] = errmin + headF->dp->mpvals[k];
				headF->dp->prept[k] = minloc;
			}
		} /*    END OF THIS DP FRAME */

		if (i < nframes - 1)
			headF = headF->next;

		if (debug_level >= 2)
		{
			Fprintf(stderr, "%d engref:%10.0f max:%7.5f loc:%4d\n",
					i, engref, maxval, maxloc);
		}

	} /* end for (i ...) */

	/***************************************************************/
	/* DONE WITH FILLING DP STRUCTURES FOR THE SET OF SAMPLED DATA */
	/*    NOW FIND A CONVERGED DP PATH                             */
	/***************************************************************/

	*vecsize = 0;            /* # of output frames returned */

	num_active_frames += nframes;

	if (num_active_frames >= size_frame_hist || last_time)
	{
		Frame *frm;
		int num_paths, best_cand, frmcnt, checkpath_done = 1;
		double patherrmin;

		if (debug_level)
			Fprintf(stderr, "available frames for backtracking: %d\n",
					num_active_frames);

		patherrmin = DBL_MAX;
		best_cand = 0;
		num_paths = headF->dp->ncands;

		/* Get the best candidate for the final frame and initialize the
		   paths' backpointers. */
		frm = headF;
		for (k = 0; k < num_paths; k++)
		{
			if (patherrmin > headF->dp->dpvals[k])
			{
				patherrmin = headF->dp->dpvals[k];
				best_cand = k;    /* index indicating the best candidate at a path */
			}
			pcands[k] = frm->dp->prept[k];
		}

		if (last_time)
		{     /* Input data was exhausted. force final outputs. */
			cmpthF = headF;        /* Use the current frame as starting point. */
		}
		else
		{
			/* Starting from the most recent frame, trace back each candidate's
		   best path until reaching a common candidate at some past frame. */
			frmcnt = 0;
			while (true)
			{
				frm = frm->prev;
				frmcnt++;
				checkpath_done = 1;
				for (k = 1; k < num_paths; k++)
				{ /* Check for convergence. */
					if (pcands[0] != pcands[k])
						checkpath_done = 0;
				}
				if (!checkpath_done)
				{ /* Prepare for checking at prev. frame. */
					for (k = 0; k < num_paths; k++)
					{
						pcands[k] = frm->dp->prept[pcands[k]];
					}
				}
				else
				{    /* All paths have converged. */
					cmpthF = frm;
					best_cand = pcands[0];
					if (debug_level)
						Fprintf(stderr,
								"paths went back %d frames before converging\n", frmcnt);
					break;
				}
				if (frm == tailF)
				{    /* Used all available data? */
					if (num_active_frames < size_frame_out)
					{ /* Delay some more? */
						checkpath_done = 0; /* Yes, don't backtrack at this time. */
						cmpthF = nullptr;
					}
					else
					{        /* No more delay! Force best-guess output. */
						checkpath_done = 1;
						cmpthF = headF;
						/*	    Fprintf(stderr,
							"WARNING: no converging path found after going back %d frames, will use the lowest cost path\n",num_active_frames);*/
					}
					break;
				} /* end if (frm ...) */
			}    /* end while (1) */
		} /* end if (last_time) ... else */

		/*************************************************************/
		/* BACKTRACKING FROM cmpthF (best_cand) ALL THE WAY TO tailF    */
		/*************************************************************/
		i = 0;
		frm = cmpthF;    /* Start where convergence was found (or faked). */
		while (frm != tailF->prev && checkpath_done)
		{
			if (i == output_buf_size)
			{ /* Need more room for outputs? */
				output_buf_size *= 2;
				if (debug_level)
					Fprintf(stderr,
							"reallocating space for output frames: %d\n",
							output_buf_size);

				rms_speech = (double *)
						realloc((void *) rms_speech,
								sizeof(double) * output_buf_size);
				f0p = (double *) realloc((void *) f0p,
										 sizeof(double) * output_buf_size);
				vuvp =
						(double *) realloc((void *) vuvp,
										   sizeof(double) * output_buf_size);
				acpkp =
						(double *) realloc((void *) acpkp,
										   sizeof(double) * output_buf_size);


			}
			rms_speech[i] = frm->rms;
			acpkp[i] = frm->dp->pvals[best_cand];
			loc1 = frm->dp->locs[best_cand];
			vuvp[i] = 1.0;
			best_cand = frm->dp->prept[best_cand];
			ftemp = (double) loc1;
			if (loc1 > 0)
			{        /* Was f0 actually estimated for this frame? */
				if (loc1 > start && loc1 < stop)
				{ /* loc1 must be a local maximum. */
					double cormax, cprev, cnext, den;

					j = loc1 - start;
					cormax = frm->cp->correl[j];
					cprev = frm->cp->correl[j + 1];
					cnext = frm->cp->correl[j - 1];
					den = (double) (2.0 * (cprev + cnext - (2.0 * cormax)));
					/*
					 * Only parabolic interpolate if cormax is indeed a local
					 * turning point. Find peak of curve that goes though the 3 points
					 */

					if (fabs(den) > 0.000001)
						ftemp += 2.0f - ((((5.0f * cprev) + (3.0f * cnext) - (8.0f * cormax)) / den));
				}
				f0p[i] = (double) (freq / ftemp);
			}
			else
			{        /* No valid estimate; just fake some arbitrary F0. */
				f0p[i] = 0;
				vuvp[i] = 0.0;
			}
			frm = frm->prev;

			if (debug_level >= 2)
				Fprintf(stderr, " i:%4d%8.1f%8.1f\n", i, f0p[i], vuvp[i]);
			/* f0p[i] starts from the most recent one */
			/* Need to reverse the order in the calling function */
			i++;
		} /* end while() */
		if (checkpath_done)
		{
			*vecsize = i;
			tailF = cmpthF->next;
			num_active_frames -= *vecsize;
		}
	} /* end if() */

	if (debug_level)
		Fprintf(stderr, "writing out %d frames.\n", *vecsize);

	*f0p_pt = f0p;
	*vuvp_pt = vuvp;
	*acpkp_pt = acpkp;
	*rms_speech_pt = rms_speech;
	/*  *acpkp_pt = acpkp;*/

	if (first_time) first_time = 0;
	return (0);
}


/*--------------------------------------------------------------------*/
Frame *alloc_frame(int nlags, int ncands)
{
	Frame *frm;
	int j;

	frm = (Frame *) malloc(sizeof(Frame));
	frm->dp = (Dprec *) malloc(sizeof(Dprec));
	frm->dp->ncands = 0;
	frm->cp = (Cross *) malloc(sizeof(Cross));
	frm->cp->correl = (double *) malloc(sizeof(double) * nlags);
	frm->dp->locs = (short *) malloc(sizeof(short) * ncands);
	frm->dp->pvals = (double *) malloc(sizeof(double) * ncands);
	frm->dp->mpvals = (double *) malloc(sizeof(double) * ncands);
	frm->dp->prept = (short *) malloc(sizeof(short) * ncands);
	frm->dp->dpvals = (double *) malloc(sizeof(double) * ncands);

	/*  Initialize the cumulative DP costs to zero */
	for (j = ncands - 1; j >= 0; j--)
		frm->dp->dpvals[j] = 0.0;

	return (frm);
}


/*--------------------------------------------------------------------*/
/* push window stat to stack, and pop the oldest one */

static int save_windstat(double *rho, int order, double err, double rms)
{
	int i, j;

	if (wReuse > 1)
	{               /* push down the stack */
		for (j = 1; j < wReuse; j++)
		{
			for (i = 0; i <= order; i++) windstat[j - 1].rho[i] = windstat[j].rho[i];
			windstat[j - 1].err = windstat[j].err;
			windstat[j - 1].rms = windstat[j].rms;
		}
		for (i = 0; i <= order; i++) windstat[wReuse - 1].rho[i] = rho[i]; /*save*/
		windstat[wReuse - 1].err = (double) err;
		windstat[wReuse - 1].rms = (double) rms;
		return 1;
	}
	else if (wReuse == 1)
	{
		for (i = 0; i <= order; i++) windstat[0].rho[i] = rho[i];  /* save */
		windstat[0].err = (double) err;
		windstat[0].rms = (double) rms;
		return 1;
	}
	else
		return 0;
}


/*--------------------------------------------------------------------*/
int retrieve_windstat(double *rho, int order, double *err, double *rms)
{
	Windstat wstat;
	int i;

	if (wReuse)
	{
		wstat = windstat[0];
		for (i = 0; i <= order; i++) rho[i] = wstat.rho[i];
		*err = wstat.err;
		*rms = wstat.rms;
		return 1;
	}
	else return 0;
}


/*--------------------------------------------------------------------*/
static double
get_similarity(int order, int size, double *pdata, double *cdata, double *rmsa, double *rms_ratio, double pre, double stab, int w_type, int init)
{
	double rho3[BIGSORD + 1], err3, rms3, rmsd3, b0, t, a2[BIGSORD + 1],
			rho1[BIGSORD + 1], a1[BIGSORD + 1], b[BIGSORD + 1], err1, rms1, rmsd1;

/* (In the lpc() calls below, size-1 is used, since the windowing and
   preemphasis function assumes an extra point is available in the
   input data array.  This condition is apparently no longer met after
   Derek's modifications.) */

	/* get current window stat */
	xlpc(order, stab, size - 1, cdata,
		 a2, rho3, (double *) nullptr, &err3, &rmsd3, pre, w_type);
	rms3 = wind_energy(cdata, size, w_type);

	if (!init)
	{
		/* get previous window stat */
		if (!retrieve_windstat(rho1, order, &err1, &rms1))
		{
			xlpc(order, stab, size - 1, pdata,
				 a1, rho1, (double *) nullptr, &err1, &rmsd1, pre, w_type);
			rms1 = wind_energy(pdata, size, w_type);
		}
		xa_to_aca(a2 + 1, b, &b0, order);
		t = xitakura(order, b, &b0, rho1 + 1, &err1) - .8f;
		if (rms1 > 0.0)
			*rms_ratio = (0.001f + rms3) / rms1;
		else if (rms3 > 0.0)
			*rms_ratio = 2.0;    /* indicate some energy increase */
		else
			*rms_ratio = 1.0;    /* no change */
	}
	else
	{
		*rms_ratio = 1.0;
		t = 10.0;
	}
	*rmsa = rms3;
	save_windstat(rho3, order, err3, rms3);
	return ((double) (0.2 / t));
}


/* -------------------------------------------------------------------- */
/* This is an ad hoc signal stationarity function based on Itakura
 * distance and relative amplitudes.
 */
/* 
  This illustrates the window locations when the very first frame is read.
  It shows an example where each frame step |  .  | is 10 msec.  The
  frame step size is variable.  The window size is always 30 msec.
  The window centers '*' is always 20 msec apart.
  The windows cross each other right at the center of the DP frame, or
  where the '.' is.

                          ---------*---------   current window

              ---------*---------  previous window

  |  .  |  .  |  .  |  .  |  .  |  .  |  .  |  .  |  .  |
              ^           ^  ^
              ^           ^  ^
              ^           ^  fdata
              ^           ^
              ^           q
	      p

                          ---
                          ind

  fdata, q, p, ind, are variables used below.
   
*/

static Stat *stat = nullptr;
static double *mem = nullptr;

Stat *get_stationarity(double *fdata, double freq, int buff_size, int nframes, int frame_step, int first_time)
{
	static int nframes_old = 0, memsize;
	double preemp = 0.4f, stab = 30.0f;
	double *p, *q, *r, *datend;
	int ind, i, j, m, size, order, agap, w_type = 3;

	agap = (int) (STAT_AINT * freq);
	size = (int) (STAT_WSIZE * freq);
	ind = (agap - size) / 2;

	if (nframes_old < nframes || !stat || first_time)
	{
		/* move this to init_dp_f0() later */
		nframes_old = nframes;

		if (stat)
		{
			free((char *) stat->stat);
			free((char *) stat->rms);
			free((char *) stat->rms_ratio);
			free((char *) stat);
		}
		if (mem) free((void *) mem);
		stat = (Stat *) malloc(sizeof(Stat));
		stat->stat = (double *) malloc(sizeof(double) * nframes);
		stat->rms = (double *) malloc(sizeof(double) * nframes);
		stat->rms_ratio = (double *) malloc(sizeof(double) * nframes);
		memsize = (int) (STAT_WSIZE * freq) + (int) (STAT_AINT * freq);
		mem = (double *) malloc(sizeof(double) * memsize);

		for (j = 0; j < memsize; j++) mem[j] = 0;
	}

	if (nframes == 0) return (stat);

	q = fdata + ind;
	datend = fdata + buff_size;

	if ((order = (int) (2.0 + (freq / 1000.0))) > BIGSORD)
	{
		Fprintf(stderr,
				"Optimim order (%d) exceeds that allowable (%d); reduce Fs\n", order, BIGSORD);
		order = BIGSORD;
	}

	/* prepare for the first frame */
	for (j = memsize / 2, i = 0; j < memsize; j++, i++) mem[j] = fdata[i];

	/* never run over end of frame, should already taken care of when read */

	for (j = 0, p = q - agap; j < nframes; j++, p += frame_step, q += frame_step)
	{
		if ((p >= fdata) && (q >= fdata) && (q + size <= datend))
			stat->stat[j] = get_similarity(order, size, p, q,
										   &(stat->rms[j]),
										   &(stat->rms_ratio[j]), preemp,
										   stab, w_type, 0);
		else
		{
			if (first_time)
			{
				if ((p < fdata) && (q >= fdata) && (q + size <= datend))
					stat->stat[j] = get_similarity(order, size, nullptr, q,
												   &(stat->rms[j]),
												   &(stat->rms_ratio[j]),
												   preemp, stab, w_type, 1);
				else
				{
					stat->rms[j] = 0.0;
					stat->stat[j] = 0.01f * 0.2f;   /* a big transition */
					stat->rms_ratio[j] = 1.0;   /* no amplitude change */
				}
			}
			else
			{
				if ((p < fdata) && (q + size <= datend))
				{
					stat->stat[j] = get_similarity(order, size, mem,
												   mem + (memsize / 2) + ind,
												   &(stat->rms[j]),
												   &(stat->rms_ratio[j]),
												   preemp, stab, w_type, 0);
					/* prepare for the next frame_step if needed */
					if (p + frame_step < fdata)
					{
						for (m = 0; m < (memsize - frame_step); m++)
							mem[m] = mem[m + frame_step];
						r = q + size;
						for (m = 0; m < frame_step; m++)
							mem[memsize - frame_step + m] = *r++;
					}
				}
			}
		}
	}

	/* last frame, prepare for next call */
	for (j = (memsize / 2) - 1, p = fdata + (nframes * frame_step) - 1; j >= 0 && p >= fdata; j--)
		mem[j] = *p--;
	return (stat);
}

/* -------------------------------------------------------------------- */
/*	Round the argument to the nearest integer.			*/

int
eround(double flnum)
{
	return ((flnum >= 0.0) ? (int) (flnum + 0.5) : (int) (flnum - 0.5));
}


void free_dp_f0()
{
	int i;
	Frame *frm, *next;

	free((void *) pcands);
	pcands = nullptr;

	free((void *) rms_speech);
	rms_speech = nullptr;

	free((void *) f0p);
	f0p = nullptr;

	free((void *) vuvp);
	vuvp = nullptr;

	free((void *) acpkp);
	acpkp = nullptr;

	free((void *) peaks);
	peaks = nullptr;

	free((void *) locs);
	locs = nullptr;

	if (wReuse)
	{
		free((void *) windstat);
		windstat = nullptr;
	}

	frm = headF;

	for (i = 0; i < size_cir_buffer; i++)
	{
		next = frm->next;
		free((void *) frm->cp->correl);
		free((void *) frm->dp->locs);
		free((void *) frm->dp->pvals);
		free((void *) frm->dp->mpvals);
		free((void *) frm->dp->prept);
		free((void *) frm->dp->dpvals);
		free((void *) frm->cp);
		free((void *) frm->dp);
		free((void *) frm);
		frm = next;
	}
	headF = nullptr;
	tailF = nullptr;

	free((void *) stat->stat);
	free((void *) stat->rms);
	free((void *) stat->rms_ratio);

	free((void *) stat);
	stat = nullptr;

	free((void *) mem);
	mem = nullptr;
}

std::vector<double>
rapt(phonometrica::Array<double> &input, double sample_freq, int frame_shift, double minF0, double maxF0, double voice_bias, int otype)
{
	int fnum = 0;
	double *fdata;
	int done;
	long buff_size, actsize;
	double sf;
	F0_params par;
	double *f0p, *vuvp, *rms_speech, *acpkp;
	int i, vecsize;
	static int framestep = -1;
	long sdstep = 0, total_samps;
	int ndone = 0;
	double *output, *unvoiced;
	int count = 0;
	int startpos = 0, endpos = -1;
	long max;
//	float_list *cur = nullptr, *prev = nullptr;
	double p, fsp, alpha, beta;
	unsigned long next = 1;
	int length = int(input.size());

#if 1
//	for (i = 0, tmpf = data; tmpf != nullptr; i++, tmpf = tmpf->next)
	for (intptr_t i = 1; i <= input.size(); i++)
	{
		p = (double) nrandom(&next);
//		tmpf->f += (double) (p * 50.0);
		input[i] += (double) (p * 50.0);
//		prev = tmpf;
	}
#endif
	fnum = (int) (ceil((double) input.size() / (double) frame_shift));
	fsp = sample_freq * (10.0 / (double) frame_shift);
	alpha = (int) lround(0.00275 * fsp);
	beta = (int) lround((9600.0 / minF0 - 168.0) * fsp / 96000.0);
	if (beta < 0)
	{
		beta = 0;
	}

	//auto data = input.data();
#if 1
	auto extra = int((alpha + beta + 3) * frame_shift);
	input.resize(input.size() + extra);

	for (intptr_t i = length + 1; i <= input.size(); i++)
	{
		p = (double) nrandom(&next);
		input[i] = p * 50.0;
	}
	auto data = input.data();
	length += extra;
#endif

//	data = (double *) malloc(sizeof(double) * length);
	std::vector<double> result((5 + length / frame_shift), 0.0);
	output = result.data();
//	output = (double *) malloc(sizeof(double)
//							   * (5 + length / frame_shift));
	unvoiced = (double *) malloc(sizeof(double)
								 * (5 + length / frame_shift));

//	for (i = 0, tmpf = data; tmpf != nullptr; i++, tmpf = tmpf->next)
//	{
//		data[i] = tmpf->f;
//	}

	par.cand_thresh = 0.3f;
	par.lag_weight = 0.3f;
	par.freq_weight = 0.02f;
	par.trans_cost = 0.005f;
	par.trans_amp = 0.5f;
	par.trans_spec = 0.5f;
	par.voice_bias = 0.0f;
	par.double_cost = 0.35f;
	par.min_f0 = minF0;
	par.max_f0 = maxF0;
	par.frame_step = frame_shift / sample_freq;
	par.wind_dur = 0.0075f;
	par.n_cands = 20;
	par.mean_f0 = 200;          /* unused */
	par.mean_f0_weight = 0.0f;  /* unused */
	par.conditioning = 0;       /* unused */
	par.voice_bias = voice_bias; /* overwrite U/V threshold for pitch command */

	if (startpos < 0) startpos = 0;
	if (endpos >= (length - 1) || endpos == -1)
	{
		endpos = length - 1;
	}
	sf = (double) sample_freq;

	if (framestep > 0)          /* If a value was specified with -S, use it. */
		par.frame_step = (double) (framestep / sf);

	if (check_f0_params(&par, sf))
	{
		fprintf(stderr, "invalid/inconsistent parameters -- exiting.\n");

	}

	total_samps = endpos - startpos + 1;

	if (total_samps < ((par.frame_step * 2.0) + par.wind_dur) * sf)
	{
		fprintf(stderr, "data range too small for analysis by get_f0.\n");
	}

	if (init_dp_f0(sf, &par, &buff_size, &sdstep)
		|| buff_size > INT_MAX || sdstep > INT_MAX)
	{
		fprintf(stderr, "problem in init_dp_f0().\n");
	}

	if (buff_size > total_samps)
		buff_size = total_samps;

	max = buff_size > sdstep ? buff_size : sdstep;
	actsize = buff_size < length ? buff_size : length;
	fdata = (double *) malloc(sizeof(double) * max);

	/*  Snack_ProgressCallback(sound->cmdPtr, interp, "Computing pitch", 0.0);*/
	ndone = startpos;

	while (true)
	{
		done = (actsize < buff_size) || (total_samps == buff_size);
		for (i = 0; i < actsize; i++)
		{
			fdata[i] = data[i + ndone];
		}
		if (dp_f0(fdata, (int) actsize, (int) sdstep, sf, &par,
				  &f0p, &vuvp, &rms_speech, &acpkp, &vecsize, done))
		{
			break;  /* skip final frame if it is too small */

		}

		for (i = vecsize - 1; i >= 0; i--)
		{
			output[count] = f0p[i];
			unvoiced[count] = vuvp[i];
			count++;
		}


		if (done) break;

		ndone += sdstep;

		actsize = buff_size < (length - ndone)
				  ? buff_size : (length - ndone);

		total_samps -= sdstep;

		if (actsize > total_samps)
			actsize = total_samps;

		/*    if (1) {
		  int res = Snack_ProgressCallback(sound->cmdPtr, interp, "Computing pitch", (double) ndone / sound->length);
		  if (res != TCL_OK) {
		return TCL_ERROR;
		  }
		  }*/
	}

	/*Snack_ProgressCallback(sound->cmdPtr, interp, "Computing pitch", 1.0);*/

	for (i = 0; i < fnum; i++)
	{
		switch (otype)
		{
			case 1:                   /* f0 */
				fwrite(output + i, sizeof(double), 1, stdout);
				break;
			case 2:                   /* log(f0) */
				if (output[i] != 0.0)
				{
					output[i] = log(output[i]);
				}
				else
				{
					output[i] = -1.0E10;
				}
				fwrite(output + i, sizeof(double), 1, stdout);
				break;
			default:                  /* pitch */
				if (output[i] != 0.0)
				{
					output[i] = sample_freq / output[i];
				}
				fwrite(output + i, sizeof(double), 1, stdout);
				break;
		}
	}

	free(fdata);
	free_dp_f0();

	return result;
}


std::vector<double>
rapt2(phonometrica::Array<double> &input, double sample_rate, double time_step, double min_f0, double max_f0)
//rapt(phonometrica::Array<double> &input, double sample_freq, int frame_shift, double minF0, double maxF0, double voice_bias, int otype)
{
	int done;
	intptr_t buff_size, actsize;
	double start_time;
	F0_params par;
	double *f0p, *vuvp, *rms_speech, *acpkp;
	int i, vecsize;
	intptr_t frame_step = lround(sample_rate * time_step);
	intptr_t sdstep = 0, total_samps;
	int ndone = 0;
	std::vector<double> result((5 + input.size() / frame_step), 0.0);
	intptr_t count = 0;
	intptr_t startpos = 0, endpos = -1;

	par.cand_thresh = 0.3f;
	par.lag_weight = 0.3f;
	par.freq_weight = 0.02f;
	par.trans_cost = 0.005f;
	par.trans_amp = 0.5f;
	par.trans_spec = 0.5f;
	par.voice_bias = 0.0f;
	par.double_cost = 0.35f;
	par.min_f0 = min_f0;
	par.max_f0 = max_f0;
	par.frame_step = time_step;
	par.wind_dur = 0.0075f;
	par.n_cands = 20;
	par.mean_f0 = 200;          /* unused */
	par.mean_f0_weight = 0.0f;  /* unused */
	par.conditioning = 0;       /* unused */

	if (startpos < 0) startpos = 0;
	if (endpos >= (input.size() - 1) || endpos == -1)
	{
		endpos = input.size() - 1;
	}

	if (frame_step > 0)  /* If a value was specified with -S, use it. */
		par.frame_step = (double) (frame_step / sample_rate);
	start_time = 0.0f;
	if (check_f0_params(&par, sample_rate))
	{
		throw std::runtime_error("invalid/inconsistent parameters");
	}

	total_samps = endpos - startpos + 1;
	if (total_samps < ((par.frame_step * 2.0) + par.wind_dur) * sample_rate)
	{
		throw std::runtime_error("input range too small for analysis by get_f0.");
	}
	/* Initialize variables in get_f0.c; allocate data structures;
	 * determine length and overlap of input frames to read.
	 */
	if (init_dp_f0(sample_rate, &par, &buff_size, &sdstep) || buff_size > INT_MAX || sdstep > INT_MAX)
	{
		throw std::runtime_error("problem in init_dp_f0().");
	}

	if (debug_level)
		Fprintf(stderr, "init_dp_f0 returned buff_size %ld, sdstep %ld.\n", buff_size, sdstep);

	if (buff_size > total_samps)
		buff_size = total_samps;

	actsize = (std::min)(buff_size, input.size());
	std::vector<double> fdata((std::max)(buff_size, sdstep), 111);
	ndone = startpos;

	while (true)
	{
		done = (actsize < buff_size) || (total_samps == buff_size);
		auto from = input.begin() + ndone;
		auto to = from + actsize;
		std::copy(from, to, fdata.begin());

		/*if (sound->debug > 0) Snack_WriteLog("dp_f0...\n");*/
		if (dp_f0(fdata.data(), (int) actsize, (int) sdstep, sample_rate, &par,
				  &f0p, &vuvp, &rms_speech, &acpkp, &vecsize, done))
		{
			throw std::runtime_error("problem in dp_f0().");
		}
		/*if (sound->debug > 0) Snack_WriteLogInt("done dp_f0",vecsize);*/
		for (i = vecsize - 1; i >= 0; i--)
		{
			result[count] = f0p[i];
			count++;
		}

		if (done) break;

		ndone += sdstep;
		actsize = (std::min)(buff_size, input.size() - ndone);
		total_samps -= sdstep;

		if (actsize > total_samps)
			actsize = total_samps;
	}
	free_dp_f0();

	return result;
}