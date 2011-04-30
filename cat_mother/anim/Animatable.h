#ifndef _ANIM_ANIMATABLE_H
#define _ANIM_ANIMATABLE_H


#include <lang/Object.h>


namespace lang {
	class String;}


namespace anim
{


/** 
 * Abstract base for objects that can be animated. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Animatable :
	public lang::Object
{
public:
	/** 
	 * Sets state of the object controllers to specified time. 
	 * Default implementation uses blendState().
	 */
	virtual void	setState( float time );

	/** 
	 * Sets state of the object by blending n other object controller states. 
	 * Derived classes should override at least this if they use controllers.
	 * @param anims Array of pointers to source animatable objects.
	 * @param times Animation state times for source objects.
	 * @param weights Weights of the source animations. Sum should equal to 1.
	 * @param n Number of source animatable objects.
	 */
	virtual void	blendState( Animatable** anims, 
						const float* times, const float* weights, int n ) = 0;
};


} // anim


#endif // _ANIM_ANIMATABLE_H

