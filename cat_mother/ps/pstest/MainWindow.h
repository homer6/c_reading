#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H


#include <win/FrameWindow.h>


namespace util {
	class ExProperties;}


/** Prototype main window. */
class MainWindow :
	public win::FrameWindow
{
public:
	MainWindow();
	~MainWindow();

	LRESULT		handleMessage( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp );
	void		handleKeyDown( int key );
	void		handleKeyUp( int key );

private:
	MainWindow( const MainWindow& );
	MainWindow& operator=( const MainWindow& );
};


#endif // _MAINWINDOW_H
