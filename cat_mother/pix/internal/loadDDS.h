#include <stdio.h>
#include <stdint.h>
#include <lang/Array.h>
#include <pix/Image.h>
#include <pix/Surface.h>


namespace io {
	class OutputStream;
	class InputStream;}


namespace pix
{


class SurfaceFormat;

enum DDS_Flags {
	DDSD_CAPS			= 0x00000001, 
	DDSD_HEIGHT			= 0x00000002, 
	DDSD_WIDTH			= 0x00000004, 
	DDSD_PITCH			= 0x00000008, 
	DDSD_PIXELFORMAT	= 0x00001000, 
	DDSD_MIPMAPCOUNT	= 0x00020000, 
	DDSD_LINEARSIZE		= 0x00080000, 
	DDSD_DEPTH			= 0x00800000, 
};

enum DDS_PixelFormat_Flags {
	DDPF_ALPHAPIXELS	= 0x00000001, 
	DDPF_FOURCC			= 0x00000004, 
	DDPF_RGB			= 0x00000040, 
};

enum DDS_dwCaps1 {
	DDSCAPS_COMPLEX		= 0x00000008, 
	DDSCAPS_TEXTURE		= 0x00001000, 
	DDSCAPS_MIPMAP		= 0x00400000, 
};

enum DDS_dwCaps2 {
	DDSCAPS2_CUBEMAP			= 0x00000200, 
	DDSCAPS2_CUBEMAP_POSITIVEX	= 0x00000400, 
	DDSCAPS2_CUBEMAP_NEGATIVEX	= 0x00000800, 
	DDSCAPS2_CUBEMAP_POSITIVEY	= 0x00001000, 
	DDSCAPS2_CUBEMAP_NEGATIVEY	= 0x00002000, 
	DDSCAPS2_CUBEMAP_POSITIVEZ	= 0x00004000, 
	DDSCAPS2_CUBEMAP_NEGATIVEZ	= 0x00008000, 
	DDSCAPS2_VOLUME				= 0x00200000,
};

// Why surfaces and directly and not pointers? DDS files can contain a variable amount of bitmaps (including mipmaps) and also contain header data which will be used by the driver

/** 
 * Loads DDS image from the file. 
 * @return true if load ok.
 * @author Toni Aittoniemi, Jani Kajala (jani.kajala@helsinki.fi)
 */
bool loadDDS( io::InputStream* file, int* width, int* height, int* pitch, int* mipMapLevels, SurfaceFormat* format, Image::ImageType* type, lang::Array<Surface>& surfaces );


}
