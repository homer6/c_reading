#ifndef _THEAD_IMPL_H
#define _THEAD_IMPL_H


namespace lang
{


/** 
 * Thread handle. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
typedef struct _Thread_t {}	*Thread_t;


/** 
 * Creates a new thread. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
int		Thread_create( Thread_t* t, void* (*start)(void*), void* arg );

/** 
 * Marks thread resources for release. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
int		Thread_detach( Thread_t t );

/** 
 * Waits for thread to die and releases thread resources. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
int		Thread_join( Thread_t t );

/** 
 * Delays current thread for specified number of milliseconds. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
int		Thread_sleep( int millis );


} // lang


#endif // _THEAD_IMPL_H
