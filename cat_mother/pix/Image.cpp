#include "Image.h"
#include "loadJPEG.h"
#include "loadBMP.h"
#include "loadTGA.h"
#include "loadDDS.h"
#include "Surface.h"
#include "SurfaceUtil.h"
#include "SurfaceFormat.h"
#include <io/FileInputStream.h>
#include <io/FileOutputStream.h>
#include <io/IOException.h>
#include <lang/Array.h>
#include <lang/Object.h>
#include <lang/String.h>
#include <assert.h>
#include <stdint.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;

//-----------------------------------------------------------------------------

namespace pix
{


class Image::ImageImpl :
	public Object
{
public:
	explicit ImageImpl( const lang::String& filename );

	ImageImpl( InputStream* in, const lang::String& filename );

	ImageImpl( int width, int height, const SurfaceFormat& format );
	
	ImageImpl( int width, int height, int subsurfaces, const SurfaceFormat& format );

	ImageImpl( int width, int height, int subsurfaces, int mipmaplevels, const SurfaceFormat& format );

	ImageImpl( int width, int height, const ImageImpl* other );

	ImageImpl( const SurfaceFormat& format, const ImageImpl* other );

	ImageImpl();

	void		load( InputStream* in, const lang::String& name );

	void		save( OutputStream* out, const lang::String& name );

	void		save( const lang::String& filename );

	void		swap( ImageImpl& other );

	Surface&	surface( int index )												{return m_surfaces[index];}

	const Surface&	surface( int index ) const										{return m_surfaces[index];}

	int			surfaces() const													{return m_surfaces.size();}

	Surface*	surfaceArray()														{return m_surfaces.begin();}

	int			mipMapLevels()														{return m_mipMapLevels;}
		
	const lang::String&		filename() const										{return m_filename;}

	ImageType		type() const													{return m_type;}

	const SurfaceFormat&	format() const											{return m_surfaces[0].format();}

private:
	lang::String			m_filename;
	Array<Surface>			m_surfaces;
	int						m_mipMapLevels;
	ImageType				m_type;

	void	allocate( int width, int height, int subsurface, const SurfaceFormat& format );
};

//-----------------------------------------------------------------------------

Image::ImageImpl::ImageImpl( InputStream* in, const lang::String& name )
{
	load( in, name );
}

Image::ImageImpl::ImageImpl( int width, int height, const SurfaceFormat& format )
{
	allocate( width, height, 0, format );
}

Image::ImageImpl::ImageImpl( int width, int height, int subsurfaces, const SurfaceFormat& format )
{
	m_surfaces.setSize( subsurfaces );
	for ( int i = 0; i < m_surfaces.size(); ++i )
		allocate( width, height, i, format );
}

Image::ImageImpl::ImageImpl( int width, int height, const ImageImpl* other )
{
	m_surfaces.setSize( other->m_surfaces.size() );
	m_mipMapLevels = other->m_mipMapLevels;
	for ( int i = 0; i < m_surfaces.size(); ++i )
	{
		allocate( width, height, i, other->format() );
		m_surfaces[i].blt( &other->m_surfaces[i] );
	}
	m_filename = other->m_filename;
}

Image::ImageImpl::ImageImpl( const SurfaceFormat& format, const ImageImpl* other )
{
	m_surfaces.setSize( other->m_surfaces.size() );
	m_mipMapLevels = other->m_mipMapLevels;
	for ( int i = 0; i < m_surfaces.size(); ++i )
	{
		allocate( other->m_surfaces[i].width(), other->m_surfaces[i].height(), i, format );
		m_surfaces[i].blt( &other->m_surfaces[i] );
	}
	m_filename = other->m_filename;
}

Image::ImageImpl::ImageImpl()
{
	m_filename = "";
	m_surfaces.setSize(0);
	m_mipMapLevels = 1;
	m_type = TYPE_CUSTOM;
}

Image::~Image()
{
}

void Image::ImageImpl::allocate( int width, int height, int subsurface, const SurfaceFormat& format )
{
	assert( width > 0 );
	assert( height > 0 );
	assert( format.pixelSize() > 0 );
	
	m_filename	= "";
	if ( (subsurface + 1) > m_surfaces.size() ) 
		m_surfaces.setSize( subsurface + 1 );
	m_surfaces[subsurface].create( width, height, format );
}


void Image::ImageImpl::load( InputStream* in, const String& name )
{
	uint8_t* bits = 0;

	try
	{
		// check minimum file name length (?.ext)
		m_filename = name;
		const int filenameLen = name.length();
		if ( filenameLen < 5 ) 
			throw IOException( Format("Failed to load image file: {0}",name) );

		// find out extension (in lowercase)
		lang::String ext = name.substring( name.length()-4, name.length() ).toLowerCase();

		// load image data
		int width = 0;
		int height = 0;
		int pitch = 0;
		int mipmaplevels = 1;
		SurfaceFormat format = SurfaceFormat::SURFACE_UNKNOWN;
		ImageType type = TYPE_BITMAP;
		bool ok = false;
		if ( lang::String(".bmp") == ext )
			bits = loadBMP( in, &width, &height, &format );
		else if ( lang::String(".tga") == ext )
			bits = loadTGA( in, &width, &height, &format );
		else if ( lang::String(".jpg") == ext )
			bits = loadJPEG( in, &width, &height, &format );
		else if ( lang::String(".dds") == ext )
			ok = loadDDS( in, &width, &height, &pitch, &mipmaplevels, &format, &type, m_surfaces );

		// image data loading failed?
		if ( !bits && !ok )
			throw IOException( Format("Image file {0} format unsupported",name) );

		// Blt only linear data surfaces with uncompressed data, don't blit DDS data again
		if ( format.bltSupported() && lang::String(".dds") != ext )
		{
			// create actual surface and copy image data to it
			allocate( width, height, 0, format );
			m_surfaces[0].blt( 0, 0, width, height, bits, width, height, width*format.pixelSize(), format );
		}

		// image info
		m_mipMapLevels = mipmaplevels;
		m_filename = name;
		m_type = type;

		// release resources
		if ( bits ) {delete[] bits; bits = 0;}
	}
	catch ( ... )
	{
		// release resources
		if ( bits ) {delete[] bits; bits = 0;}
		throw;
	}
}

void Image::ImageImpl::save( OutputStream* out, const String& name )
{
//	assert( data() );

	// check minimum file name length (?.ext)
	const int filenameLen = name.length();
	if ( filenameLen < 5 ) 
		throw IOException( Format("Failed to save image file: {0}",name) );

	// find out extension (in lowercase)
	lang::String ext = name.substring( name.length()-4, name.length() ).toLowerCase();

	// save image data
	bool ok = false;
	if ( lang::String(".bmp") == ext )
		ok = saveBMP( out, m_surfaces[0].data(), m_surfaces[0].width(), m_surfaces[0].height(), m_surfaces[0].format() );
	else if ( lang::String(".tga") == ext )
		ok = saveTGA( out, m_surfaces[0].data(), m_surfaces[0].width(), m_surfaces[0].height(), m_surfaces[0].format() );
	else if ( lang::String(".jpg") == ext )
		ok = saveJPEG( out, m_surfaces[0].data(), m_surfaces[0].width(), m_surfaces[0].height(), m_surfaces[0].format() );

	// image data saving failed?
	if ( !ok )
		throw IOException( Format("Failed to save image file: {0}",name) );

	// store name
	m_filename = name;
}

//-----------------------------------------------------------------------------

Image::Image( io::InputStream* in )
{
	m_this = new ImageImpl( in, in->toString() );
}

Image::Image( InputStream* in, const String& name )
{
	m_this = new ImageImpl( in, name );
}

Image::Image( int width, int height, const Image* other )
{
	m_this = new ImageImpl( width, height, other->m_this );
}

Image::Image( const SurfaceFormat& format, const Image* other )
{
	m_this = new ImageImpl( format, other->m_this );
}

Image::Image( int width, int height, const SurfaceFormat& format )
{
	m_this = new ImageImpl( width, height, format );
}

Image::Image( int width, int height, int subsurfaces, const SurfaceFormat& format )
{
	m_this = new ImageImpl( width, height, subsurfaces, format );
}

Image::Image( const Image& other )
{
	m_this = new ImageImpl( *other.m_this );
}

Image::Image()
{
	m_this = new ImageImpl;
}

Image& Image::operator=( const Image& other )
{
	*m_this = *other.m_this;
	return *this;
}

void Image::save( OutputStream* out, const lang::String& name )
{
	m_this->save( out, name );
}

const lang::String& Image::filename() const
{
	return m_this->filename();
}

void Image::save( io::OutputStream* out )
{
	m_this->save( out, out->toString() );
}

void Image::load( InputStream* in )
{
	m_this->load( in, in->toString() );
}

void Image::load( InputStream* in, const String& name )
{
	m_this->load( in, name );
}

Surface& Image::surface( int index )
{
	return m_this->surface( index );
}

const Surface& Image::surface( int index ) const
{
	return m_this->surface( index );
}

int Image::surfaces() const
{
	return m_this->surfaces();
}

pix::Surface* Image::surfaceArray()
{
	return m_this->surfaceArray();
}

int Image::mipMapLevels() const
{
	return m_this->mipMapLevels();
}

Image::ImageType Image::type() const 
{
	return m_this->type();
}

int Image::width() const 
{
	return m_this->surface(0).width();
}

int Image::height() const 
{
	return m_this->surface(0).height();
}

int Image::pitch() const 
{
	return m_this->surface(0).pitch();
}

const SurfaceFormat&	Image::format() const 
{
	return m_this->format();
}



} // pix
