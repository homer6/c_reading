// sgviewerView.h : interface of the CSgviewerView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SGVIEWERVIEW_H__D53EDAE7_A166_11D5_BC88_0000B49545E8__INCLUDED_)
#define AFX_SGVIEWERVIEW_H__D53EDAE7_A166_11D5_BC88_0000B49545E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <sg/Node.h>
#include <sg/LineList.h>
#include <sg/Context.h>
#include <util/Vector.h>


namespace sg {
	class Shader;
	class Primitive;
	class Mesh;
	class Camera;
	class Node;}


class CSgviewerView : 
	public CView
{
public:
	void	pauseTime();

private:
	float			m_time;
	long			m_prevTime;
	float			m_flySpeed;		// fly camera movement km/h
	float			m_flySpeedRot;	// fly camera rotation degrees/s
	bool			m_statistics;
	P(sg::Node)		m_dragNode;
	float			m_dragStartX;
	float			m_dragStartY;
	float			m_dragDist;
	bool			m_dragRotate;
	float			m_distance;
	P(sg::Node)		m_distanceNode;
	bool			m_distanceDirty;
	bool			m_paused;
	float			m_dt;
	bool			m_bvolVis;
	P(sg::LineList)	m_dbgLines;
	P(sg::Mesh)		m_dbgMesh;
	bool			m_grabScreen;
	util::Vector<math::Matrix4x4>			m_boneMatrices;
	util::Vector<const math::Matrix4x4*>	m_boneMatrixPtrs;

	void	startDrag( UINT flags, CPoint point );
	void	stopDrag();

protected: // create from serialization only
	CSgviewerView();
	DECLARE_DYNCREATE(CSgviewerView)

// Attributes
public:
	CSgviewerDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSgviewerView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSgviewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	void OnDrawImpl(CDC* pDC);

	void	draw();
	void	setShaderParams( sg::Primitive* prim, sg::Shader* fx, sg::Mesh* mesh, sg::Camera* camera, sg::Node* root );

	static void		printNodeTree( sg::Node* root, int margin=0 );

// Generated message map functions
protected:
	//{{AFX_MSG(CSgviewerView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown( UINT ch, UINT rep, UINT flags );
	afx_msg void OnSysCommand( UINT nID, LPARAM lParam );
	afx_msg void OnLButtonDown( UINT flags, CPoint point );
	afx_msg void OnLButtonUp( UINT flags, CPoint point );
	afx_msg void OnRButtonDown( UINT flags, CPoint point );
	afx_msg void OnRButtonUp( UINT flags, CPoint point );
	afx_msg void OnMouseMove( UINT flags, CPoint point );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in sgviewerView.cpp
inline CSgviewerDoc* CSgviewerView::GetDocument()
   { return (CSgviewerDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SGVIEWERVIEW_H__D53EDAE7_A166_11D5_BC88_0000B49545E8__INCLUDED_)
