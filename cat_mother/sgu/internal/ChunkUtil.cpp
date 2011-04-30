#include "ChunkUtil.h"
#include <lang/String.h>
#include <anim/Interpolator.h>
#include <io/ChunkInputStream.h>
#include <io/IOException.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;
using namespace anim;

//-----------------------------------------------------------------------------

namespace sgu
{


void ChunkUtil::readAnim( ChunkInputStream* in, Interpolator* anim, const String& name, int loadFlags )
{
	// read animation settings
	int channels = in->readInt();
	int keys = in->readInt();
	in->readInt();	// interp. type
	if ( channels != anim->channels() )
		throw IOException( Format("Invalid number of channels ({0}, expected {1}) in {2} animation", channels, anim->channels(), name) );

	// read max 1 key if animations are not needed
	if ( 0 != (loadFlags & NO_ANIMATIONS) && keys > 1 )
		keys = 1;

	// read key frames
	anim->setKeys( keys );
	for ( int i = 0 ; i < keys ; ++i )
	{
		float time = in->readFloat();
		float* value = anim->getTempBuffer( channels );
		
		for ( int k = 0 ; k < channels ; ++k )
			value[k] = in->readFloat();

		anim->setKeyTime( i, time );
		anim->setKeyValue( i, value, channels );
	}
}


} // sgu
