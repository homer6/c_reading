#ifndef _SD_ERRORS_H
#define _SD_ERRORS_H


namespace sd
{


/** 
 * Error codes returned by the sd driver. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
enum Error
{
	/** No error. */
	ERROR_NONE,
	/** Operation failed for unknown reason. */
	ERROR_GENERIC,
	/** Failed to create main DirectX object. */
	ERROR_DIRECTXNOTINSTALLED,
	/** No active window. */
	ERROR_NOWINDOW,
	/** Buffer memory was lost. */
	ERROR_BUFFERLOST,
};


} // sd


#endif // _SD_ERRORS_H
