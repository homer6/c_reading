#include "ContextUtil.h"
#include <sg/Context.h>
#include <gd/GraphicsDevice.h>
#include <io/File.h>
#include <io/FileOutputStream.h>
#include <pix/Image.h>
#include <pix/Surface.h>
#include <pix/SurfaceFormat.h>
#include <lang/String.h>
#include <lang/Exception.h>
#include <stdio.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace sg;
using namespace pix;
using namespace lang;

//-----------------------------------------------------------------------------

namespace sgu
{


void ContextUtil::grabScreen( const String& ext )
{
	// create screen compatible image
	gd::GraphicsDevice* dev = Context::device();
	int w = dev->width();
	int h = dev->height();
	SurfaceFormat fmt = SurfaceFormat::SURFACE_UNKNOWN;
	dev->getFormat( &fmt );
	P(Image) img = new Image( w, h, fmt );

	// grab back buffer
	void* ptr = 0;
	int pitch;
	if ( !dev->lockBackBuffer( &ptr, &pitch ) )
		return;
	img->surface(0).blt( 0, 0, w, h, ptr, w, h, pitch, fmt );
	dev->unlockBackBuffer();

	// find free file name
	int count = 0;
	String fname;
	do
	{
		fname = Format( "screenshot{0,00}.{1}", ++count, ext ).format();
	} while ( File(fname).exists() );

	// save image
	P(FileOutputStream) out = new FileOutputStream( fname );
	img->save( out );
	out->close();
}

Context::TextureFilterType ContextUtil::toTextureFilterType( const lang::String& str )
{
	Context::TextureFilterType filter = Context::TEXF_NONE;

	String s = str.toUpperCase();
	if ( s == "POINT" )
		filter = Context::TEXF_POINT;
	else if ( s == "LINEAR" )
		filter = Context::TEXF_LINEAR;
	else if ( s == "ANISOTROPIC" )
		filter = Context::TEXF_ANISOTROPIC;
	else
		throw Exception( Format("Unknown texture filter type: {0}", str) );

	return filter;
}


} // sgu
