#ifndef _IO_READER_H
#define _IO_READER_H


#include <lang/Char.h>
#include <lang/Object.h>
#include <lang/String.h>


namespace io
{


/**
 * Abstract base class for character stream readers.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Reader :
	public lang::Object
{
public:
	///
	virtual ~Reader();

	/**
	 * Reads specified number of characters from the stream.
	 * Skips malformed byte sequences.
	 * @exception IOException
	 * @return Number of character actually read. -1 if end-of-stream occured.
	 */
	virtual long	read( lang::Char* buffer, long count ) = 0;

	/**
	 * Skips specified number of characters from the stream.
	 * @exception IOException
	 * @return Number of character actually skipped.
	 */
	virtual long	skip( long count );

	/** Closes the stream. */
	virtual void	close() = 0;

	/**
	 * Marks the current position in this input stream.
	 * Following calls to the reset method restore
	 * this stream to the last marked position.
	 * If marking is not supported for the stream then 
	 * markSupported() returns false and mark and reset do nothing.
	 * @param readlimit Maximum number of characters to peek.
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
	 * Returns true if this reader supports marking. Default is false.
	 * @see mark
	 * @exception IOException
	 */
	virtual bool	markSupported() const;

	/** Returns name of the reader. */
	virtual lang::String	toString() const = 0;

protected:
	///
	Reader();

private:
	Reader( const Reader& );
	Reader& operator=( const Reader& );
};


} // io


#endif // _IO_READER_H
