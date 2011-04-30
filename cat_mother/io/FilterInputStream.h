#ifndef _IO_FILTERINPUTSTREAM_H
#define _IO_FILTERINPUTSTREAM_H


#include <io/InputStream.h>


namespace io
{


/**
 * FilterInputStream overrides all methods of InputStream with 
 * versions that pass all requests to the source input stream.
 * The source stream must be exist as long as FilterInputStream is used.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class FilterInputStream :
	public InputStream
{
public:
	/** 
	 * Forwards all operations to specified source stream.
	 */
	explicit FilterInputStream( InputStream* source );

	/**
	 * Closes the source stream and releases associated resources.
	 * Closed stream cannot perform any operations and cannot be reopened.
	 */
	void	close();

	/**
	 * Tries to read specified number of bytes from the source stream.
	 * Doesn't block the caller if specified number of bytes isn't available.
	 *
	 * @return Number of bytes actually read.
	 * @exception IOException
	 */
	long	read( void* data, long size );

	/**
	 * Tries to skip over n bytes from the source stream.
	 *
	 * @return Number of bytes actually skipped.
	 * @exception IOException
	 */
	long	skip( long n );

	/** 
	 * Returns the number of bytes that can be read from the source stream without blocking.
	 *
	 * @exception IOException
	 */
	long	available() const;

	/** Returns name of the source stream. */
	lang::String	toString() const;

private:
	InputStream*	m_source;

	FilterInputStream();
	FilterInputStream( const FilterInputStream& );
	FilterInputStream& operator=( const FilterInputStream& );
};


} // io


#endif // _IO_FILTERINPUTSTREAM_H
