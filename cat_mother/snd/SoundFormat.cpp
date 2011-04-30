#include "SoundFormat.h"
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace snd
{


SoundFormat::SoundFormat() :
	m_samplesPerSec( 0 ),
	m_bitsPerSample( 0 ),
	m_channels( 0 )
{
}

SoundFormat::SoundFormat( int samplesPerSec, int bitsPerSample, int channels ) :
	m_samplesPerSec( (unsigned short)samplesPerSec ),
	m_bitsPerSample( (unsigned char)bitsPerSample ),
	m_channels( (unsigned char)channels )
{
	assert( samplesPerSec == (int)m_samplesPerSec );
	assert( bitsPerSample == (int)m_bitsPerSample );
	assert( channels == (int)m_channels );
}


} // snd
