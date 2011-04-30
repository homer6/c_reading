#include <io/DirectoryInputStreamArchive.h>
#include <sd/SoundDriver.h>
#include <sd/SoundDevice.h>
#include <sd/SoundBuffer.h>
#include <snd/SoundManager.h>
#include <snd/SoundFile.h>
#include <win/FrameWindow.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include <lang/Exception.h>
#include <lang/DynamicLinkLibrary.h>
#include <math/Matrix4x4.h>

//-----------------------------------------------------------------------------

using namespace io;
using namespace sd;
using namespace snd;
using namespace win;
using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

void printFormat( const SoundFormat& format )
{
	Debug::println( "samplesPerSec = {0}", format.samplesPerSec() );
	Debug::println( "bitsPerSample = {0}", format.bitsPerSample() );
	Debug::println( "channels      = {0}", format.channels() );
}

void run( String fname )
{
	// check file name
	if ( fname == "" )
		throw Exception( Format("File name command line argument missing") );

	// init sound drv
	const char drvname[] = "sd_dx8";
	DynamicLinkLibrary dll( drvname );
	createSoundDriverFunc createSoundDriver = (createSoundDriverFunc)dll.getProcAddress( "createSoundDriver" );
	if ( !createSoundDriver )
		throw Exception( Format("Corrupted sound library driver: {0}", drvname) );
	
	// check sound drv version
	getSoundDriverVersionFunc getSoundDriverVersion = (getSoundDriverVersionFunc)dll.getProcAddress( "getSoundDriverVersion" );
	if ( !getSoundDriverVersion )
		throw Exception( Format("Old graphics library driver: {0}", drvname) );
	int ver = getSoundDriverVersion();
	if ( ver < SoundDriver::VERSION )
		throw Exception( Format("Old version ({1,#}) of the graphics library driver: {0}", drvname, ver) );
	if ( ver > SoundDriver::VERSION )
		throw Exception( Format("Too new version ({1,#}) of the graphics library driver: {0}", drvname, ver) );

	// init sound drv
	P(SoundDriver) drv = (*createSoundDriver)();
	if ( !drv )
		throw Exception( Format("Failed to init graphics library driver: {0}", drvname) );

	// init sound device
	P(SoundDevice) dev = drv->createSoundDevice();
	int err = dev->create( 32, 44*1024, 16, 2 );
	if ( err )
		throw Exception( Format("Failed to init sound device") );

	// init directory archive
	P(DirectoryInputStreamArchive) arch = new DirectoryInputStreamArchive;
	arch->addPath( "./" );
	arch->addPath( "/dx8sdk/samples/Multimedia/Media" );
	
	// open sound file
	SoundFile soundFile( fname, arch );
	printFormat( soundFile.format() );
	SoundFormat soundFormat = soundFile.format();

	// create sound buffer
	P(SoundBuffer) sb = drv->createSoundBuffer();
	err = sb->create( dev, soundFile.size(), soundFormat.samplesPerSec(), soundFormat.bitsPerSample(), soundFormat.channels(), SoundBuffer::USAGE_STATIC|SoundBuffer::USAGE_CONTROL3D );
	if ( err )
		throw Exception( Format("Failed to init sound buffer") );

	// load sound data to sound buffer
	void* data = 0;
	int bytes = 0;
	err = sb->lock( 0, soundFile.size(), &data, &bytes, 0, 0, 0 );
	if ( err )
		throw Exception( Format("Failed to lock sound buffer") );
	soundFile.read( data, bytes );
	sb->unlock( data, bytes, 0, 0 );

	// main loop
	while ( Window::flushWindowMessages() )
	{
		if ( GetKeyState(VK_SPACE) < 0 )
			sb->play(0);

		// set position in 3D
		Matrix4x4 tm(1);
		tm.setTranslation( Vector3(5,0,0) );
		sb->setTransform( tm );
		sb->setMinDistance( 1.f );
		sb->setMaxDistance( 10.f );
		sb->commit();
	}

	// deinit sound buffer
	sb->destroy();
	sb = 0;

	// deinit sound device
	dev->destroy();
	dev = 0;

	// deinit sound driver
	drv->destroy();
}

int WINAPI WinMain( HINSTANCE inst, HINSTANCE, LPSTR cmdLine, int cmdShow )
{
	P(FrameWindow) wnd = new FrameWindow;

	try
	{
		wnd->create( "playsnd_framewnd", "snd test", 512, 512, false, inst );
		run( cmdLine );
	}
	catch ( Throwable& e )
	{
		char msg[1024];
		e.getMessage().format().getBytes( msg, sizeof(msg), "ASCII-7" );
		MessageBox( 0, msg, "Error", MB_OK|MB_ICONERROR );
		wnd->destroy();
		return 1;
	}
	return 0;
}

