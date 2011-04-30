#ifndef _ID_ERRORS_H
#define _ID_ERRORS_H


namespace id
{


/** 
 * Error codes returned by the gd driver. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
enum Error
{
	/** No error. */
	ERROR_NONE,
	/** Operation failed for unknown reason. */
	ERROR_GENERIC,
	/** A window must be active before create() is called. */
	ERROR_NOACTIVEWINDOW,
};


} // id


#endif // _ID_ERRORS_H
