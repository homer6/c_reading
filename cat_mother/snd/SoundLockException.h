#ifndef _SND_SOUNDLOCKEXCEPTION_H
#define _SND_SOUNDLOCKEXCEPTION_H


#include <lang/Exception.h>


namespace snd
{


/** 
 * Throw if sound device locking failed. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SoundLockException :
	public lang::Exception
{
public:
	SoundLockException( const lang::String& typeName )	: lang::Exception( lang::Format("Failed to lock {0}", typeName) ) {}
};


} // snd


#endif // _SND_SOUNDLOCKEXCEPTION_H
