#include "stdafx.h"
#include "sgviewer.h"
#include "MainFrm.h"
#include "sgviewerDoc.h"
#include "sgviewerView.h"
#include <sg/Context.h>
#include <sg/Texture.h>
#include <io/File.h>
#include <io/FileInputStream.h>
#include <io/FileOutputStream.h>
#include <dev/Profile.h>
#include <lang/Float.h>
#include <lang/Debug.h>
#include <lang/Integer.h>
#include <lang/Exception.h>
#include <util/Properties.h>
#include <direct.h>
#include "build.h"
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace io;
using namespace dev;
using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CSgviewerApp, CWinApp)
	//{{AFX_MSG_MAP(CSgviewerApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------

static Context::TextureFilterType toTextureFilterType( String str )
{
	Context::TextureFilterType filter = Context::TEXF_NONE;
	str = str.toLowerCase();
	
	if ( str == "point" )
		filter = Context::TEXF_POINT;
	else if ( str == "linear" )
		filter = Context::TEXF_LINEAR;
	else if ( str == "anisotropic" )
		filter = Context::TEXF_ANISOTROPIC;

	return filter;
}

//-----------------------------------------------------------------------------

CSgviewerApp theApp;

//-----------------------------------------------------------------------------

CSgviewerApp::CSgviewerApp() :
	m_defMovSpeeds( Allocator<float>(__FILE__,__LINE__) ),
	m_defRotSpeeds( Allocator<float>(__FILE__,__LINE__) )
{
	context = 0;
	font = 0;
	active = true;
	m_pause = 0;
	m_quit = false;
}	

CSgviewerApp::~CSgviewerApp()
{
	if ( context )
	{
		context->destroy();
		context = 0;
	}
}

BOOL CSgviewerApp::InitInstance()
{
	// load graphics driver
	try 
	{
		// Standard initialization
		// If you are not using these features and wish to reduce the size
		//  of your final executable, you should remove from the following
		//  the specific initialization routines you do not need.

		Debug::println( "------------------------------------------------------------------------" );
		Debug::println( "sgviewer Build {0}", BUILD_NUMBER );
		Debug::println( "------------------------------------------------------------------------" );

		// Change the registry key under which our settings are stored.
		// TODO: You should modify this string to be something appropriate
		// such as the name of your company or organization.
		SetRegistryKey( "Cat_Mother" );

		// set build number
		int buildNum = GetProfileInt( "sgviewer", "BuildNumber", -1 );
		if ( buildNum != BUILD_NUMBER )
		{
			WriteProfileInt( "sgviewer", "BuildNumber", BUILD_NUMBER );
			setPropPathToCwd();
		}

		// load app properties
		if ( !File(getExePathFilename("sgviewer.prop")).exists() )
		{
			String old = getExePathFilename("sgviewer.prop");
			setPropPathToCwd();
			Debug::println( "File {0} not exist, loading properties from {1}", old, getExePathFilename("sgviewer.prop") );
		}
		else
		{
			Debug::println( "Loading properties from {0}", getExePathFilename("sgviewer.prop") );
		}
		FileInputStream in( getExePathFilename("sgviewer.prop") );
		m_props.load( &in );
		in.close();

		if ( !m_props.containsKey("RenderingDriver") )
			throw Exception( Format("RenderingDriver not set in properties") );
		context = new Context( m_props.get("RenderingDriver") );

		m_defMovSpeeds.add( m_props.getFloat("MovementSpeed1") );
		m_defMovSpeeds.add( m_props.getFloat("MovementSpeed2") );
		m_defMovSpeeds.add( m_props.getFloat("MovementSpeed3") );
		m_defMovSpeeds.add( m_props.getFloat("MovementSpeed4") );
		m_defRotSpeeds.add( m_props.getFloat("RotationSpeed1") );
		m_defRotSpeeds.add( m_props.getFloat("RotationSpeed2") );
		m_defRotSpeeds.add( m_props.getFloat("RotationSpeed3") );
		m_defRotSpeeds.add( m_props.getFloat("RotationSpeed4") );

#ifdef _AFXDLL
		Enable3dControls();			// Call this when using MFC in a shared DLL
#else
		Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

		LoadStdProfileSettings();  // Load standard INI file options (including MRU)

		// Register the application's document templates.  Document templates
		//  serve as the connection between documents, frame windows and views.

		CSingleDocTemplate* pDocTemplate;
		pDocTemplate = new CSingleDocTemplate(
			IDR_MAINFRAME,
			RUNTIME_CLASS(CSgviewerDoc),
			RUNTIME_CLASS(CMainFrame),       // main SDI frame window
			RUNTIME_CLASS(CSgviewerView));
		AddDocTemplate(pDocTemplate);

		// Enable DDE Execute open
		EnableShellOpen();
		RegisterShellFileTypes(TRUE);

		// Parse command line for standard shell commands, DDE, file open
		CCommandLineInfo cmdInfo;
		ParseCommandLine(cmdInfo);

		// Dispatch commands specified on the command line
		if (!ProcessShellCommand(cmdInfo))
			return FALSE;
		if ( m_quit )
		{
			if ( context )
				context->destroy();
			context = 0;
			return FALSE;
		}

		// The one and only window has been initialized, so show and update it.
		m_pMainWnd->ShowWindow( SW_SHOW );
		m_pMainWnd->UpdateWindow();
		
		// adjust window size
		int sw = defaultWidth();
		int sh = defaultHeight();
		RECT cr;
		m_pMainWnd->GetClientRect( &cr );
		RECT wr;
		m_pMainWnd->GetWindowRect( &wr );
		int dx = sw - cr.right;
		int dy = sh - cr.bottom;
		m_pMainWnd->MoveWindow( wr.left-dx/2, wr.top-dy/2, wr.right-wr.left+dx, wr.bottom-wr.top+dy, TRUE );

		// Enable drag/drop open
		m_pMainWnd->DragAcceptFiles();

		// init rendering device

		// settings
		if ( m_props.getBoolean("Textures16bit") )
			Texture::setDefaultBitDepth( 16 );
		if ( m_props.getBoolean("HalfTextureResolution") )
			Texture::setDownScaling( true );

		// frame buffer surfaces
		int sf = Context::SURFACE_TARGET + Context::SURFACE_DEPTH;
		if ( m_props.getBoolean("StencilEnabled") )
			sf += Context::SURFACE_STENCIL;
		
		// rasterizer
		Context::RasterizerType rz = Context::RASTERIZER_HW;
		if ( m_props.getBoolean("RefRasterizer") )
			rz = Context::RASTERIZER_SW;

		// vertex processing
		Context::VertexProcessingType vp = Context::VERTEXP_SW;
		if ( m_props.getBoolean("HWTnL") && rz == Context::RASTERIZER_HW )
			vp = Context::VERTEXP_HW;

		// automatic texture compression
		Context::TextureCompressionType tc = Context::TC_NONE;
		if ( m_props.getBoolean("AutoTextureCompression") )
			tc = Context::TC_COMPRESSED;

		context->open( defaultWidth(), defaultHeight(), m_props.getInteger("BitsPerPixel"), m_props.getInteger("Refresh"), sf, rz, vp, tc );
		context->setMipMapFilter( toTextureFilterType(m_props["MipMapFilter"]) );
		context->setMipMapLODBias( m_props.getFloat("MipMapLODBias") );

		// remove menu in fullscreen mode
		if ( context->fullscreen() )
			m_pMainWnd->SetMenu( 0 );

		// load default font
		FileInputStream fontImageInputStream( getExePathFilename( "arial.bmp" ) );
		FileInputStream fontCharsetInputStream( getExePathFilename( "arial.txt" ) );
		font = new Font( &fontImageInputStream, &fontCharsetInputStream );
		fontCharsetInputStream.close();
		fontImageInputStream.close();

		// init normalizer cube texture
		FileInputStream texIn( getExePathFilename("normalize.dds") );
		normalizerCubeMap = new CubeTexture( &texIn );
		texIn.close();

		// init lightmap shader
		/*{FileInputStream fxin( getExePathFilename("lightmap.fx") );
		lightmapShader = new Effect( &fxin );
		fxin.close();}*/
	}
	catch ( Throwable& e )
	{
		if ( context )
			context->close();
		
		char buf[1000];
		e.getMessage().format().getBytes( buf, sizeof(buf), "ASCII-7" );
		::MessageBox( 0, buf, "Error - sgviewer", MB_OK|MB_ICONERROR|MB_SYSTEMMODAL );
		
		if ( m_pMainWnd )
			m_pMainWnd->DestroyWindow();
		m_pMainWnd = 0;
		return FALSE;
	}

	m_pMainWnd->Invalidate();
	return TRUE;
}

int CSgviewerApp::ExitInstance()
{
	lightmapShader = 0;
	normalizerCubeMap = 0;
	font = 0;

	if ( context )
		context->destroy();
	context = 0;

	return CWinApp::ExitInstance();
}

CSgviewerApp& CSgviewerApp::getApp()
{
	return theApp;
}

void CSgviewerApp::setPause( bool paused )
{
	if ( paused )
		++m_pause;
	else
		--m_pause;

	if ( !paused )
	{
		if ( m_pMainWnd )
		{
			CFrameWnd* frame = (CFrameWnd*)m_pMainWnd;
			CSgviewerView* view = (CSgviewerView*)frame->GetActiveView();
			if ( view )
				view->pauseTime();
			m_pMainWnd->InvalidateRect( 0, FALSE );
		}
	}
}

void CSgviewerApp::setStats( bool enabled )
{
	m_props["Statistics"] = String::valueOf( (enabled?1:0) );
}

bool CSgviewerApp::pause() const
{
	return 0 != m_pause || !active;
}

bool CSgviewerApp::stats() const
{
	return m_props.getBoolean("Statistics");
}

int CSgviewerApp::defaultWidth() const
{
	return m_props.getFloat("Width");
}

int CSgviewerApp::defaultHeight() const
{
	return m_props.getFloat("Height");
}

float CSgviewerApp::front() const
{
	return m_props.getFloat("Front");
}

float CSgviewerApp::back() const
{
	return m_props.getFloat("Back");
}

float CSgviewerApp::getMovementSpeed( int n ) const
{
	return m_defMovSpeeds[n];
}

int CSgviewerApp::movementSpeeds() const
{
	return m_defMovSpeeds.size();
}

float CSgviewerApp::getRotationSpeed( int n ) const
{
	return m_defRotSpeeds[n];
}

int CSgviewerApp::rotationSpeeds() const
{
	return m_defRotSpeeds.size();
}

int	CSgviewerApp::debugLines() const
{
	return m_props.getInteger("MaxDebugLines");
}

float CSgviewerApp::splitPrimitivePolygons() const
{
	return m_props.getFloat("SplitPrimitivePolygons");
}

float CSgviewerApp::splitPrimitiveSize() const
{
	return m_props.getFloat("SplitPrimitiveSize");
}

bool CSgviewerApp::profiling() const
{
	return m_props.getBoolean("Profiling");
}

const util::ExProperties& CSgviewerApp::prop() const 
{
	return m_props;
}

void CSgviewerApp::setPropPathToCwd()
{
	char cwd[2048];
	_getcwd( cwd, sizeof(cwd) );
	BOOL ok = WriteProfileString( "sgviewer", "PropPath", cwd );
	if ( ok )
		Debug::println( "Wrote sgviewer.prop path to {0}", cwd );
}

String CSgviewerApp::getExePathFilename( const String& fname )
{
	// get name of the exe
	/*const int bufferSize = 1000;
	char buffer[bufferSize];
	int len = GetModuleFileName( GetModuleHandle(0), buffer, bufferSize-1 );
	buffer[len] = 0;

	// get exe directory
	String exeDir = File(buffer).getParent();*/

	// get prop directory
	char cwd[2048];
	_getcwd( cwd, sizeof(cwd) );
	String exeDir = (String)GetProfileString( "sgviewer", "PropPath", cwd );

	// return ../fname or ./fname, depenending on which exists
	// (prefer ./fname)
	File fh( exeDir, fname );
	if ( fh.exists() )
		return fh.getPath();
	File parent( File(fh.getParent()).getParent(), fname );
	if ( parent.exists() )
		return parent.getPath();
	return fh.getPath();
}

//-----------------------------------------------------------------------------

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCancelMode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_WM_CANCELMODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CSgviewerApp::OnAppAbout()
{
	setPause( true );
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
	setPause( false );
}

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	// set version text
	char vertext[200];
	sprintf( vertext, "sgviewer Build %i", BUILD_NUMBER );
	CWnd* verwin = GetDlgItem(IDC_VER);
	if ( verwin )
		verwin->SetWindowText( vertext );
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAboutDlg::OnCancelMode() 
{
	CDialog::OnCancelMode();
	
	// TODO: Add your message handler code here
}

