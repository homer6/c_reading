#ifndef _SND_SOUNDFORMAT_H
#define _SND_SOUNDFORMAT_H


namespace snd
{


/** 
 * Wave data format: frequency, bits per channel and number of channels. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SoundFormat
{
public:
	SoundFormat();
	SoundFormat( int samplesPerSec, int bitsPerSample, int channels );

	/** Returns sound data frequency, number of samples per second. */
	int		samplesPerSec() const;

	/** Returns number of bits per channel sample. */
	int		bitsPerSample() const;

	/** Number of channels. */
	int		channels() const;

private:
	unsigned short	m_samplesPerSec;
	unsigned char	m_bitsPerSample;
	unsigned char	m_channels;
};


#include "SoundFormat.inl"


} // snd


#endif // _SND_SOUNDFORMAT_H
