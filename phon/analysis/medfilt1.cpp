/*
 * Copyright 2000 Paul Kienzle, <pkienzle@kienzle.powernet.co.uk>
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained. 
 * Paul Kienzle is not responsible for the consequences of using
 * this software.
 *
 * Mar 2000 - Kai Habel (kahacjde@linux.zrz.tu-berlin.de)
 *      Change: ColumnVector x=arg(i).vector_value();
 *      to: ColumnVector x=ColumnVector(arg(i).vector_value());
 * Oct 2000 - Paul Kienzle (pkienzle@kienzle.powernet.co.uk)
 *      rewrite to ignore NaNs rather than replacing them with zero
 *      extend to handle matrix arguments
 * Jun 2021 - Julien Eychenne
 *      adapt for Phonometrica
 */

#include <cmath>
#include <phon/array.hpp>

using namespace phonometrica;

// The median class holds a sorted data window.  This window is
// intended to slide over the data, so when the window shifts
// by one position, the old value from the start of the window must
// be removed and the new value from the end of the window must
// be added.  Since removals and additions generally occur in pairs,
// a hole is left in the sorted window when the value is removed so
// that on average, fewer values need to be shifted to close the
// hole and open a new one in the sorted position.
class Median
{
private:
	double *window; // window data
	int max;        // length of window used
	int hole;       // position of hole, or max if no hole
	void close_hole() // close existing hole
	{
		// move hole to the end of the window
		while (hole < max - 1)
		{
			window[hole] = window[hole + 1];
			hole++;
		}
		// shorten window (if no hole, then hole==max)
		if (hole == max - 1) max--;
	}

	void print();

public:
	Median(int n)
	{
		max = hole = 0;
		window = new double[n];
	}

	void add(double v);          // add a new value
	void remove(double v);       // remove an existing value
	void clear()
	{ max = hole = 0; } // clear the window
	double operator()();        // find the median in the window
};


// Remove a value from the sorted window, leaving a hole.  The caller
// must promise to only remove values that they have added.
void Median::remove(double v)
{
	// NaN's are not added or removed
	if (std::isnan(v)) return;

	//  octave_stdout << "Remove " << v << " from "; print();

	// only one hole allowed, so close pre-existing ones
	close_hole();

	// binary search to find the value to remove
	int lo = 0, hi = max - 1;
	hole = hi / 2;
	while (lo <= hi)
	{
		if (v > window[hole]) lo = hole + 1;
		else if (v < window[hole]) hi = hole - 1;
		else break;
		hole = (lo + hi) / 2;
	}

	// Verify that it is the correct value to replace
	// Note that we shouldn't need this code since we are always replacing
	// a value that is already in the window, but for some reason
	// v==window[hole] occasionally doens't work.
	if (v != window[hole])
	{
		for (hole = 0; hole < max - 1; hole++)
			if (fabs(v - window[hole]) < fabs(v - window[hole + 1])) break;
		PHON_LOG("medfilt1: value %f not found---removing %f instead\n", v, window[hole]);
	}
}

// Insert a new value in the sorted window, plugging any holes, or
// extending the window as necessary.  The caller must promise not
// to add more values than median was created with, without
// removing some beforehand.
void Median::add(double v)
{
	// NaN's are not added or removed
	if (std::isnan(v)) return;

	//  octave_stdout << "Add " << v << " to "; print();

	// If no holes, extend the array
	if (hole == max) max++;

	// shift the hole up to the beginning as far as it can go.
	while (hole > 0 && window[hole - 1] > v)
	{
		window[hole] = window[hole - 1];
		hole--;
	}

	// or shift the hole down to the end as far as it can go.
	while (hole < max - 1 && window[hole + 1] < v)
	{
		window[hole] = window[hole + 1];
		hole++;
	}

	// plug in the replacement value
	window[hole] = v;

	// close the hole
	hole = max;

	//  octave_stdout << " gives "; print(); octave_stdout << endl;
}

// Compute the median value from the sorted window
// Return the central value if there is one or the average of the 
// two central values.  Return NaN if there are no values.
double Median::operator()()
{
	close_hole();

	if (max % 2 == 1)
		return window[(max - 1) / 2];
	else if (max == 0)
		return std::nan("");
	else
		return (window[max / 2 - 1] + window[max / 2]) / 2.0;
}

Array<double> medfilt1(const Array<double> &signal, int n)
{
	// Create a window to hold the sorted median values
	Median median(n);
	intptr_t mid = n / 2; // mid-point of the window
	intptr_t len = signal.size(); // number of points to process
	Array<double> filter(len, 0.0); // filtered signal to return

	intptr_t start = -n, end = 0, pos = -(n - mid) + 1;
	while (pos < len)
	{
		if (start >= 0) median.remove(signal(start+1));
		if (end < len) median.add(signal(end + 1));
		if (pos >= 0) filter(pos+1) = median();
		start++, end++, pos++;
	}

	return filter;
}
