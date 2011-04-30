#include <memory.h>


template <class T> static void zero( T& v )
{
	memset( &v, 0, sizeof(v) );
}
