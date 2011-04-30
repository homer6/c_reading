#ifndef _LANG_SYSTEM_H
#define _LANG_SYSTEM_H


namespace lang
{


/** 
 * System class provides running environment related functionality:
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class System
{
public:
	/** Returns the current time in milliseconds. */
	static long		currentTimeMillis();

private:
	System();
	System( const System& );
	System& operator=( const System& );
};


} // lang


#endif // _LANG_SYSTEM_H
