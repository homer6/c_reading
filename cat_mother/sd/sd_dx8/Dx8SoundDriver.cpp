#include "StdAfx.h"
#include "error.h"
#include "Dx8SoundDriver.h"
#include "Dx8SoundDevice.h"
#include "Dx8SoundBuffer.h"
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

Dx8SoundDriver::Dx8SoundDriver() :
	m_refs(0)
{
}

Dx8SoundDriver::~Dx8SoundDriver() 
{
	destroy();
}

void Dx8SoundDriver::addReference() 
{
	InterlockedIncrement( &m_refs );
}

void Dx8SoundDriver::release() 
{
	if ( 0 == InterlockedDecrement( &m_refs ) )
		delete this;
}

void Dx8SoundDriver::destroy() 
{
	int objs = DrvObject::objects();
	assert( objs == 0 );
	if ( objs > 0 )
		error( "Driver dependent objects still exist (x%i) at device driver deinit", objs );

	DrvObject::deleteAll();
}

sd::SoundBuffer* Dx8SoundDriver::createSoundBuffer() 
{
	return new Dx8SoundBuffer;
}

sd::SoundDevice* Dx8SoundDriver::createSoundDevice() 
{
	return new Dx8SoundDevice;
}
