#ifndef _WINDOW_H
#define _WINDOW_H


#include <lang/Object.h>

#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif


namespace win
{


/** 
 * Simple wrapper of Win32 window handle. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Window :
	public lang::Object
{
public:
	/** 
	 * Prepares for window creation. 
	 * Use create() to initialize the window. 
	 */
	Window();

	/** Destroys the window if not already destroyed. */
	virtual ~Window();

	/** 
	 * Creates the window. 
	 * @param className Type of the window to be created.
	 * @param name Title of the window to be created.
	 * @param width Width (in pixels) of the window to be created.
	 * @param height Height (in pixels) of the window to be created.
	 * @param style Style flags for the window.
	 * @param exStyle Extended style flags for the window.
	 * @param x X-coordinate of the top-left corner.
	 * @param y Y-coordinate of the top-left corner.
	 * @param w Width of the window (in pixels).
	 * @param h Height of the window (in pixels).
	 * @param instance Handle to application instance.
	 * @param parent Parent window if any.
	 * @param iconResourceId Icon resource ID or 0 if default app icon is used.
	 * @exception Exception
	 */
	void	create( const char* className, const char* name, 
				DWORD style, DWORD exStyle, int x, int y, int w, int h, 
				HINSTANCE instance,	Window* parent=0, int iconResourceId=0 );

	/** Destroys the window. */
	void	destroy();

	/** Returns Win32 window handle. */
	HWND	handle();

	/** 
	 * Called when window message arrives. 
	 * If the application doesn't handle the message,
	 * it should call implementation of the base class.
	 */
	virtual LRESULT		handleMessage( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp );

	/** Called by default handleMessage() to handle key down events. */
	virtual void		handleKeyDown( int key );

	/** Called by default handleMessage() to handle key up events. */
	virtual void		handleKeyUp( int key );

	/** Returns true if the window is active. */
	bool				active() const;

	/** 
	 * Flushes window message queue. 
	 * @return false if application quit was requested.
	 */
	static bool			flushWindowMessages();

private:
	HWND	m_hwnd;
	bool	m_active;

	Window( const Window& );
	Window& operator=( const Window& );
};


} // win


#endif // _WINDOW_H
