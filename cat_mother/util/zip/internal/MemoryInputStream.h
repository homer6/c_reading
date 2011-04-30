#ifndef _MEMORYINPUTSTREAM_H
#define _MEMORYINPUTSTREAM_H


#include <io/InputStream.h>
#include <lang/String.h>


namespace util
{


/**
 * Class for reading stream from memory.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class MemoryInputStream :
	public io::InputStream
{
public:
	MemoryInputStream( lang::Object* dataOwner, const void* data, long size, const lang::String& name );

	long			read( void* data, long size );
	void			mark( int readlimit );
	void			reset();
	bool			markSupported() const;
	long			available() const;
	lang::String	toString() const;

private:
	P(lang::Object)		m_dataOwner;
	const void*			m_data;
	long				m_ptr;
	long				m_size;
	long				m_mark;
	lang::String		m_name;

	MemoryInputStream( const MemoryInputStream& );
	MemoryInputStream& operator=( const MemoryInputStream& );
};


} // util


#endif // _MEMORYINPUTSTREAM_H
