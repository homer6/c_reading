#ifndef _UTIL_HASH_H
#define _UTIL_HASH_H


namespace util
{


/** 
 * Default hash function object used by Hashtable.
 * Uses 'int K::hashCode()' for hashing
 * if the key is not basic arithmetic type.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class K> class Hash
{
public:
	int operator()( const K& x ) const
	{
		return x.hashCode();
	}
};


template <> class Hash<char> 
	{public: int operator()( const char& x ) const {return x;}};
template <> class Hash<short> 
	{public: int operator()( const short& x ) const {return x;}};
template <> class Hash<int> 
	{public: int operator()( const int& x ) const {return x;}};
template <> class Hash<float> 
	{public: int operator()( const float& x ) const {return *(int*)&x;}};
template <> class Hash<long> 
	{public: int operator()( const long& x ) const {return (int)x;}};


} // util


#endif // _UTIL_HASH_H
