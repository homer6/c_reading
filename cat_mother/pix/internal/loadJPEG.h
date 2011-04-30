#include <stdio.h>
#include <stdint.h>


namespace io {
	class InputStream;
	class OutputStream;}


namespace pix
{


class SurfaceFormat;


/** 
 * Loads JPEG image from the file.
 * @return Image data. 0 if the image cannot be loaded. delete[] returned array after use.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
uint8_t* loadJPEG( io::InputStream* in, int* width, int* height, SurfaceFormat* format );

/** 
 * Saves image to a file in JPEG format.
 * @return true if image saved ok.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
bool saveJPEG( io::OutputStream* file, const void* bits, int width, int height, const SurfaceFormat& format );


} // pix
