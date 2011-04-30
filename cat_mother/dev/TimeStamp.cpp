#include "TimeStamp.h"
#include <assert.h>
#include <stdint.h>

#ifdef WIN32
#include <wtypes.h>
#include <mmsystem.h>
#pragma comment( lib, "winmm" )
#else
#include <time.h>
#endif

#include "config.h"

//-----------------------------------------------------------------------------

namespace dev
{


/** Time stamp query function prototype. */
typedef void (*Func)( unsigned long* lo, unsigned long* hi );

//-----------------------------------------------------------------------------

static double			s_ticksPerSec		= 1.0;
static Func				s_func				= 0;

//-----------------------------------------------------------------------------

/**
 * Reads time stamp. Intel Pentium compatible only.
 */
static void intelRCTS( unsigned long* lo, unsigned long* hi )
{
#if defined(_MSC_VER) && defined(_M_IX86)

	uint32_t lowerBits;
	uint32_t higherBits;
	__asm 
	{
		_emit 0x0F
		_emit 0x31
		// eax = bits 0-31
		// edx = bits 32-63
		lea ecx, lowerBits
		mov dword ptr [ecx], eax
		lea ecx, higherBits
		mov dword ptr [ecx], edx
	}

	*lo = lowerBits;
	*hi = higherBits;

#else

	*lo = 0;
	*hi = 0;
	throw int(1);

#endif
}

/**
 * Reads time stamp. Generic implementation.
 */
static void genericRCTS( unsigned long* lo, unsigned long* hi )
{
#ifdef WIN32
	*lo = timeGetTime();
	*hi = 0;
#else
	*lo = clock();
	*hi = 0;
#endif
}

/** 64-bit subtraction. */
static void sub64( unsigned long low1, unsigned long high1, unsigned long low2, unsigned long high2,
	unsigned long* lowOut, unsigned long* highOut )
{
	uint64_t d;
	d = high1;
	d <<= 32;
	d += low1;

	uint64_t s;
	s = high2;
	s <<= 32;
	s += low2;

	d -= s;

	*lowOut	= (uint32_t)( d );
	*highOut = (uint32_t)( d>>32 );
}

/** 64-bit addition. */
static void add64( unsigned long low1, unsigned long high1, unsigned long low2, unsigned long high2,
	unsigned long* lowOut, unsigned long* highOut )
{
	uint64_t d;
	d = high1;
	d <<= 32;
	d += low1;

	uint64_t s;
	s = high2;
	s <<= 32;
	s += low2;

	d += s;

	*lowOut	= (uint32_t)( d );
	*highOut = (uint32_t)( d>>32 );
}

//-----------------------------------------------------------------------------

static void tryTimeFunc( Func testFunc, Func* ok )
{
	try
	{
		unsigned long lo, hi;
		testFunc( &lo, &hi );
		*ok = testFunc;
	}
	catch ( ... ) {}
}

static void selectTimeFunc()
{
	double	ticksPerSec = 0;
	Func	func		= 0;

	// select default tick rate
#ifdef WIN32
	ticksPerSec = 1000.0;
#else
	ticksPerSec = CLOCKS_PER_SEC;
#endif

	// select time function
	func = genericRCTS;
	tryTimeFunc( intelRCTS, &func );

	// count ticks / second
	if ( func != genericRCTS )
	{
		// time0
		unsigned long lo1,hi1;
		func( &lo1, &hi1 );
		const double testTime = 0.5;

#ifdef WIN32
		DWORD c0 = timeGetTime();
		while ( double(timeGetTime()-c0)/double(1000) < testTime );
#else
		clock_t c0 = clock();
		while ( double(clock()-c0)/double(CLOCKS_PER_SEC) < 1.0 );
#endif

		// time1
		unsigned long lo2,hi2;
		func( &lo2, &hi2 );

		// dt = time1 - time0
		unsigned long dlo, dhi;
		sub64( lo2, hi2, lo1, hi1, &dlo, &dhi );

		ticksPerSec = ( double(dhi) * 4.294967296e9 + double(dlo) ) / testTime;
	}

	s_ticksPerSec = ticksPerSec;
	s_func = func;
}

//-----------------------------------------------------------------------------

TimeStamp::TimeStamp()
{
	if ( !s_func )
		selectTimeFunc();

	(*s_func)( &low, &high );
}

TimeStamp::TimeStamp( unsigned long low, unsigned long high )
{
	this->low	= low;
	this->high	= high;
}

TimeStamp TimeStamp::operator-=( const TimeStamp& other )
{
	*this = *this - other;
	return *this;
}

TimeStamp TimeStamp::operator+=( const TimeStamp& diff )
{
	*this = *this + diff;
	return *this;
}

TimeStamp TimeStamp::operator-( const TimeStamp& other ) const
{
	TimeStamp t(0,0);
	sub64( low, high, other.low, other.high, &t.low, &t.high );
	return t;
}

TimeStamp TimeStamp::operator+( const TimeStamp& other ) const
{
	TimeStamp t(0,0);
	add64( low, high, other.low, other.high, &t.low, &t.high );
	return t;
}

double TimeStamp::seconds() const
{
	assert( s_func );
	double d = double(high) * 4.294967296e9 + double(low);
	double secs = d / s_ticksPerSec;
	return secs;
}


} // dev
