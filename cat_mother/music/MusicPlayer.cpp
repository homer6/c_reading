#include "MusicPlayer.h"
#include "COMPtr.h"
#include <lang/Array.h>
#include <lang/Debug.h>
#include <lang/Exception.h>
#include "config.h"

//-----------------------------------------------------------------------------

#define SAFE_RELEASE(_I) if (_I) _I->Release(); _I=0

//-----------------------------------------------------------------------------

#pragma comment( lib, "strmiids.lib" )		// exports class identifiers
#pragma comment( lib, "quartz.lib" )		// exports AMGetErrorText

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace music
{


MusicPlayer::MusicPlayer() :
	m_graphBuilder(0),
	m_mediaControl(0),
	m_mediaSeeking(0),
	m_sourceCurrent(0)
{
	IGraphBuilder* graphBuilder = 0;
	HRESULT hr = CoCreateInstance( CLSID_FilterGraph, 0, CLSCTX_INPROC, IID_IGraphBuilder, reinterpret_cast<void**>(&graphBuilder) );
	m_graphBuilder = graphBuilder;
	SAFE_RELEASE( graphBuilder );
	if ( hr != S_OK )
		throw Exception( Format("CoCreateInstance(FilterGraph) failed: {0}", getError(hr)) );

	IMediaControl* mediaControl = 0;
	hr = m_graphBuilder->QueryInterface( IID_IMediaControl, reinterpret_cast<void**>(&mediaControl) );
	m_mediaControl = mediaControl;
	SAFE_RELEASE( mediaControl );
	if ( hr != S_OK )
		throw Exception( Format("IGraphBuilder.QueryInterface(IMediaControl) failed: {0}", getError(hr)) );
	
	IMediaSeeking* mediaSeeking = 0;
	hr = m_graphBuilder->QueryInterface( IID_IMediaSeeking, reinterpret_cast<void**>(&mediaSeeking) );
	m_mediaSeeking = mediaSeeking;
	SAFE_RELEASE( mediaSeeking );
	if ( hr != S_OK )
		throw Exception( Format("IGraphBuilder.QueryInterface(IMediaSeeking) failed: {0}", getError(hr)) );

	IBasicAudio* audiop = 0;
	hr = m_graphBuilder->QueryInterface( IID_IBasicAudio, reinterpret_cast<void**>(&audiop) );
	m_audio = audiop;
	SAFE_RELEASE( audiop );
	if ( hr != S_OK )
		throw Exception( Format("IGraphBuilder.QueryInterface(IID_IBasicAudio) failed: {0}", getError(hr)) );
}

MusicPlayer::~MusicPlayer()
{
}

void MusicPlayer::pause( bool enabled )
{
	if ( enabled )
		m_mediaControl->Stop();
	else
		m_mediaControl->Run();
}

void MusicPlayer::play()
{
	//Debug::println( "Now playing: {0}", m_name );

	// seek graph to the beginning
	LONGLONG pos = 0;
	HRESULT hr = m_mediaSeeking->SetPositions( &pos, AM_SEEKING_AbsolutePositioning, &pos, AM_SEEKING_NoPositioning );
	if ( hr != S_OK )
		throw Exception( Format("IMediaSeeking.SetPositions failed: {0}", getError(hr)) );

	// start the graph
	m_mediaControl->Run();
}

void MusicPlayer::stop()
{
	m_mediaControl->Stop();
}

void MusicPlayer::setSourceFile( const String& name )
{
	COMPtr<IBaseFilter>	sourceNext		= 0;
	COMPtr<IPin>		pin				= 0;

	// get wchar_t filename
	Array<wchar_t,500> fname( name.length()+1 );
	for ( int i = 0 ; i < name.length() ; ++i )
		fname[i] = name.charAt(i);
	fname[name.length()] = 0;

	// add source filter
	IBaseFilter* filter = 0;
	HRESULT hr = m_graphBuilder->AddSourceFilter( fname.begin(), fname.begin(), &filter );
	sourceNext = filter;
	SAFE_RELEASE( filter );
	if ( hr != S_OK )
		throw Exception( Format("IGraphBuilder.AddSourceFilter({0}) failed: {1}", name, getError(hr)) );

	// get output pin
	IPin* pinp = 0;
	hr = sourceNext->FindPin( L"Output", &pinp );
	pin = pinp;
	SAFE_RELEASE( pinp );
	if ( hr != S_OK )
		throw Exception( Format("IBaseFilter.FindPin failed: {0}", getError(hr)) );

	// stop old graph
	hr = m_mediaControl->Stop();
	if ( hr != S_OK )
		throw Exception( Format("IMediaControl.Stop failed: {0}", getError(hr)) );

	// get filters
	IEnumFilters*	filterEnum		= 0;
	int				filterCount		= 0;

	hr = m_graphBuilder->EnumFilters( &filterEnum );
	while ( S_OK == filterEnum->Skip(1) )
		++filterCount;
	Array<COMPtr<IBaseFilter>,100> filters( filterCount );
	filterEnum->Reset();
	for ( int i = 0 ; S_OK == filterEnum->Next(1, &filter, 0) ; ++i )
	{
		filters[i] = filter;
		SAFE_RELEASE( filter );
	}
	SAFE_RELEASE( filterEnum );

	// break filter connections
	for ( int i = 0 ; i < filters.size() ; ++i )
	{
		filter = filters[i];
		m_graphBuilder->RemoveFilter( filter );
		if ( m_sourceCurrent != filter )
			m_graphBuilder->AddFilter( filter, 0 );
	}

	// render output pin
	hr = m_graphBuilder->Render( pin );
	if ( hr != S_OK )
		throw Exception( Format("IGraphBuilder.Render failed: {0}", getError(hr)) );
	m_sourceCurrent = sourceNext;
	sourceNext = 0;
	pin = 0;

	// get audio interface
	IBasicAudio* audiop = 0;
	hr = m_graphBuilder->QueryInterface( IID_IBasicAudio, reinterpret_cast<void**>(&audiop) );
	m_audio = audiop;
	SAFE_RELEASE( audiop );
	if ( hr != S_OK )
		throw Exception( Format("IGraphBuilder.QueryInterface(IID_IBasicAudio) failed: {0}", getError(hr)) );

	m_name = name;
}

void MusicPlayer::setVolume( int vol )
{
	HRESULT hr = m_audio->put_Volume( vol );
	if ( hr != S_OK )
		Debug::printlnError( "music: IBasicAudio.put_Volume failed: {0}", getError(hr) );
		//throw Exception( Format("IBasicAudio.put_Volume failed: {0}", getError(hr)) );
}

int MusicPlayer::volume() const
{
	long vol = 0;
	HRESULT hr = m_audio->get_Volume( &vol );
	if ( hr != S_OK )
		Debug::printlnError( "music: IBasicAudio.get_Volume failed: {0}", getError(hr) );
		//throw Exception( Format("IBasicAudio.put_Volume failed: {0}", getError(hr)) );
	return (int)vol;
}

bool MusicPlayer::stopped() const
{
	LONGLONG cur, stop;
	HRESULT hr = m_mediaSeeking->GetPositions( &cur, &stop );
	return S_OK == hr && cur == stop;
}

String MusicPlayer::getError( HRESULT hr )
{
	char msg[256];
	int count = AMGetErrorText( hr, msg, sizeof(msg) );
	if ( count >= sizeof(msg) )
		count = sizeof(msg)-1;
	msg[count] = 0;
	return msg;
}


} // music
