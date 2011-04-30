#ifndef _SG_CONTEXTOBJECT_H
#define _SG_CONTEXTOBJECT_H


#include <anim/Animatable.h>


namespace sg
{


class ContextObjectList;


/** 
 * Base class for graphics library context dependent objects. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ContextObject :
	public anim::Animatable
{
public:
	///
	ContextObject();

	///
	virtual ~ContextObject();

	/** Default implementation of Animatable. Does nothing. Derived animated ContextObjects can override this. */
	virtual void		blendState( anim::Animatable** anims, 
							const float* times, const float* weights, int n );

	/** Uploads object to the rendering device. */
	virtual void		load() = 0;

	/** Unloads object from the rendering device. */
	virtual void		unload() = 0;

	/** Releases resources of the object. Object cannot be used after this. */
	virtual void		destroy() = 0;

	/** Releases resources of the context dependent objects. */
	static void			destroyAll();

private:
	friend class ContextObjectList;
	ContextObject* m_next;

	ContextObject( const ContextObject& );
	ContextObject& operator=( const ContextObject& );
};


} // sg


#endif // _SG_CONTEXTOBJECT_H
