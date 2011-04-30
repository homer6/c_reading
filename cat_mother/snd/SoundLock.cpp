#include "SoundLock.h"
#include <sd/SoundBuffer.h>
#include <snd/Sound.h>
#include <snd/SoundLockException.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace snd
{


SoundLock::SoundLock( Sound* obj, int offset, int bytes, void** data1, int* bytes1, void** data2, int* bytes2 ) :
	m_obj(obj)
{
	assert( obj->m_sb );
	int err = obj->m_sb->lock( offset, bytes, &m_data1, &m_bytes1, &m_data2, &m_bytes2, 0 );
	if ( err ) 
		throw SoundLockException( obj->name() );

	if ( data1 )
		*data1 = m_data1;
	if ( data2 )
		*data2 = m_data2;
	if ( bytes1 )
		*bytes1 = m_bytes1;
	if ( bytes2 )
		*bytes2 = m_bytes2;
}

SoundLock::~SoundLock()
{
	m_obj->m_sb->unlock( m_data1, m_bytes1, m_data2, m_bytes2 );
}


} // snd
