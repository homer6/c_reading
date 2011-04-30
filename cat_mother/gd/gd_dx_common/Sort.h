#ifndef _SORT_H
#define _SORT_H


/** 
 * Sort utilities. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Sort
{
public:
	/**
	 * Sorts array using buckets. O(n) algorithm. Sort accuracy is 8 bits.
	 * @param begin Beginning of the array to sort.
	 * @param end End of the array to sort.
	 * @param buffer Temporary buffer. Must be at least TWICE the size of input.
	 * @param order [out] Receives ascending order. (indices to the source data)
	 */
	static void bucketSort( const float* begin, const float* end, 
		int* buffer, int* order );

	/**
	 * Sorts array using std::sort. O(n logn) algorithm. Sort accuracy is float.
	 * @param begin Beginning of the array to sort.
	 * @param end End of the array to sort.
	 * @param buffer Temporary buffer. Must be at least TWICE the size of input.
	 * @param order [out] Receives ascending order. (indices to the source data)
	 */
	static void stdSort( const float* begin, const float* end, 
		int* buffer, int* order );
};


#endif // _SORT_H
