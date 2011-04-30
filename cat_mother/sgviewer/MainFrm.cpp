// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "sgviewer.h"
#include "MainFrm.h"
#include "config.h"

using namespace sg;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_ENTERMENULOOP()
	ON_WM_EXITMENULOOP()
	ON_WM_SYSCOMMAND()
	ON_WM_ACTIVATE()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

static LRESULT CALLBACK dummyWindowProc( HWND, UINT, WPARAM, LPARAM ) {return 0;}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	int w = CSgviewerApp::getApp().defaultWidth();
	int h = CSgviewerApp::getApp().defaultHeight();
	cs.x = (GetSystemMetrics(SM_CXSCREEN)-w)/2;
	cs.y = (GetSystemMetrics(SM_CYSCREEN)-h)/2;
	cs.cx = w;
	cs.cy = h;

	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnDestroy() 
{
	if ( CSgviewerApp::getApp().context )
		CSgviewerApp::getApp().context->close();

	CFrameWnd::OnDestroy();
}

void CMainFrame::OnEnterMenuLoop( BOOL b )
{
	//CFrameWnd::OnEnterMenuLoop( b );
	CSgviewerApp::getApp().setPause( true );
}

void CMainFrame::OnExitMenuLoop( BOOL b )
{
	//CFrameWnd::OnExitMenuLoop( b );
	CSgviewerApp::getApp().setPause( false );
}

BOOL CMainFrame::OnEraseBkgnd(CDC* pDC) 
{
	//return CFrameWnd::OnEraseBkgnd(pDC);
	return TRUE;
}

void CMainFrame::OnSysCommand( UINT nID, LPARAM lParam )
{
	switch ( nID )
	{
	case SC_CONTEXTHELP:
	case SC_DEFAULT:
	case SC_HOTKEY:
	case SC_HSCROLL:
	case SC_KEYMENU:
	case SC_MAXIMIZE:
	case SC_MINIMIZE:
	case SC_MONITORPOWER:
	case SC_MOUSEMENU:
	case SC_MOVE:
	case SC_NEXTWINDOW:
	case SC_PREVWINDOW:
	case SC_RESTORE:
	case SC_SCREENSAVE:
	case SC_SIZE:
	case SC_TASKLIST:
	case SC_VSCROLL:
		if ( CSgviewerApp::getApp().context && CSgviewerApp::getApp().context->fullscreen() )
			break;
	default:
		CFrameWnd::OnSysCommand( nID, lParam );
		break;
	}
}

void CMainFrame::OnActivate( UINT state, CWnd* other, BOOL minimized )
{
	CSgviewerApp::getApp().active = (state != WA_INACTIVE);
}

void CMainFrame::OnKeyDown( UINT ch, UINT rep, UINT flags )
{
	if ( 'T' == ch )
	{
		CSgviewerApp::getApp().setStats( !CSgviewerApp::getApp().stats() );
	}
	else if ( VK_ESCAPE == ch )
	{
		CSgviewerApp::getApp().m_pMainWnd->DestroyWindow();
	}
}
