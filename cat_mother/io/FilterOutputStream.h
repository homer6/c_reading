#ifndef _IO_FILTEROUTPUTSTREAM_H
#define _IO_FILTEROUTPUTSTREAM_H


#include <io/OutputStream.h>


namespace io
{


/**
 * FilterOutputStream overrides all methods of OutputStream with 
 * versions that pass all requests to the target output stream.
 * The target stream must be exist as long as FilterOutputStream is used.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class FilterOutputStream :
	public OutputStream
{
public:
	/** 
	 * Forwards all operations to specified target stream.
	 */
	explicit FilterOutputStream( OutputStream* target );

	/**
	 * Closes the target stream and releases associated resources.
	 * Closed stream cannot perform any operations and cannot be reopened.
	 */
	void	close();

	/**
	 * Forces any buffered output bytes to be written out to the target stream.
	 *
	 * @exception IOException
	 */
	 void	flush();

	/**
	 * Writes specified number of bytes to the target stream.
	 *
	 * @exception IOException
	 */
	 void	write( const void* data, long size );

	 /** Returns name of the target stream. */
	lang::String	toString() const;

private:
	OutputStream*	m_target;

	FilterOutputStream();
	FilterOutputStream( const FilterOutputStream& );
	FilterOutputStream& operator=( const FilterOutputStream& );
};


} // io


#endif // _IO_FILTEROUTPUTSTREAM_H
