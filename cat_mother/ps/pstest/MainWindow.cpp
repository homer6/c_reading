#include "MainWindow.h"

//-----------------------------------------------------------------------------

MainWindow::MainWindow()
{
}

MainWindow::~MainWindow()
{
}

LRESULT MainWindow::handleMessage( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	return FrameWindow::handleMessage( hwnd, msg, wp, lp );
}

void MainWindow::handleKeyDown( int key )
{
	switch ( key )
	{
	case VK_ESCAPE:	DestroyWindow( handle() ); break;
	}
}

void MainWindow::handleKeyUp( int /*key*/ )
{
}
