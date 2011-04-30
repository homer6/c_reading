#include "StdAfx.h"
#include "ProgressDialog.h"
#include "resource.h"
#include "AbortExport.h"
#include "WinUtil.h"
#include <lang/Debug.h>
#include <lang/Exception.h>

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

static BOOL CALLBACK dlgProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	ProgressDialog* dlg = (ProgressDialog*)GetWindowLongPtr( hwnd, GWLP_USERDATA );

	switch ( msg ) 
	{
	case WM_INITDIALOG:{
		dlg = (ProgressDialog*)lp;
		SetWindowLongPtr( hwnd, GWLP_USERDATA, lp );
		CenterWindow( hwnd, GetParent(hwnd) );
		break;}

	case WM_COMMAND:{
		switch ( LOWORD(wp) ) 
		{
		case IDABORT:{
			dlg->abort();
			break;}
		}
		break;}

	default:
		return FALSE;
	}
	return TRUE;
}       

//-----------------------------------------------------------------------------

ProgressDialog::ProgressDialog( HINSTANCE instance, HWND parent )
{
	m_hwnd = CreateDialogParam( instance, MAKEINTRESOURCE(IDD_PROGRESS),
		parent, dlgProc, (LPARAM)this );

	if ( !m_hwnd )
		throw Exception( Format("Failed to create progress dialog") );

	m_taskdesc = GetDlgItem( m_hwnd, IDC_TASKDESC );
	require( m_taskdesc );
	m_pbar = GetDlgItem( m_hwnd, IDC_PROGRESS1 );
	
	require( m_pbar );
	SendMessage( m_pbar, PBM_SETRANGE, 0, MAKELPARAM(0,1000) );

	ShowWindow( m_hwnd, SW_NORMAL );

	m_abort = false;
}

ProgressDialog::~ProgressDialog()
{
	destroy();
}

void ProgressDialog::destroy()
{
	if ( m_hwnd )
	{
		DestroyWindow( m_hwnd );
		m_hwnd = 0;
	}
}

void ProgressDialog::setText( const lang::String& text )
{
	check();

	char buf[256];
	text.getBytes( buf, sizeof(buf), "ASCII-7" );
	SetWindowText( m_taskdesc, buf );
	Debug::println( "{0}", text );

	update();
}

void ProgressDialog::setProgress( float pos )
{
	require( pos >= 0.f && pos <= 1.f );

	check();

	SendMessage( m_pbar, PBM_SETPOS, (int)(pos*1000.f), 0 );
	
	update();
}

void ProgressDialog::abort()
{
	m_abort = true;
}

void ProgressDialog::check()
{
	if ( m_abort || !m_hwnd )
		throw AbortExport();

	if ( !WinUtil::flushWindowMessages() )
		throw AbortExport();
}

void ProgressDialog::update()
{
	UpdateWindow( m_hwnd );
}
