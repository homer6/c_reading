#include "StdAfx.h"
#include "FrameWindow.h"
#include "config.h"

//-----------------------------------------------------------------------------

namespace win
{


FrameWindow::FrameWindow()
{
}

FrameWindow::~FrameWindow()
{
}

void FrameWindow::create( const char* className, const char* name,
	int w, int h, bool popup, HINSTANCE instance, int iconResourceId )
{
	// create main window
	DWORD style = WS_VISIBLE;
	DWORD styleEx = 0;
	if ( popup )
	{
		style |= WS_POPUP;
		styleEx |= WS_EX_TOPMOST;
	}
	else
	{
		style |= WS_OVERLAPPEDWINDOW;
	}
	Window::create( className, name, style, styleEx, 0, 0, w, h, instance, 0, iconResourceId );

	// adjust window size
	RECT cr;
	GetClientRect( handle(), &cr );
	RECT wr;
	GetWindowRect( handle(), &wr );
	MoveWindow( handle(), wr.left, wr.top, w+(w-cr.right), h+(h-cr.bottom), TRUE );

	// hide cursor if popup window
	if ( popup )
		ShowCursor( FALSE );
}


} // win
