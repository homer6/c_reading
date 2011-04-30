#ifndef _PIX_SURFACEUTIL_H
#define _PIX_SURFACEUTIL_H


namespace pix
{


class SurfaceFormat;


/** 
 * Utilities for surface pixel data management. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SurfaceUtil
{
public:
	/** 
	 * Returns 32-bit ARGB pixel at specified coordinate. 
	 * Supports also DXT-compressed image formats.
	 */
	static long		getPixel( int x, int y, 
						int width, int height, 
						const void* data, int pitch, 
						const SurfaceFormat& format );

	/** 
	 * Copies block of pixels from one surface format to another. 
	 * Uses Floyd-Steinberg dithering.
	 */
	static void		blt( const SurfaceFormat& dstFormat, int dstWidth, int dstHeight, void* dstData, int dstPitch, 
						const SurfaceFormat& srcFormat, const void* srcData, int srcPitch );

	/** 
	 * Copies block of pixels from one surface format to another. 
	 * Uses Floyd-Steinberg dithering. Stretches/shrinks source image if needed.
	 */
	static void		blt( const SurfaceFormat& dstFormat, 
						int dstX, int dstY, int dstWidth, int dstHeight, 
						void* dstData, int dstPitch, 
						const SurfaceFormat& srcFormat, 
						int srcX, int srcY, int srcWidth, int srcHeight, 
						const void* srcData, int srcPitch );
};


} // pix


#endif // _PIX_SURFACEUTIL_H
