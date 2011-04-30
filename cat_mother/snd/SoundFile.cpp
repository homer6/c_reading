#include "SoundFile.h"
#include <io/IOException.h>
#include <io/InputStream.h>
#include <io/DataInputStream.h>
#include <io/InputStreamArchive.h>
#include <lang/Array.h>
#include <lang/Debug.h>
#include <string.h>
#include <stdint.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;

//-----------------------------------------------------------------------------

namespace snd
{


struct RIFFChunk
{
	char		id[4];
	uint32_t	size;	// size of data
	// data...
};

struct WaveFormatHeader
{ 
	uint16_t	formatTag;
	uint16_t	channels;
	uint32_t	samplesPerSec;
	uint32_t	avgBytesPerSec;
	uint16_t	blockAlign;
}; 

/** 
 * Finds sub RIFF chunk from current chunk.
 * @param in Input stream.
 * @param end Current chunk end.
 * @param id Id of chunk to find.
 * @param chunk [out] Receives found sub chunk info.
 */
static void findChunk( DataInputStream* in, long end, const char* id, RIFFChunk* chunk )
{
	while ( in->size() < end )
	{
		long bytes = in->read( chunk, sizeof(*chunk) );
		if ( bytes != sizeof(*chunk) )
			throw IOException( Format("Failed to read {0} chunk from {1}", id, in->toString()) );

		if ( !memcmp(id,chunk->id,4) )
			break;
		
		in->skip( chunk->size );
	}

	if ( memcmp(id,chunk->id,4) )
		throw IOException( Format("Failed to read {0} chunk from {1}", id, in->toString()) );
}

//-----------------------------------------------------------------------------

SoundFile::SoundFile( const String& name, InputStreamArchive* arch ) :
	m_in( arch->getInputStream( name ) ),
	m_dataIn( new DataInputStream(m_in) ),
	m_format(0,0,0),
	m_dataBegin(0),
	m_dataEnd(0)
{
	DataInputStream& in = *m_dataIn;

	// find RIFF chunk
	RIFFChunk ckRIFF;
	findChunk( &in, in.available(), "RIFF", &ckRIFF );
	long endRIFF = in.size() + ckRIFF.size;

	// form WAVE
	char formWAVE[4];
	in.readFully( formWAVE, 4 );
	if ( memcmp(formWAVE,"WAVE",4) )
		throw IOException( Format("Failed to read WAVE form from {0}", name) );

	// find fmt chunk
	RIFFChunk ckfmt;
	findChunk( &in, endRIFF, "fmt ", &ckfmt );
	long endfmt = in.size() + ckfmt.size;

	// read fmt data
	Array<uint8_t,1000> fmtdata;
	fmtdata.setSize( ckfmt.size );
	in.readFully( fmtdata.begin(), ckfmt.size );
	const WaveFormatHeader* wavefmt = reinterpret_cast<const WaveFormatHeader*>( fmtdata.begin() );
	m_format = SoundFormat( wavefmt->samplesPerSec, wavefmt->avgBytesPerSec / wavefmt->samplesPerSec / wavefmt->channels * 8, wavefmt->channels );

	// leave fmt chunk
	in.skip( endfmt-in.size() );

	// find data chunk
	RIFFChunk ckdata;
	findChunk( &in, endRIFF, "data", &ckdata );
	m_dataBegin = in.size();
	m_dataEnd = in.size() + ckdata.size;

	// debug info
	int samples = size() / (m_format.bitsPerSample()/8) / m_format.channels();
	float sec = (float)samples / (float)m_format.samplesPerSec();
	Debug::println( "Sound {0}: {1}kHz, {2}bits, {3} chn, {4} sec", name, m_format.samplesPerSec()/1000, m_format.bitsPerSample(), m_format.channels(), sec );
}

SoundFile::~SoundFile()
{
	if ( m_in )
		m_in->close();
}

void SoundFile::read( void* data, long bytes ) 
{
	assert( bytes <= size() );

	m_dataIn->readFully( data, bytes );

	assert( size() >= 0 );
}

long SoundFile::size() const 
{
	return m_dataEnd - m_dataIn->size();
}

const SoundFormat& SoundFile::format() const 
{
	return m_format;
}


} // snd
