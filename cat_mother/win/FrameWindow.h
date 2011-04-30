#ifndef _WIN_FRAMEWINDOW_H
#define _WIN_FRAMEWINDOW_H


#include <win/Window.h>


namespace win
{


/** 
 * Base class for application main window. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class FrameWindow :
	public win::Window
{
public:
	///
	FrameWindow();

	///
	~FrameWindow();

	/** 
	 * Creates the main window. 
	 * @param className Type of the window to be created.
	 * @param name Title of the window to be created.
	 * @param w Width (in pixels) of the window to be created.
	 * @param h Height (in pixels) of the window to be created.
	 * @param popup If true then create borderless topmost popup window.
	 * @param instance Handle to application instance.
	 * @param iconResourceId Icon resource ID or 0 if default app icon is used.
	 * @exception Exception
	 */
	void	create( const char* className, const char* name,
				int w, int h, bool popup, HINSTANCE instance, int iconResourceId=0 );

private:
	FrameWindow( const FrameWindow& );
	FrameWindow& operator=( const FrameWindow& );
};


} // win


#endif // _WIN_FRAMEWINDOW_H
