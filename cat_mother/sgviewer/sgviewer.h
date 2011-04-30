// sgviewer.h : main header file for the SGVIEWER application
//

#if !defined(AFX_SGVIEWER_H__D53EDADF_A166_11D5_BC88_0000B49545E8__INCLUDED_)
#define AFX_SGVIEWER_H__D53EDADF_A166_11D5_BC88_0000B49545E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include <sg/Font.h>
#include <sg/Effect.h>
#include <sg/Context.h>
#include <sg/CubeTexture.h>
#include <util/Vector.h>
#include <util/ExProperties.h>

/////////////////////////////////////////////////////////////////////////////
// CSgviewerApp:
// See sgviewer.cpp for the implementation of this class
//

class CSgviewerApp : 
	public CWinApp
{
public:
	/** Rendering context. */
	P(sg::Context)	context;

	/** Default font. */
	P(sg::Font)		font;

	/** Global normalizer cube texture. */
	P(sg::CubeTexture)	normalizerCubeMap;

	/** Global lightmap shader. Use by cloning. */
	P(sg::Effect)		lightmapShader;

	/** True if the app window is active. */
	bool	active;

	/** Toggles rendering. */
	void	setPause( bool paused );

	/** Toggles statistics. */
	void	setStats( bool enabled );

	/** Sets property path to current working directory. */
	void	setPropPathToCwd();

	/** Returns true if rendering is disabled. */
	bool	pause() const;

	/** Returns true if stats are enabled. */
	bool	stats() const;

	/** Returns default window width. */
	int		defaultWidth() const;

	/** Returns default window height. */
	int		defaultHeight() const;

	/** Returns front plane distance. */
	float	front() const;

	/** Returns back plane distance. */
	float	back() const;

	/** Returns nth movement speed. */
	float	getMovementSpeed( int n ) const;

	/** Returns number of different movement speeds. */
	int		movementSpeeds() const;

	/** Returns nth rotation speed. */
	float	getRotationSpeed( int n ) const;

	/** Returns number of different rotation speeds. */
	int		rotationSpeeds() const;

	/** Returns debug line count. */
	int		debugLines() const;

	/** Returns true if profiling is enabled. */
	bool	profiling() const;

	/** Returns minimum number of polygons in a primitive to be splitted. */
	float	splitPrimitivePolygons() const;

	/** Returns minimum size of a primitive to be splitted. */
	float	splitPrimitiveSize() const;

	static CSgviewerApp& getApp();

	const util::ExProperties&	prop() const;

private:
	int					m_pause;
	bool				m_quit;
	util::ExProperties	m_props;
	util::Vector<float>	m_defMovSpeeds;
	util::Vector<float>	m_defRotSpeeds;

public:
	CSgviewerApp();
	~CSgviewerApp();

	lang::String getExePathFilename( const lang::String& fname );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSgviewerApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CSgviewerApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SGVIEWER_H__D53EDADF_A166_11D5_BC88_0000B49545E8__INCLUDED_)
