#ifndef _PIX_SURFACE_H
#define _PIX_SURFACE_H


#include <pix/SurfaceFormat.h>


namespace pix
{


/** 
 * Simple pixel array. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Surface
{
public:
	/** Creates an empty 0x0 surface of undefined format. */
	Surface();

	/** Copy by value. */
	Surface( const Surface& other );

	/** Creates an array of specified size (pixels) and format. */
	Surface( int width, int height, const SurfaceFormat& format );

	/** Creates an array of specified size (bytes) and format. */
	Surface( int datasize, const SurfaceFormat& format );

	/** Creates an array of specified size (pixels & bytes) and format. */
	Surface( int width, int height, int pitch, int datasize, const SurfaceFormat& format );

	///
	~Surface();

	/** Copy by value. */
	Surface&				operator=( const Surface& other );

	/** Stretches a copy of Other image to this image. */
	void			blt( const Surface* other );

	/** 
	 * Stretches a copy of a rectangle from Other image to this image. 
	 * Both rectangles must be inside image bounds.
	 */
	void			blt( int x, int y, int w, int h, const Surface* other, 
						int otherX, int otherY, int otherW, int otherH );

	/** 
	 * Stretches a copy of source pixel data to this image. 
	 * Both rectangles must be inside image bounds.
	 */
	void			blt( int x, int y, int width, int height, 
						const void* sourceData, int sourceWidth, int sourceHeight, 
						int sourcePitch, const SurfaceFormat& sourceFormat );

	/**
	 * Copies bits from source to this image.
	 */
	void			copyData( const void* sourceData, int size );

	/** Creates width x height surface with specified pixel format. */
	void			create( int width, int height, const SurfaceFormat& format );

	/** Creates sized surface with specified pixel format. */
	void			create( int datasize, const SurfaceFormat& format );

	/** Swaps contents of two surface objects. */
	void			swap( Surface& other );

	/** Sets a 32-bit ARGB format pixel at specified position. */
	void			setPixel( int x, int y, long argb );
	
	/** 
	 * Returns a 32-bit ARGB format pixel at specified position. 
	 * Supports also DXT-compressed image formats.
	 */
	long			getPixel( int x, int y ) const;
	
	/** Returns pointer to pixel data. */
	void*					data()													{return m_data;}

	/** Returns width of the array in pixels. */
	int						width() const											{return m_w;}

	/** Returns height of the array in pixels. */
	int						height() const											{return m_h;}

	/** Returns distance to next scaline in bytes. */
	int						pitch() const;

	/** Returns data format. */
	const SurfaceFormat&	format() const											{return m_fmt;}

	/** Returns pointer to pixel data. */
	const void*				data() const											{return m_data;}

	/** Returns data size. */
	int						dataSize() const										{return m_dataSize;}

	/** Returns true if the rectangle is inside current surface dimensions. */
	bool					isInside( int x, int y, int w, int h ) const;

	// Static operations
	/** Copies pixels from source to destination format. */
	static void		blt( void* dst, int x, int y, int w, int h,
						int dstPitch, const SurfaceFormat& dstFormat,
						const void* src, int srcW, int srcH,
						int srcPitch, const SurfaceFormat& srcFormat );

private:
	char*			m_data;
	int				m_w;
	int				m_h;
	int				m_pitch;
	int				m_dataSize;
	SurfaceFormat	m_fmt;

	void			defaults();
	void			destroy();
};


} // pix


#endif // _PIX_SURFACE_H
