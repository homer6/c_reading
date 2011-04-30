#ifndef _MUSIC_MUSICPLAYER_H
#define _MUSIC_MUSICPLAYER_H


#include <lang/Object.h>
#include <lang/String.h>
#include <music/internal/COMPtr.h>


#ifndef WIN32
#error MusicPlayer has not been implemented on non-Win32 platforms
#endif
#include <dshow.h>


namespace music
{


/**
 * @see MusicManager
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */	
class MusicPlayer :
	public lang::Object
{
public:
	MusicPlayer();

	~MusicPlayer();

	void	play();

	void	stop();

	void	pause( bool enabled );

	void	setSourceFile( const lang::String& name );

	void	setVolume( int vol );

	int		volume() const;

	bool	stopped() const;

private:
	COMPtr<IGraphBuilder>	m_graphBuilder;
	COMPtr<IMediaControl>	m_mediaControl;
	COMPtr<IMediaSeeking>	m_mediaSeeking;
	COMPtr<IBasicAudio>		m_audio;
	COMPtr<IBaseFilter>		m_sourceCurrent;
	lang::String			m_name;

	static lang::String getError( HRESULT hr );

	MusicPlayer( const MusicPlayer& );
	MusicPlayer& operator=( const MusicPlayer& );
};


} // music


#endif // _MUSIC_MUSICPLAYER_H
