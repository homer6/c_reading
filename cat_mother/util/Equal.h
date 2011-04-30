#ifndef _UTIL_EQUAL_H
#define _UTIL_EQUAL_H


namespace util
{


/** 
 * Predicate for testing if two objects are equal. Uses operator==. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class T> class Equal
{
public:
	bool operator()( const T& a, const T& b ) const
	{
		return a == b;
	}
};


} // util


#endif // _UTIL_EQUAL_H
