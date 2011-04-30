#include <util/Vector.h>
#include <assert.h>


namespace ps
{


template <class T> void swapRemove( util::Vector<T>& arr, int index )
{
	assert( index >= 0 && index < arr.size() );
	arr[index] = arr[arr.size()-1];
	arr.setSize( arr.size() - 1 );
}


} // ps
