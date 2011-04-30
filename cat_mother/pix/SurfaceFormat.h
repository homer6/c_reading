#ifndef _PIX_SURFACEFORMAT_H
#define _PIX_SURFACEFORMAT_H


namespace pix
{


/** 
 * Describes how surface bits are arranged to pixels.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SurfaceFormat
{
public:
	/** 
	 * Type of surface pixel format.
	 */
	enum SurfaceFormatType
	{
		/// The surface format is unknown.
		SURFACE_UNKNOWN,

		/// 24-bit RGB pixel format.
		SURFACE_R8G8B8,
		/// 32-bit RGB pixel format with alpha. 
		SURFACE_A8R8G8B8,
		/// 32-bit RGB pixel format where 8 bits are reserved for each color. 
		SURFACE_X8R8G8B8,
		/// 16-bit RGB pixel format. 
		SURFACE_R5G6B5,
		/// 16-bit RGB pixel format. 
		SURFACE_R5G5B5,
		/// 4-bit palettized pixel format. (unsupported by copyPixels)
		SURFACE_P4,
		/// 8-bit palettized pixel format. (unsupported by copyPixels)
		SURFACE_P8,
		/// 16-bit pixel format where 5 bits are reserved for color and 1 bit is reserved for transparency. 
		SURFACE_A1R5G5B5,
		/// 16-bit RGB pixel format where 4 bits are reserved for each color. 
		SURFACE_X4R4G4B4,
		/// 16-bit RGB pixel format. 
		SURFACE_A4R4G4B4,
		/// 8-bit RGB texture format. 
		SURFACE_R3G3B2,
		/// 8-bit RGB texture format. 
		SURFACE_R3G2B3,
		/// 8-bit alpha-only.
		SURFACE_A8,
		/// 16-bit RGB pixel format with alpha.
		SURFACE_A8R3G3B2,
		/// 16-bit RGB pixel format with alpha.
		SURFACE_A8R3G2B3,

		/// DirectX Compressed Texture ( opaque or 1-bit alpha ), blit not supported
		SURFACE_DXT1,
		/// DirectX Compressed Texture ( explicit alpha ), blit not supported
		SURFACE_DXT3,
		/// DirectX Compressed Texture ( interpolated alpha ), blit not supported
		SURFACE_DXT5,

		/// 32-bit depth buffer format
		SURFACE_D32,
		/// 16-bit depth buffer format
		SURFACE_D16,
		/// 32-bit depth buffer format, depth using 24 bits and stencil 8 bits
		SURFACE_D24S8,

		/// Number of pixel formats.
		SURFACE_LAST,
	};

	/** Creates unspecified surface format. */
	SurfaceFormat();

	/** Creates surface format from enumerated pixel format type. */
	SurfaceFormat( SurfaceFormatType type );

	/** Creates a surface format from bit count and 4 masks. */
	SurfaceFormat( int bitCount, long redMask, long greenMask, long blueMask, long alphaMask );

	/** Sets true if surfaces using this pixel format can be compressed. */
	void	setCompressable( bool enabled );

	/** Returns type of surface format. */
	SurfaceFormatType	type() const;

	/** 
	 * Returns size (in bytes) of a pixel in the surface pixel format.
	 * Returns 0 if the format is unknown.
	 */
	int		pixelSize() const;

	/** Returns true if the pixels of this format have alpha channel information. */
	bool	hasAlpha() const;

	/** 
	 * Returns ith channel bit mask. 
	 * Order of channel indices are (r,g,b,a).
	 * If the format does not have specified channel then the return value is 0.
	 */
	long	getChannelMask( int i ) const;

	/** 
	 * Returns ith channel bit count. 
	 * Order of channel indices are (r,g,b,a).
	 * If the format does not have specified channel then the return value is 0.
	 */
	int		getChannelBitCount( int i ) const;

	/** 
	 * Copies pixels from one surface pixel format to another.
	 * Only valid for non-compressed formats.
	 *
	 * Usage example: (copy pixels from format RGBA8888 to RGB565)
	 * <pre>
	   SurfaceFormat dstFormat( SURFACE_R5G6B5 );
	   dstFormat.copyPixels( dst, SURFACE_A8R8G8B8, src, pixels );
	   </pre>
	 */
	void	copyPixels( void* dst, const SurfaceFormat& srcFormat, const void* src, int pixels ) const;

	/** Returns true if the formats are the same. */
	bool	operator==( const SurfaceFormat& other ) const;

	/** Returns true if the formats are different. */
	bool	operator!=( const SurfaceFormat& other ) const;

	/** Returns true if this is DXT compressed pixel format. */
	bool	compressed() const;

	/** Returns true if surfaces using this pixel format can be automatically compressed by the graphics driver. */
	bool	compressable() const;

	/** Returns true if blit is supported. */
	bool	bltSupported() const;

private:
	SurfaceFormatType	m_type;
	bool				m_compressable;
};


} // pix


#endif // _PIX_SURFACEFORMAT_H
