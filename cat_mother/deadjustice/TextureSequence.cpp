#include "TextureSequence.h"
#include <io/InputStream.h>
#include <io/InputStreamArchive.h>
#include <lang/Format.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace sg;
using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

TextureSequence::TextureSequence( InputStreamArchive* arch, const String& name,	
	const String& imageFilenameFmt, int firstImage, int lastImage ) :
	m_frames( Allocator<P(Texture)>(__FILE__) ),
	m_name(name)
{
	for ( int i = firstImage ; i <= lastImage ; ++i )
	{
		String fname = Format( imageFilenameFmt, i ).format();
		P(InputStream) ins = arch->getInputStream(fname);
		m_frames.add( new Texture( ins ) );
	}
}

TextureSequence::~TextureSequence() 
{
}
 
Texture* TextureSequence::getFrame( int i ) const 
{
	return m_frames[i];
}

int TextureSequence::frames() const 
{
	return m_frames.size();
}

const String& TextureSequence::name() const
{
	return m_name;
}
