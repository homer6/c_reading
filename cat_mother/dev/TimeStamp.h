#ifndef _DEV_TIMESTAMP_H
#define _DEV_TIMESTAMP_H


namespace dev
{


/** 
 * CPU time stamp.
 * Accuracy is at least as good as in standard library clock() function, 
 * but accuracy up to processor clock speed can be achieved
 * with Intel Pentium compatible CPUs.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class TimeStamp
{
public:
	/** Bits 0-31 of the time stamp. */
	unsigned long	low;

	/** Bits 32-63 of the time stamp. */
	unsigned long	high;

	/** Initializes time stamp with current time. */
	TimeStamp();

	/** Creates time stamp from pair of values. */
	TimeStamp( unsigned long low, unsigned long high );

	/** Subtracts other time stamp from this time stamp. */
	TimeStamp			operator-=( const TimeStamp& other );

	/** Adds other time stamp to this time stamp. */
	TimeStamp			operator+=( const TimeStamp& other );

	/** Returns result of other time stamp subtracted from this time stamp. */
	TimeStamp			operator-( const TimeStamp& other ) const;

	/** Returns result of other time stamp added to this time stamp. */
	TimeStamp			operator+( const TimeStamp& other ) const;

	/** 
	 * Returns time stamp in seconds. 
	 * Note that the result is meaningful only for time stamp differences.
	 */
	double				seconds() const;
};


} // dev


#endif // _DEV_TIMESTAMP_H
