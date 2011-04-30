#ifndef _SCRIPTUTIL_H
#define _SCRIPTUTIL_H


#include "ScriptMethod.h"
#include <script/Scriptable.h>
#include <string.h>
#include <assert.h>


/** 
 * Helper functions for scripting.
 * @param T Derived scriptable class.
 * @param B Base scriptable class.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class T, class B> class ScriptUtil
{
public:
	/** 
	 * Adds n C++ methods to be usable from the script.
	 * @return Base index for the added methods.
	 */
	static int addMethods( T* obj, ScriptMethod<T>* methods, int n )
	{
		// check that the same method is not added twice
		for ( int i = 0 ; i < n ; ++i )
		{
			for ( int j = i+1 ; j < n ; ++j )
			{
				assert( methods[i].func() != methods[j].func() );
				assert( strcmp( methods[i].name(), methods[j].name() ) );
			}
		}

		int methodBase = 0;
		if ( n > 0 )
		{
			methodBase = obj->addMethod( methods[0].name() );
			for ( int i = 1 ; i < n ; ++i )
				obj->addMethod( methods[i].name() );
		}
		return methodBase;
	}

	/**
	 * Calls C++ method. If the index is out of range then base
	 * class (B) methodCall is called.
	 * @param obj The object calling this method.
	 * @param vm Script virtual machine requesting the method execution.
	 * @param i Index of the method to be executed.
	 * @param methodBase Base index for methods of this class.
	 * @param methods An array of methods of this class.
	 * @param n Number of methods in the array.
	 * @return Number of return values in the script stack.
	 */
	static int methodCall( T* obj, script::VM* vm, int i, int methodBase, ScriptMethod<T>* methods, int n )
	{
		int index = i - methodBase;
		if ( index >= 0 && index < n )
			return ( obj->*methods[index].func() )( vm, methods[index].name() );
		else
			return obj->B::methodCall( vm, i );
	}
};


#endif // _SCRIPTUTIL_H
