// sgviewerDoc.h : interface of the CSgviewerDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SGVIEWERDOC_H__D53EDAE5_A166_11D5_BC88_0000B49545E8__INCLUDED_)
#define AFX_SGVIEWERDOC_H__D53EDAE5_A166_11D5_BC88_0000B49545E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <sg/Mesh.h>
#include <sg/Camera.h>
#include <util/Vector.h>


class CSgviewerDoc : public CDocument
{
public:
	String			name;
	P(sg::Node)		scene;
	P(sg::Node)		defaultLight;

	P(sg::Mesh)		cube1;
	P(sg::Mesh)		cube2;
	math::Vector3	cube1dim;
	math::Vector3	cube2dim;

	void			selectFlyCamera();
	void			selectNextCamera();
	sg::Camera*		activeCamera() const;
	sg::Camera*		flyCamera() const;

	sg::Node*		pickNode( float x, float y, float* distance=0 );
	void			dragNode( float dx, float dy, float dz, sg::Node* node, float dist );
	void			dragNodeRotate( float dx, float dy, sg::Node* node );

private:
	int								camera;
	util::Vector< P(sg::Camera) >	cameras;

	/** Call this after setting up scene. */
	void prepareDoc();

protected: // create from serialization only
	CSgviewerDoc();
	DECLARE_DYNCREATE(CSgviewerDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSgviewerDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSgviewerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CSgviewerDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SGVIEWERDOC_H__D53EDAE5_A166_11D5_BC88_0000B49545E8__INCLUDED_)
