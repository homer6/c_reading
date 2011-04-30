#ifndef _BSP_PROGRESSINDICATOR_H
#define _BSP_PROGRESSINDICATOR_H


namespace bsp
{


/** 
 * Interface to BSP tree building progress indicator. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ProgressIndicator
{
public:
	/** Adds specified amount of work to progress indicator. */
	virtual void addProgress( double work ) = 0;
};


} // bsp


#endif // _BSP_PROGRESSINDICATOR_H
