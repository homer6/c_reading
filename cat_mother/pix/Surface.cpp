#include "Surface.h"
#include "SurfaceUtil.h"
#include <mem/raw.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace pix
{


Surface::Surface()
{
	defaults();
}

Surface::Surface( const Surface& other )
{
	defaults();
	*this = other;
}

Surface::Surface( int width, int height, const SurfaceFormat& format )
{
	defaults();
	create( width, height, format );
}

Surface::Surface( int datasize, const SurfaceFormat& format )
{
	defaults();
	create( datasize, format );
}

Surface::Surface( int width, int height, int pitch, int datasize, const SurfaceFormat& format )
{
	defaults();
	create( datasize, format );

	m_w		= width;
	m_h		= height;
	m_pitch = pitch;	
}

Surface::~Surface()
{
	destroy();
}

void Surface::create( int width, int height, const SurfaceFormat& format )
{
	m_pitch = width * format.pixelSize();

	int bytes = height * m_pitch;
	char* data	= (char*)mem_alloc( bytes );
	memset( data, 0xFF, bytes );

	destroy();
	m_data	= data;
	m_w		= width;
	m_h		= height;
	m_fmt	= format;
	m_dataSize = height * m_pitch;
}

void Surface::create( int datasize, const SurfaceFormat& format )
{
	char* data = (char*)mem_alloc( datasize );
	memset( data, 0xFF, datasize );
	
	destroy();
	m_data	= data;
	m_w		= 0;
	m_h		= 0;
	m_pitch	= 0;
	m_fmt	= format;
	m_dataSize = datasize;
}

void Surface::swap( Surface& other )
{
	int				width	= other.m_w;
	int				height	= other.m_h;
	int				pitch	= other.m_pitch;
	SurfaceFormat	format	= other.m_fmt;
	char*			data	= other.m_data;
	int			dataSize	= other.m_dataSize;

	other.m_data	= m_data;
	other.m_w		= m_w;
	other.m_h		= m_h;
	other.m_pitch	= m_pitch;
	other.m_fmt		= m_fmt;
	other.m_dataSize = m_dataSize;

	m_data	= data;
	m_w		= width;
	m_h		= height;
	m_pitch	= pitch;
	m_fmt	= format;
	m_dataSize = dataSize;
}

void Surface::defaults()
{
	m_data		= 0;
	m_w			= 0;
	m_h			= 0;
	m_pitch		= 0;
	m_dataSize	= 0;
	m_fmt		= SurfaceFormat::SURFACE_UNKNOWN;
}

void Surface::destroy()
{
	if ( m_data )
	{
		mem_free( m_data );
		defaults();
	}
}

Surface& Surface::operator=( const Surface& other )
{
	int				width	= other.m_w;
	int				height	= other.m_h;
	int				pitch	= other.m_pitch;
	SurfaceFormat	format	= other.m_fmt;
	int			  datasize  = other.m_dataSize;

	char*			data	= 0;

	if ( datasize > 0 )
	{
		data = (char*)mem_alloc( datasize );
		memcpy( data, other.m_data, datasize );
	}

	destroy();
	m_data		= data;
	m_w			= width;
	m_h			= height;
	m_pitch		= pitch;
	m_fmt		= format;
	m_dataSize	= datasize;
	return *this;
}

void Surface::blt( const Surface* src )
{
	blt( 0, 0, width(), height(), src, 0, 0, src->width(), src->height() );
}

void Surface::blt( int x, int y, int w, int h, const Surface* src, int srcX, int srcY, int srcW, int srcH )
{
	assert( src != this );
	assert( src->format().bltSupported() );
	assert( format().bltSupported() );
	assert( isInside(x,y,w,h) );
	assert( src->isInside(srcX,srcY,srcW,srcH) );

	SurfaceUtil::blt( format(), x, y, w, h, data(), pitch(), src->format(), srcX, srcY, srcW, srcH, src->data(), src->pitch() );
}

void Surface::blt( int x, int y, int w, int h, 
	const void* src, int srcW, int srcH, int srcPitch, const SurfaceFormat& srcFormat )
{
	assert( format().bltSupported() );
	assert( srcFormat.bltSupported() );
	assert( isInside(x,y,w,h) );
	assert( srcPitch >= srcFormat.pixelSize()*srcW );

	blt( data(), x, y, w, h, pitch(), format(), src, srcW, srcH, srcPitch, srcFormat );
}

void Surface::blt( void* dst, int x, int y, int w, int h, 
	int dstPitch, const SurfaceFormat& dstFormat,
	const void* src, int srcW, int srcH, 
	int srcPitch, const SurfaceFormat& srcFormat )
{
	assert( srcFormat.bltSupported() );
	assert( dstFormat.bltSupported() );

	if ( dstFormat.bltSupported() )
		SurfaceUtil::blt( dstFormat, x, y, w, h, dst, dstPitch, srcFormat, 0, 0, srcW, srcH, src, srcPitch );
}

void Surface::copyData( const void* sourceData, int size ) 
{
	int				bytes	= size;
	char*			data	= 0;

	if ( bytes > 0 )
	{
		data = (char*)mem_alloc( bytes );
		memcpy( data, sourceData, bytes );
	}

	destroy();

	m_data	= data;
	m_dataSize = size;
}

void Surface::setPixel( int x, int y, long argb )
{
	assert( x >= 0 && x < width() );
	assert( y >= 0 && y < height() );
	assert( format().bltSupported() );

	uint8_t buff[4] = { uint8_t(argb), uint8_t(argb>>8), uint8_t(argb>>16), uint8_t(argb>>24) };

	uint8_t* pixeldata = reinterpret_cast<uint8_t*>( data() );
	long i = x * format().pixelSize() + y * (long)pitch();
	format().copyPixels( pixeldata+i, SurfaceFormat::SURFACE_A8R8G8B8, buff, 1 );
}

long Surface::getPixel( int x, int y ) const
{
	return SurfaceUtil::getPixel( x, y, m_w, m_h, m_data, m_pitch, m_fmt );
}

bool Surface::isInside( int x, int y, int w, int h ) const
{
	assert( w > 0 );
	assert( h > 0 );

	return	
		x >= 0 && x < width() && 
		y >= 0 && y < height() &&
		x+w <= width() && 
		y+h <= height();
}

int Surface::pitch() const
{
	assert( m_dataSize == m_h * m_pitch || 
		m_fmt == SurfaceFormat::SURFACE_DXT1 ||
		m_fmt == SurfaceFormat::SURFACE_DXT3 ||
		m_fmt == SurfaceFormat::SURFACE_DXT5 );
	return m_pitch;
}


} // pix
