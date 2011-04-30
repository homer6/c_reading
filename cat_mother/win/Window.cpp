#include "StdAfx.h"
#include "Window.h"
#include <lang/Exception.h>
#include <stdio.h>
#include <stdarg.h>

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace win
{


struct WINDOWSETUPPARAM
{
	Window*		window;

	WINDOWSETUPPARAM()
	{
		window = 0;
	}
};

//-----------------------------------------------------------------------------

static void dprintf( const char* fmt, ... )
{
	char buff[1024];
	va_list marker;
	va_start( marker, fmt );
	_vsnprintf( buff, sizeof(buff), fmt, marker );
	va_end( marker );
	OutputDebugStringA( buff );
}

static LRESULT CALLBACK wndProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	if ( WM_CREATE == msg )
	{
		char name[256];
		GetWindowText( hwnd, name, sizeof(name) );
		dprintf( "Creating window \"%s\"\n", name );

		LPCREATESTRUCT cs = (LPCREATESTRUCT)lp;
		WINDOWSETUPPARAM* param = (WINDOWSETUPPARAM*)cs->lpCreateParams;
		SetWindowLong( hwnd, GWL_USERDATA, (long)param->window );
	}
	else
	{
		if ( WM_DESTROY == msg )
		{
			char name[256];
			GetWindowText( hwnd, name, sizeof(name) );
			dprintf( "Destroying window \"%s\"\n", name );
		}

		Window* win = (Window*)GetWindowLong( hwnd, GWL_USERDATA );
		if ( win )
		{
			try
			{
				return win->handleMessage( hwnd, msg, wp, lp );
			}
			catch ( Throwable& e )
			{
				char msg[256];
				memset( msg, 0, sizeof(msg) );
				e.getMessage().format().getBytes( msg, sizeof(msg), "ASCII-7" );
				dprintf( "Exception caught in wndProc: \"%s\"\n", msg );
			}
		}
	}
	return DefWindowProc( hwnd, msg, wp, lp );
}

//-----------------------------------------------------------------------------

Window::Window() :
	m_hwnd(0), m_active(false)
{
}

Window::~Window()
{
	destroy();
}

void Window::create( const char* className, const char* name, 
	DWORD style, DWORD exStyle, int x, int y, int w, int h, HINSTANCE instance, Window* parent, int iconResourceId )
{
	HWND parentHwnd = 0;
	if ( parent )
		parentHwnd = parent->m_hwnd;

	HICON icon = 0;
	if ( iconResourceId > 0 )
		icon = LoadIcon( instance, MAKEINTRESOURCE(iconResourceId) );

	WNDCLASSEXA wcex;
	wcex.cbSize			= sizeof(WNDCLASSEXA); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)wndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= (HINSTANCE)instance;
	wcex.hIcon			= icon;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= className;
	wcex.hIconSm		= 0;
	if ( !RegisterClassExA(&wcex) )
		throw Exception( Format("Failed to register a window class.") );

	WINDOWSETUPPARAM param;
	param.window = this;

	m_hwnd = CreateWindowExA( exStyle, className, name, style, x, y, w, h, parentHwnd, 0, instance, &param );
	if ( !m_hwnd )
	{
		UnregisterClass( className, instance );
		throw Exception( Format("Failed to create a window.") );
	}
}

void Window::destroy()
{
	if ( m_hwnd )
	{
		DestroyWindow( m_hwnd );
		m_hwnd = 0;
	}
}

bool Window::flushWindowMessages()
{
	MSG msg;
	if ( PeekMessage(&msg,0,0,0,PM_NOREMOVE) )
	{
		while ( WM_QUIT != msg.message && PeekMessage(&msg,0,0,0,PM_REMOVE) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		return WM_QUIT != msg.message;
	}
	return true;
}

LRESULT Window::handleMessage( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	switch ( msg )
	{
	case WM_KEYDOWN:
		handleKeyDown( wp );
		break;

	case WM_KEYUP:
		handleKeyUp( wp );
		break;

	case WM_ACTIVATE:{
		m_active = ( LOWORD(wp) != WA_INACTIVE );
		
		char name[256];
		GetWindowText( hwnd, name, sizeof(name) );
		dprintf( "Window \"%s\" %s\n", name, (m_active?"activated":"deactivated") );
		break;}

	case WM_DESTROY:{
		m_hwnd = 0;
		PostQuitMessage( 0 );
		break;}
	}

	return DefWindowProc( hwnd, msg, wp, lp );
}

void Window::handleKeyDown( int )
{
}

void Window::handleKeyUp( int )
{
}

HWND Window::handle()
{
	return m_hwnd;
}

bool Window::active() const
{
	return m_active;
}


} // win
