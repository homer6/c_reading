#include "StdAfx.h"
#include "WinUtil.h"

//-----------------------------------------------------------------------------

bool WinUtil::flushWindowMessages()
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
