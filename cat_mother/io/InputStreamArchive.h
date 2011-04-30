#ifndef _IO_INPUTSTREAMARCHIVE_H
#define _IO_INPUTSTREAMARCHIVE_H


#include <lang/Object.h>
#include <lang/String.h>


namespace io
{


class InputStream;


/** 
 * Abstract base to input stream archives. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class InputStreamArchive :
	public lang::Object
{
public:
	///
	InputStreamArchive() {}

	/** Closes the archive. */
	virtual void				close() = 0;

	/** 
	 * Returns input stream to specified entry. 
	 * @exception IOException
	 */
	virtual io::InputStream*	getInputStream( const lang::String& name ) = 0;

	/** 
	 * Returns input stream to ith entry. 
	 * @exception IOException
	 */
	virtual io::InputStream*	getInputStream( int index ) = 0;

	/** 
	 * Returns ith entry name from the archive. 
	 * @exception IOException
	 */
	virtual lang::String		getEntry( int index ) const = 0;

	/** 
	 * Returns number of entries in the archive. 
	 * @exception IOException
	 */
	virtual int					size() const = 0;

	/** Returns name of the archive. */
	virtual lang::String		toString() const = 0;

private:
	InputStreamArchive( const InputStreamArchive& );
	InputStreamArchive& operator=( const InputStreamArchive& );
};


} // io


#endif // _IO_INPUTSTREAMARCHIVE_H
