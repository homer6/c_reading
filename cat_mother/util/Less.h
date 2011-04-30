#ifndef _UTIL_LESS_H
#define _UTIL_LESS_H


namespace util
{


/** 
 * Predicate for testing if the first object is less than the second. Uses operator<. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class T> class Less
{
public:
	bool operator()( const T& a, const T& b ) const
	{
		return a < b;
	}
};


} // util


#endif // _UTIL_LESS_H
