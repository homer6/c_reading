#include "loadJPEG.h"
#include "SurfaceFormat.h"
#include <io/InputStream.h>
#include <io/OutputStream.h>
#include <string.h>
#include <assert.h>
#include "../../external/libjpeg/jpeglib.h"
#include "../../external/libjpeg/jinclude.h"
#include "../../external/libjpeg/jerror.h"
#include "config.h"

//-----------------------------------------------------------------------------

#define INPUT_BUF_SIZE  4096
#define OUTPUT_BUF_SIZE 4096

//-----------------------------------------------------------------------------

using namespace io;

//-----------------------------------------------------------------------------

namespace pix
{


class JpegException {};

//-----------------------------------------------------------------------------

METHODDEF(void) errorExit( j_common_ptr cinfo )
{
	jpeg_abort( cinfo );
	jpeg_destroy( cinfo );
	throw JpegException();
}

METHODDEF(void) outputMessage( j_common_ptr )
{
}

//-----------------------------------------------------------------------------

typedef struct {
	struct jpeg_source_mgr pub;	/* public fields */

	InputStream* infile;		/* source stream */

	JOCTET * buffer;		/* start of buffer */
	boolean start_of_file;	/* have we gotten any data yet? */
} my_source_mgr;

typedef my_source_mgr* pix_src_ptr;

METHODDEF(void)
init_source (j_decompress_ptr cinfo) {
	pix_src_ptr src = (pix_src_ptr) cinfo->src;

	/* We reset the empty-input-file flag for each image,
 	 * but we don't clear the input buffer.
	 * This is correct behavior for reading a series of images from one source.
	*/

	src->start_of_file = TRUE;
}

/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */

METHODDEF(boolean)
fill_input_buffer (j_decompress_ptr cinfo) 
{
	pix_src_ptr src = (pix_src_ptr) cinfo->src;

	size_t nbytes = src->infile->read( src->buffer, INPUT_BUF_SIZE );

	if (nbytes <= 0) {
		if (src->start_of_file)	/* Treat empty input file as fatal error */
			ERREXIT(cinfo, JERR_INPUT_EMPTY);

		WARNMS(cinfo, JWRN_JPEG_EOF);

		/* Insert a fake EOI marker */

		src->buffer[0] = (JOCTET) 0xFF;
		src->buffer[1] = (JOCTET) JPEG_EOI;

		nbytes = 2;
	}

	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = nbytes;
	src->start_of_file = FALSE;

	return TRUE;
}

/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */

METHODDEF(void)
skip_input_data (j_decompress_ptr cinfo, long num_bytes) {
	pix_src_ptr src = (pix_src_ptr) cinfo->src;

	/* Just a dumb implementation for now.  Could use fseek() except
     * it doesn't work on pipes.  Not clear that being smart is worth
	 * any trouble anyway --- large skips are infrequent.
	*/

	if (num_bytes > 0) {
		while (num_bytes > (long) src->pub.bytes_in_buffer) {
		  num_bytes -= (long) src->pub.bytes_in_buffer;

		  (void) fill_input_buffer(cinfo);

		  /* note we assume that fill_input_buffer will never return FALSE,
		   * so suspension need not be handled.
		   */
		}

		src->pub.next_input_byte += (size_t) num_bytes;
		src->pub.bytes_in_buffer -= (size_t) num_bytes;
	}
}

/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF(void)
term_source (j_decompress_ptr /*cinfo*/) {
  /* no work necessary here */
}

/*
 * Prepare for input from a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing decompression.
 */

GLOBAL(void)
jpeg_pix_src (j_decompress_ptr cinfo, InputStream* infile ) {
	pix_src_ptr src;

	// allocate memory for the buffer. is released automatically in the end

	if (cinfo->src == NULL) {
		cinfo->src = (struct jpeg_source_mgr *) (*cinfo->mem->alloc_small)
			((j_common_ptr) cinfo, JPOOL_PERMANENT, SIZEOF(my_source_mgr));

		src = (pix_src_ptr) cinfo->src;

		src->buffer = (JOCTET *) (*cinfo->mem->alloc_small)
			((j_common_ptr) cinfo, JPOOL_PERMANENT, INPUT_BUF_SIZE * SIZEOF(JOCTET));
	}

	// initialize the jpeg pointer struct with pointers to functions
	
	src = (pix_src_ptr) cinfo->src;
	src->pub.init_source = init_source;
	src->pub.fill_input_buffer = fill_input_buffer;
	src->pub.skip_input_data = skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
	src->pub.term_source = term_source;
	src->infile = infile;
	src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
	src->pub.next_input_byte = NULL; /* until buffer loaded */
}

//-----------------------------------------------------------------------------

typedef struct {
	struct jpeg_destination_mgr pub;	/* public fields */

	OutputStream* outfile;		/* destination stream */

	JOCTET * buffer;		/* start of buffer */
} my_destination_mgr;

typedef my_destination_mgr * pix_dst_ptr;

METHODDEF(void)
init_destination (j_compress_ptr cinfo) {
	pix_dst_ptr dest = (pix_dst_ptr) cinfo->dest;

	dest->buffer = (JOCTET *)
	  (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  OUTPUT_BUF_SIZE * SIZEOF(JOCTET));

	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
}

METHODDEF(boolean)
empty_output_buffer (j_compress_ptr cinfo) {
	pix_dst_ptr dest = (pix_dst_ptr) cinfo->dest;

	dest->outfile->write( dest->buffer, OUTPUT_BUF_SIZE );
	//if (dest->outfile->write(dest->buffer, OUTPUT_BUF_SIZE) != OUTPUT_BUF_SIZE)
	//	ERREXIT(cinfo, JERR_FILE_WRITE);

	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;

	return TRUE;
}

METHODDEF(void)
term_destination (j_compress_ptr cinfo) {
	pix_dst_ptr dest = (pix_dst_ptr) cinfo->dest;

	size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;

	/* Write any data remaining in the buffer */

	if (datacount > 0) {
		dest->outfile->write( dest->buffer, datacount );
		//if (dest->outfile->write(dest->buffer, datacount) != datacount)
		//  ERREXIT(cinfo, JERR_FILE_WRITE);
	}
}

GLOBAL(void)
jpeg_pix_dst (j_compress_ptr cinfo, OutputStream* outfile ) {
	pix_dst_ptr dest;

	if (cinfo->dest == NULL) {
		cinfo->dest = (struct jpeg_destination_mgr *)(*cinfo->mem->alloc_small)
			((j_common_ptr) cinfo, JPOOL_PERMANENT, SIZEOF(my_destination_mgr));
	}

	dest = (pix_dst_ptr) cinfo->dest;
	dest->pub.init_destination = init_destination;
	dest->pub.empty_output_buffer = empty_output_buffer;
	dest->pub.term_destination = term_destination;
	dest->outfile = outfile;
}

//-----------------------------------------------------------------------------

uint8_t* loadJPEG( InputStream* in, int* width, int* height, SurfaceFormat* format )
{
	uint8_t* image = 0;

	try
	{
		jpeg_error_mgr errMgr;
		memset( &errMgr, 0, sizeof(errMgr) );
		errMgr.error_exit = errorExit;
		errMgr.output_message = outputMessage;

		jpeg_decompress_struct cinfo;
		memset( &cinfo, 0, sizeof(cinfo) );
		cinfo.err = jpeg_std_error( &errMgr );
		jpeg_create_decompress( &cinfo );

		jpeg_pix_src( &cinfo, in );
		//jpeg_stdio_src( &cinfo, file );
			
		jpeg_read_header( &cinfo, TRUE );
			
		// <set decompression param here>

		jpeg_start_decompress( &cinfo );
		// output_width
		// output_height
		// out_color_components
		// output_components
		// colormap
		// actual_number_of_colors
		*width = cinfo.output_width;
		*height = cinfo.output_height;
		*format = SurfaceFormat::SURFACE_R8G8B8;

		long imgSize = long(cinfo.output_width)*long(cinfo.output_height)*3L;
		image = new uint8_t[ imgSize + cinfo.output_width*cinfo.output_components ];

		while ( cinfo.output_scanline < cinfo.output_height ) 
		{
			long scanlineindex = long(cinfo.output_scanline)*long(cinfo.output_width)*3L;
			uint8_t* scanline = image + scanlineindex;
			uint8_t* scanlinebuff = image + imgSize;
			jpeg_read_scanlines( &cinfo, reinterpret_cast<JSAMPARRAY>(&scanlinebuff), 1 );

			// scanlinebuff -> scanline
			int scanlinepitch = cinfo.output_width*cinfo.output_components;
			for ( int i = 0 ; i < scanlinepitch ; i += cinfo.output_components )
			{
				if ( 3 == cinfo.output_components )
				{
					*scanline++ = scanlinebuff[i+2];
					*scanline++ = scanlinebuff[i+1];
					*scanline++ = scanlinebuff[i+0];
				}
				else if ( 1 == cinfo.output_components )
				{
					*scanline++ = scanlinebuff[i];
					*scanline++ = scanlinebuff[i];
					*scanline++ = scanlinebuff[i];
				}
				else
				{
					throw JpegException();
				}
			}
		}

		jpeg_finish_decompress( &cinfo );
		jpeg_destroy_decompress( &cinfo );
	}
	catch ( JpegException& )
	{
		if ( image ) {delete[] image; image = 0;}
	}
	catch ( ... )
	{
		if ( image ) {delete[] image; image = 0;}
	}
	return image;
}

bool saveJPEG( OutputStream* file, const void* bits, int width, int height, const SurfaceFormat& format )
{
	assert( bits );
	assert( width > 0 );
	assert( height > 0 );
	assert( format != SurfaceFormat::SURFACE_UNKNOWN );

	bool ok = true;
	uint8_t* scanlinebuff = new uint8_t[ 3*width ];

	try
	{
		jpeg_error_mgr errMgr;
		memset( &errMgr, 0, sizeof(errMgr) );
		errMgr.error_exit = errorExit;
		errMgr.output_message = outputMessage;

		jpeg_compress_struct cinfo;
		memset( &cinfo, 0, sizeof(cinfo) );
		cinfo.err = jpeg_std_error( &errMgr );
		jpeg_create_compress( &cinfo );

		//jpeg_stdio_dest( &cinfo, file );
		jpeg_pix_dst( &cinfo, file );

		// set parameters
		//		image_width
		//		image_height
		//		input_components
		//		in_color_space
		SurfaceFormat dstFormat( SurfaceFormat::SURFACE_R8G8B8 );
		cinfo.image_width = width;
		cinfo.image_height = height;
		cinfo.input_components = 3;
		cinfo.in_color_space = ( 3 == cinfo.input_components ? JCS_RGB : JCS_GRAYSCALE );
		jpeg_set_defaults( &cinfo );

		jpeg_start_compress( &cinfo, TRUE );

		for ( int j = 0 ; j < height ; ++j ) 
		{
			long scanlineindex = j*width*format.pixelSize();
			const void* scanline = reinterpret_cast<const uint8_t*>(bits) + scanlineindex;
			
			dstFormat.copyPixels( scanlinebuff, format, scanline, width );
			for ( int k = 0 ; k < width ; ++k )
			{
				uint8_t* pix = scanlinebuff + 3*k;
				uint8_t b = pix[0];
				pix[0] = pix[2];
				pix[2] = b;
			}

			jpeg_write_scanlines( &cinfo, reinterpret_cast<JSAMPARRAY>(&scanlinebuff), 1 );
		}

		jpeg_finish_compress( &cinfo );
		jpeg_destroy_compress( &cinfo );
	}
	catch ( JpegException& )
	{
		ok = false;
	}
	catch ( ... )
	{
		ok = false;
	}

	delete[] scanlinebuff; scanlinebuff = 0;
	return ok;
}


} // pix
