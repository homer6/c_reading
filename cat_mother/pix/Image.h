#ifndef _PIX_IMAGE_H
#define _PIX_IMAGE_H


#include <lang/Object.h>


namespace io {
	class InputStream;
	class OutputStream;}

namespace lang {
	class String;}


namespace pix
{


class Surface;
class SurfaceFormat;


/**
 * Array of device independent bitmaps.
 * Use for texture with it's mipmaps or cube textures with mipmaps
 * If not using mipmaps, mipmap level count equals 1
 *
 * Supported image source file formats are JPG, TGA, BMP and DDS.
 * Both loading and saving is supported for the file formats.
 * For DDS no saving is implemented yet.
 * 
 * All coordinates used are expressed as pixels.
 * Origin is top left, x grows right and y grows down.
 *
 * The class tries to load/save image as close as possible
 * to original file format. You can still easily convert between
 * different formats, for example to load 8/16/24 bit test.bmp
 * to 32-bit ARGB format:
 *	<pre>
 *	Image img( SurfaceFormat::SURFACE_A8R8G8B8, Image("test.bmp") );
 *	</pre>
 *
 * @author Jani Kajala (jani.kajala@helsinki.fi), Toni Aittoniemi
 */
class Image :
	public lang::Object
{
public:
	/** Type of image. */
	enum ImageType 
	{
		/** Single face image, main surface is index 0, mip-maps follow. */
		TYPE_BITMAP,
		/** 6-face image, faces arranged as (+x,-x,+y,-y,+z,-z), mip-maps follow each image, respectively. */
		TYPE_CUBEMAP,
		/** Custom type, any count and arrangement not covered by BITMAP and CUBEMAP. */
		TYPE_CUSTOM
	};

	/* Creates an empty image. */
	Image();

	/** 
	 * Loads image from input stream. Takes name of the image from the stream.
	 * @exception IOException
	 */
	explicit Image( io::InputStream* in );

	/** 
	 * Loads image from input stream. 
	 * @exception IOException
	 */
	Image( io::InputStream* in, const lang::String& name );

	/** 
	 * Creates image of specified size and fits Other image to it. 
	 * Requires that the Other image has been initialized.
	 */
	Image( int width, int height, const Image* other );

	/** 
	 * Creates image of specified format and copies Other image to it. 
	 * Requires that the Other image has been initialized.
	 */
	Image( const SurfaceFormat& format, const Image* other );

	/** 
	 * Creates empty image of specified size and format. 
	 */
	Image( int width, int height, const SurfaceFormat& format );

	/** 
	 * Creates empty image of specified size and format. 
	 */
	Image( int width, int height, int subsurfaces, const SurfaceFormat& format );

	/** Copy by value. */
	explicit Image( const Image& other );

	///
	~Image();

	/** Copy by value. */
	Image&					operator=( const Image& other );

	/** 
	 * Loads image from input stream. Takes name of the image from the stream.
	 * @exception IOException
	 */
	void					load( io::InputStream* in );

	/** 
	 * Loads image from input stream. 
	 * @exception IOException
	 */
	void					load( io::InputStream* in, const lang::String& name );
	

	/** 
	 * Loads image from input stream to subsurface. Name of the image is acquired from the stream.
	 * @exception IOException
	 */
	void					load( int index, io::InputStream* in );

	/** 
	 * Loads image from input stream to subsurface. 
	 * @exception IOException
	 */
	void					load( int index, io::InputStream* in, const lang::String& name );

	/** 
	 * Saves the image to output stream. Takes name of the image from the stream.
	 * @exception IOException
	 */
	void					save( io::OutputStream* out );

	/** 
	 * Saves the image to output stream. 
	 * @exception IOException
	 */
	void					save( io::OutputStream* out, const lang::String& name );

	/** Returns array of surfaces. */
	Surface*				surfaceArray();

	/** Returns low-level surface object. */
	Surface&				surface( int index = 0 );
	const Surface&			surface( int index = 0 ) const;

	/** Returns count of surfaces. */
	int						surfaces() const;

	/** Returns mip-map count. */
	int						mipMapLevels() const;

	/** Returns image file name (or empty string if file not specified). */
	const lang::String&		filename() const;

	/** Returns width of the main surface in pixels. */
	int						width() const;

	/** Returns height of the main surface in pixels. */
	int						height() const;

	/** Returns number of bytes per scanline of the first image. */
	int						pitch() const;

	/** Returns data format. */
	const SurfaceFormat&	format() const;

	/** Returns image type. */
	ImageType		type() const;

private:
	class ImageImpl;
	P(ImageImpl) m_this;
};


} // pix


#endif // _PIX_IMAGE_H
