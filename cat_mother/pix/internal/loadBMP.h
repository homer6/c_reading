#include <stdio.h>
#include <stdint.h>


namespace io {
	class OutputStream;
	class InputStream;}


namespace pix
{


class SurfaceFormat;


/** 
 * Loads BMP image from the file.
 * @return Image data. 0 if the image cannot be loaded. delete[] returned array after use.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
uint8_t* loadBMP( io::InputStream* file, int* width, int* height, SurfaceFormat* format );

/** 
 * Saves image to a file in BMP format.
 * @return true if image saved ok.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
bool saveBMP( io::OutputStream* file, const void* bits, int width, int height, const SurfaceFormat& format );


} // pix
