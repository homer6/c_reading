#ifndef _UTIL_HASHTABLEPAIR_H
#define _UTIL_HASHTABLEPAIR_H


namespace util
{


/** 
 * Data structure used by the Hashtable implementation. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class K, class T> class HashtablePair
{
public:
	K					key;
	T					value;
	HashtablePair<K,T>*	next;
	bool				used;

	HashtablePair()	
	{	
		defaults();
	}

	void defaults()
	{
		next = 0;
		used = false;
	}
};


} // util


#endif // _UTIL_HASHTABLEPAIR_H
