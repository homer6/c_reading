#include <tester/Test.h>

/*
Michael Herf
December 2001
I ran across Pierre Tardiman's article on Radix Sort for floating point numbers, and I became interested in seeing how far I could push the performance.
I figured out what I think are a few unusual optimizations, and while I'm not really sure that any of them are new, the combination makes my code run pretty fast.


Multiple Histogramming
First, I use histogramming to make the radix work fast -- this is very standard stuff. The standard approach is to take all the bits in a certain radix and build the histogram for them, then summing across the histogram to determine where to copy each element, and then making a final pass to copy all the bits in-order. This is two read-passes per radix, but we can certainly do better.
In particular, histograms don't change when you change the order, so I just do all the histogramming in one pass through the data. One read builds several histograms.

So if you histogram a floating point number in four 8-bit passes, you can build four histograms from one pass through the data, rather than four.

This would mean 5 passes through the data, rather than the usual 8.


Floating point support
Pierre (above) has a nice approach to floating point sorting, a good way to find the sign bit and switch the order of the values. But I wanted to do it directly, without a need for a final pass.
If you look at them in binary, single precision floating point numbers have two problems that keeps them from being directly sortable.

[sign] [exponent] [mantissa]
First, the sign bit is set when the value is negative, which means that all negative numbers are bigger than positive ones. You could just flip it, of course (I thought this was all I had to do at first), but there's still another problem.

But the second problem is that the values are signed-magnitude, so "more negative" floating point numbers actually look bigger to a normal bitwise comparison.

To fix this, we have to do some bit-twiddling in integer. It turns out that flipping the exponent inverts the order of the exponents (flips them low to high), and flipping the mantissa does the same. Basically, the exponent is a "range" of numbers, and we flip the orders of these ranges. And then the mantissa is the numbers in each range, and we flip these as well.

We're supposed to call this a "bijective mapping" from 32-bit integers to themselves, which means, for every 32-bit number, there's another unique one that we map to, and we can invert the mapping to get back exactly where we started.

It turns out that when you say "flip" you can also say, "xor with 1's" -- for instance, if you had an 8-bit number "x", to compute "255-x" you could simply use (255 ^ x) instead -- without any of the evils of carrying like addition has. (In case it wasn't clear, 255 is eight "1s" in a row.)

So, to fix our floating point numbers, we define the following rules:


Always flip the sign bit.

If the sign bit was set, flip the other bits too.

To get back, we flip the sign bit always, and if the sign bit was not set, we flip the other bits too.
If we write this as a single xor, what we want is:


When the sign bit is set, xor with 0xFFFFFFFF (flip every bit)
When the sign bit is unset, xor with 0x80000000 (flip the sign bit)

This leads to two routines to convert floating point values to sortable numbers and back again. I call them FloatFlip and IFloatFlip: 
static inline uint32 FloatFlip(uint32 f)
{
	uint32 mask = -int32(f >> 31) | 0x80000000;
	return f ^ mask;
}

static inline uint32 IFloatFlip(uint32 f)
{
	uint32 mask = ((f >> 31) - 1) | 0x80000000;
	return f ^ mask;
}

This shifts the sign bit down 31 places (to make the entire number "0" or "1"), and then either inverts it or subtracts one. In particular, this always makes "0" or "0xFFFFFFFF" which becomes "0x80000000" or "0xFFFFFFFF" after or'ing in a 1.
Works nicely.


Wide radix
The third optimization notices that 11-bit histograms fit in L1, so we use 3 11-bit histograms, rather than the more standard 4 8-bit ones.
An initial implementation used 8-bit histograms, and this 11-bit optimization improves performance by about 40%.


Prefetch
The final optimization uses prefetch instructions (from the SSE instruction set) to optimize read access to memory. This gives an additional 25% speedup.

Putting it together
If done well, this code should be memory-bound. In this case, I think I fall a little bit short, though the fact that prefetch gave such a reasonable speedup means that I'm close to memory bandwidth.
My test case was: 

65536 floating-point numbers, 
randomly valued, 
sorted to an external array, 
testing on my P3/600.

My mergesort (from my class library) achieves about 12 sorts/sec on this test, approximately equivalent to the std::sort routine in Microsoft's implementation of the STL, distributed with Visual C++ (a quicksort, I believe.) Both of these implementations are considered incredibly well optimized.
After all the above optimizations, the radix achieves 97 sorts/sec on this test, a quite amazing improvement.

I hope the code is quite readable, so I'm just posting it here for everybody to try. You'd have to do some modifications to make it useful, but this is nice as a raw speed test.
*/

// Radix.cpp: a fast floating-point radix sort demo
//
//   Copyright (C) Herf Consulting LLC 2001.  All Rights Reserved.
//   Use for anything you want, just tell me what you do with it.
//   Code provided "as-is" with no liabilities for anything that goes wrong.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>	// QueryPerformanceCounter

// ------------------------------------------------------------------------------------------------
// ---- Basic types

typedef long int32;
typedef unsigned long uint32;
typedef float real32;
typedef double real64;
typedef unsigned char uint8;
typedef const char *cpointer;

// ------------------------------------------------------------------------------------------------
// Configuration/Testing

// ---- number of elements to test (shows tradeoff of histogram size vs. sort size)
const uint32 ct = 5000;

// ---- really, a correctness check, not correctness itself ;)
#define CORRECTNESS	1

// ---- use SSE prefetch (needs compiler support), not really a problem on non-SSE machines.
//		need http://msdn.microsoft.com/vstudio/downloads/ppack/default.asp
//		or recent VC to use this

#define PREFETCH 0

#if PREFETCH
#include <xmmintrin.h>	// for prefetch
#define pfval	64
#define pfval2	128
#define pf(x)	_mm_prefetch(cpointer(x + i + pfval), 0)
#define pf2(x)	_mm_prefetch(cpointer(x + i + pfval2), 0)
#else
#define pf(x)
#define pf2(x)
#endif

// ------------------------------------------------------------------------------------------------
// ---- Visual C++ eccentricities

#if _WINDOWS
#define finline __forceinline
#else
#define finline inline
#endif

// ================================================================================================
// flip a float for sorting
//  finds SIGN of fp number.
//  if it's 1 (negative float), it flips all bits
//  if it's 0 (positive float), it flips the sign only
// ================================================================================================
finline uint32 FloatFlip(uint32 f)
{
	uint32 mask = -int32(f >> 31) | 0x80000000;
	return f ^ mask;
}

finline void FloatFlipX(uint32 &f)
{
	uint32 mask = -int32(f >> 31) | 0x80000000;
	f ^= mask;
}

// ================================================================================================
// flip a float back (invert FloatFlip)
//  signed was flipped from above, so:
//  if sign is 1 (negative), it flips the sign bit back
//  if sign is 0 (positive), it flips all bits back
// ================================================================================================
finline uint32 IFloatFlip(uint32 f)
{
	uint32 mask = ((f >> 31) - 1) | 0x80000000;
	return f ^ mask;
}

// ---- utils for accessing 11-bit quantities
#define _0(x)	(x & 0x7FF)
#define _1(x)	(x >> 11 & 0x7FF)
#define _2(x)	(x >> 22 )

// ================================================================================================
// Main radix sort
// ================================================================================================
static void RadixSort11(real32 *farray, real32 *sorted, uint32 elements)
{
	uint32 i;
	uint32 *sort = (uint32*)sorted;
	uint32 *array = (uint32*)farray;

	// 3 histograms on the stack:
	const uint32 kHist = 2048;
	uint32 b0[kHist * 3];

	uint32 *b1 = b0 + kHist;
	uint32 *b2 = b1 + kHist;

	for (i = 0; i < kHist * 3; i++) {
		b0[i] = 0;
	}
	//memset(b0, 0, kHist * 12);

	// 1.  parallel histogramming pass
	//
	for (i = 0; i < elements; i++) {
		
		pf(array);

		uint32 fi = FloatFlip((uint32&)array[i]);

		b0[_0(fi)] ++;
		b1[_1(fi)] ++;
		b2[_2(fi)] ++;
	}
	
	// 2.  Sum the histograms -- each histogram entry records the number of values preceding itself.
	{
		uint32 sum0 = 0, sum1 = 0, sum2 = 0;
		uint32 tsum;
		for (i = 0; i < kHist; i++) {

			tsum = b0[i] + sum0;
			b0[i] = sum0 - 1;
			sum0 = tsum;

			tsum = b1[i] + sum1;
			b1[i] = sum1 - 1;
			sum1 = tsum;

			tsum = b2[i] + sum2;
			b2[i] = sum2 - 1;
			sum2 = tsum;
		}
	}

	// byte 0: floatflip entire value, read/write histogram, write out flipped
	for (i = 0; i < elements; i++) {

		uint32 fi = array[i];
		FloatFlipX(fi);
		uint32 pos = _0(fi);
		
		pf2(array);
		sort[++b0[pos]] = fi;
	}

	// byte 1: read/write histogram, copy
	//   sorted -> array
	for (i = 0; i < elements; i++) {
		uint32 si = sort[i];
		uint32 pos = _1(si);
		pf2(sort);
		array[++b1[pos]] = si;
	}

	// byte 2: read/write histogram, copy & flip out
	//   array -> sorted
	for (i = 0; i < elements; i++) {
		uint32 ai = array[i];
		uint32 pos = _2(ai);

		pf2(array);
		sort[++b2[pos]] = IFloatFlip(ai);
	}

	// to write original:
	// memcpy(array, sorted, elements * 4);
}

// Simple test of radix
static int test()
{
	uint32 i;

	const uint32 trials = 100;
	
	real32 *a = new real32[ct];
	real32 *b = new real32[ct];

	for (i = 0; i < ct; i++) {
		a[i] = real32(rand()) / 2048;
		if (rand() & 1) {
			a[i] = -a[i];
		}
	}

	LARGE_INTEGER s1, s2, f;
	QueryPerformanceFrequency(&f);
	QueryPerformanceCounter(&s1);


	for (i = 0; i < trials; i++) {
		RadixSort11(a, b, ct);
	}

	QueryPerformanceCounter(&s2);
	real64 hz = real64(f.QuadPart) / (s2.QuadPart - s1.QuadPart);
	printf("%d elements: %f/s\n", ct, hz * trials);

#if CORRECTNESS
	for (i = 1; i < ct; i++) {
		if (b[i - 1] >  b[i]) {
			printf("Wrong at %d\n", i);
		}

	}
#endif

	delete a;
	delete b;
	
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
