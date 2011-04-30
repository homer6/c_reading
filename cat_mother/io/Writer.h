#ifndef _IO_WRITER_H
#define _IO_WRITER_H


#include <lang/Char.h>
#include <lang/Object.h>
#include <lang/String.h>


namespace io
{


/**
 * Abstract base class for character stream writers.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Writer :
	public lang::Object
{
public:
	///
	virtual ~Writer();

	/**
	 * Writes specified number of characters to the stream.
	 * Ignores invalid character pairs.
	 * @exception IOException
	 */
	virtual void	write( const lang::Char* buffer, long count ) = 0;

	/**
	 * Flushes the stream.
	 * @exception IOException
	 */
	virtual void	flush() = 0;

	/** Closes the stream. */
	virtual void	close() = 0;

	/** Returns name of the writer. */
	virtual lang::String	toString() const = 0;

protected:
	///
	Writer();

private:
	Writer( const Writer& );
	Writer& operator=( const Writer& );
};


} // io


#endif // _IO_WRITER_H
