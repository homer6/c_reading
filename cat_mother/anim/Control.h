#ifndef _ANIM_CONTROL_H
#define _ANIM_CONTROL_H


#include <lang/Object.h>


namespace anim
{


/** 
 * Abstract base for controllers. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Control :
	public lang::Object
{
public:
	///
	Control();

	///
	virtual ~Control();

	/** 
	 * Gets value of the controller at specified time. 
	 * @param time Time of value to retrieve.
	 * @param value [out] Receives controller value.
	 * @param size Number of floats in the controller value.
	 * @param hint Last returned hint key to speed up operation. (optional)
	 * @return New hint key.
	 */
	virtual int	getValue( float time, float* value, int size, int hint=0 ) const = 0;

	/** Returns number of floats in the controller value. */
	virtual int	channels() const = 0;

private:
	Control( const Control& );
	Control& operator=( const Control& );
};


} // anim


#endif // _ANIM_CONTROL_H
