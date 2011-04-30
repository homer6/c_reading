#ifndef _GD_ERRORS_H
#define _GD_ERRORS_H


namespace gd
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
	/** Failed to create main DirectX object. */
	ERROR_DIRECTXNOTINSTALLED,
	/** No compatible depth buffer format. */
	ERROR_NODEPTH,
	/** No compatible stencil buffer format. */
	ERROR_NOSTENCIL,
	/** Cannot create stencil buffer without depth buffer. */
	ERROR_NOSTENCILWITHOUTSDEPTH,
	/** Failed to initialize full-screen window mode. */
	ERROR_FULLSCREENINITFAILED,
	/** Failed to initialize desktop window mode. */
	ERROR_DESKTOPINITFAILED,
	/** A window must be active before create() is called. */
	ERROR_NOACTIVEWINDOW,
	/** Effect description compilation failed. */
	ERROR_EFFECTCOMPILATIONERROR,
	/** Effect cannot be rendered on current device. */
	ERROR_EFFECTUNSUPPORTED,
};


} // gd


#endif // _GD_ERRORS_H
