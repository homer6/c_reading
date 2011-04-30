#ifndef _SND_SOUNDFILE_H
#define _SND_SOUNDFILE_H


#include <snd/SoundFormat.h>
#include <lang/String.h>
#include <lang/Object.h>


namespace io {
	class InputStream;
	class DataInputStream;
	class InputStreamArchive;}


namespace snd
{


/** 
 * Class for sound file loading and saving. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SoundFile :
	public lang::Object
{
public:
	/** 
	 * Reads sound file header.
	 * @exception IOException
	 */
	SoundFile( const lang::String& name, io::InputStreamArchive* arch );

	///
	~SoundFile();

	/** 
	 * Reads specified number of bytes of sound data from the file. 
	 * @exception IOException
	 */
	void	read( void* data, long bytes );

	/** Returns number of bytes sound data in the sound file. */
	long	size() const;

	/** Returns sound file format. */
	const SoundFormat&	format() const;

private:
	P(io::InputStream)			m_in;
	P(io::DataInputStream)		m_dataIn;
	SoundFormat					m_format;
	long						m_dataBegin;
	long						m_dataEnd;
};


} // snd


#endif // _SND_SOUNDFILE_H
