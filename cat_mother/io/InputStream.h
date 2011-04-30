#ifndef _IO_INPUTSTREAM_H
#define _IO_INPUTSTREAM_H


#include <lang/Object.h>
#include <lang/String.h>


namespace lang {
	class String;}


namespace io
{


/**
 * Base class of all classes reading stream of bytes.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class InputStream :
	public lang::Object
{
public:
	///
	virtual ~InputStream();

	/**
	 * Closes the stream and releases associated resources.
	 * Closed stream cannot perform any operations and cannot be reopened.
	 */
	virtual void	close();

	/**
	 * Tries to read specified number of bytes from the stream.
	 * Doesn't block the caller if specified number of bytes isn't available.
	 *
	 * @return Number of bytes actually read.
	 * @exception IOException
	 */
	virtual long	read( void* data, long size ) = 0;

	/**
	 * Tries to skip over n bytes from the stream.
	 *
	 * @return Number of bytes actually skipped.
	 * @exception IOException
	 */
	virtual long	skip( long n );

	/**
	 * Marks the current position in this input stream.
	 * Following calls to the reset method restore
	 * this stream to the last marked position.
	 * If marking is not supported for the stream then 
	 * markSupported() returns false and mark and reset do nothing.
	 * @exception IOException
	 */
	virtual void	mark( int readlimit );

	/**
	 * Restore this stream to the last marked position.
	 * @see mark
	 * @exception IOException
	 */
	virtual void	reset();

	/**
	 * Returns true if this stream supports marking. Default is false.
	 * @see mark
	 */
	virtual bool	markSupported() const;

	/** 
	 * Returns the number of bytes that can be read from the stream without blocking.
	 * @exception IOException
	 * @exception IOException
	 */
	virtual long	available() const = 0;

	/** Returns name of the stream. */
	virtual lang::String	toString() const = 0;

protected:
	///
	InputStream();

private:
	InputStream( const InputStream& );
	InputStream& operator=( const InputStream& );
};


} // io


#endif // _IO_INPUTSTREAM_H
