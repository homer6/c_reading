#ifndef _PROGRESSINTERFACE_H
#define _PROGRESSINTERFACE_H


namespace lang {
	class String;}


/** 
 * Interface to export progress indicator. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ProgressInterface
{
public:
	/** Sets static text describing current task. */
	virtual void	setText( const lang::String& text ) = 0;

	/** Sets progress bar relative [0,1] position. */
	virtual void	setProgress( float pos ) = 0;
};


#endif // _PROGRESSINTERFACE_H
