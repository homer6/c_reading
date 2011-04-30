#ifndef _SCRIPT_CLASSTAG_H
#define _SCRIPT_CLASSTAG_H


#include <script/VM.h>


namespace script
{


/** 
 * Class tag generator for applications with single script environment. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class T> class ClassTag
{
public:
	/** Returns unique class tag. */
	static int getTag( VM* vm )
	{
		if ( !sm_tag )
			sm_tag = vm->newTag();
		return sm_tag;
	}

private:
	static int sm_tag;
};


template <class T> int ClassTag<T>::sm_tag = 0;


} // script


#endif // _SCRIPT_CLASSTAG_H
