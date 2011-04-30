#ifndef _PROGRESSDIALOG_H
#define _PROGRESSDIALOG_H


#include "ProgressInterface.h"
#include <lang/Object.h>


namespace lang {
	class String;}


/**
 * Progress dialog. Contains progress bar etc.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ProgressDialog :
	public lang::Object,
	public ProgressInterface
{
public:
	/** Creates and shows the window. */
	explicit ProgressDialog( HINSTANCE instance, HWND parent );

	///
	~ProgressDialog();

	/** Destroys the window. */
	void	destroy();

	/** Sets static text describing current task. */
	void	setText( const lang::String& text );

	/** Sets progress bar relative [0,1] position. */
	void	setProgress( float pos );

	/** Causes AbortExport to be thrown next time setText or setProgress is called. */
	void	abort();

private:
	HWND	m_hwnd;
	bool	m_abort;
	HWND	m_pbar;
	HWND	m_taskdesc;

	void	check();
	void	update();

	ProgressDialog( const ProgressDialog& );
	ProgressDialog& operator=( const ProgressDialog& );
};


#endif // _PROGRESSDIALOG_H
