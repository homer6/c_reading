#ifndef _IO_OUTPUTSTREAM_H
#define _IO_OUTPUTSTREAM_H


#include <lang/Object.h>
#include <lang/String.h>


namespace lang {
	class String;}


namespace io
{


/**
 * Base class of all classes writing stream of bytes.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class OutputStream :
	public lang::Object
{
public:
	///
	virtual ~OutputStream();

	/**
	 * Closes the stream and releases associated resources.
	 * Closed stream cannot perform any operations and cannot be reopened.
	 */
	virtual void close();

	/**
	 * Forces any buffered output bytes to be written out.
	 *
	 * @exception IOException
	 */
	virtual void flush();

	/**
	 * Writes specified number of bytes to the stream.
	 *
	 * @exception IOException
	 */
	virtual void write( const void* data, long size ) = 0;

	/** Returns name of the stream. */
	virtual lang::String	toString() const = 0;

protected:
	///
	OutputStream()																	{}

private:
	OutputStream( const OutputStream& );
	OutputStream& operator=( const OutputStream& );
};


} // io


#endif // _IO_OUTPUTSTREAM_H
