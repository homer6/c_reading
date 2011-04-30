#ifndef _ABORTEXPORT_H
#define _ABORTEXPORT_H


#include <lang/Throwable.h>


/**
 * Class to be thrown if export is aborted.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class AbortExport : 
	public lang::Throwable 
{
};


#endif // _ABORTEXPORT_H
