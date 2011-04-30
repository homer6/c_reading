#include "OverlayDisplay.h"
#include "GameController.h"
#include "ScriptUtil.h"
#include <io/InputStream.h>
#include <io/InputStreamArchive.h>
#include <sg/Mesh.h>
#include <sg/Font.h>
#include <sg/Sprite.h>
#include <sg/TriangleList.h>
#include <dev/Profile.h>
#include <snd/SoundManager.h>
#include <lang/Debug.h>
#include <script/VM.h>
#include <script/ScriptException.h>
#include <util/ExProperties.h>
#include <assert.h>
#include <string.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace sg;
using namespace lang;
using namespace util;
using namespace math;
using namespace anim;
using namespace script;

//-----------------------------------------------------------------------------

ScriptMethod<OverlayDisplay> OverlayDisplay::sm_methods[] =
{
	//ScriptMethod<OverlayDisplay>( "funcName", OverlayDisplay::script_funcName ),
	ScriptMethod<OverlayDisplay>( "clear", script_clear ),
	ScriptMethod<OverlayDisplay>( "createFont", script_createFont ),
	ScriptMethod<OverlayDisplay>( "createBitmap", script_createBitmap ),
	ScriptMethod<OverlayDisplay>( "enabled", script_enabled ),
	ScriptMethod<OverlayDisplay>( "format", script_format ),
	ScriptMethod<OverlayDisplay>( "height", script_height ),
	ScriptMethod<OverlayDisplay>( "pause", script_pause ),
	ScriptMethod<OverlayDisplay>( "setEnabled", script_setEnabled ),
	ScriptMethod<OverlayDisplay>( "setPause", script_setPause ),
	ScriptMethod<OverlayDisplay>( "setTime", script_setTime ),
	ScriptMethod<OverlayDisplay>( "time", script_time ),
	ScriptMethod<OverlayDisplay>( "width", script_width ),
	ScriptMethod<OverlayDisplay>( "crosshairPos", script_crosshairPos ),
};

//-----------------------------------------------------------------------------

OverlayDisplay::OverlayDisplay( VM* vm, InputStreamArchive* arch, snd::SoundManager* soundMgr, GameController* gamecontroller ) :
	GameScriptable( vm, arch, soundMgr, 0 ),
	m_methodBase( -1 ),
	m_arch( arch ),
	m_gameController( gamecontroller ),
	m_enabled( true ),
	m_paused( false ),
	m_time( 0.f ),
	m_dt( 0.f ),
	m_fonts( Allocator<P(OverlayFont)>(__FILE__) ),
	m_bitmaps( Allocator<P(OverlayBitmap)>(__FILE__) ),
	m_tri( Sprite::createTriangleList(10*6) )
{
	m_methodBase = ScriptUtil<OverlayDisplay,GameScriptable>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

OverlayDisplay::~OverlayDisplay()
{
}

void OverlayDisplay::update( float dt )
{
	GameScriptable::update( dt );

	if ( !m_paused )
		m_time += dt;

	m_dt += dt;
}

Node* OverlayDisplay::getRenderObject( Camera* camera )
{
	GameScriptable::getRenderObject( camera );
	return 0;
}

void OverlayDisplay::render()
{
	if ( m_enabled )
	{
		// refresh on-screen display info
		if ( hasMethod("signalRefresh") )
		{
			pushMethod( "signalRefresh" );
			vm()->pushNumber( m_dt );
			call( 1, 0 );
		}

		// render sprites
		for ( int i = 0 ; i < m_bitmaps.size() ; ++i )
			m_bitmaps[i]->render();

		// render fonts
		for ( int i = 0 ; i < m_fonts.size() ; ++i )
			m_fonts[i]->render();
	}

	m_dt = 0.f;
}

int OverlayDisplay::methodCall( VM* vm, int i )
{
	return ScriptUtil<OverlayDisplay,GameScriptable>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

int	OverlayDisplay::script_createFont( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects font name", funcName) );
	
	String fontName = vm->toString(1);

	P(OverlayFont) overlay = new OverlayFont( vm, m_arch, fontName, m_tri );
	m_fonts.add( overlay );

	vm->pushTable( overlay );
	return 1;
}

int	OverlayDisplay::script_createBitmap( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects bitmap name", funcName) );
	
	String fname = vm->toString(1);

	P(OverlayBitmap) overlay = new OverlayBitmap( vm, m_arch, fname, m_tri );
	m_bitmaps.add( overlay );

	vm->pushTable( overlay );
	return 1;
}

int OverlayDisplay::script_height( script::VM* vm, const char* )
{
	vm->pushNumber( screenHeight() );
	return 1;
}

int OverlayDisplay::script_width( script::VM* vm, const char* )
{
	vm->pushNumber( screenWidth() );
	return 1;
}

int OverlayDisplay::script_format( script::VM* vm, const char* funcName )
{
	if ( !vm->isString(1) )
		throw ScriptException( Format("Function {0} excepts format string as first parameter", funcName) );

	const int MAX_ARGC = 10;
	Formattable argv[MAX_ARGC];
	int argc = 0;
	for ( int i = 2 ; i <= vm->top() && argc < MAX_ARGC ; ++i )
	{
		switch ( vm->getType(i) )
		{
		case VM::TYPE_STRING:	argv[argc] = vm->toString(i); break;
		case VM::TYPE_NUMBER:	argv[argc] = vm->toNumber(i); break;
		default:				argv[argc] = Formattable(); break;
		}
		++argc;
	}
		
	Format fmt( vm->toString(1), argc, argv );
	vm->pushString( fmt.format() );
	return 1;
}

int OverlayDisplay::script_setTime( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects time in seconds", funcName) );

	m_time = vm->toNumber(1);
	return 0;
}

int OverlayDisplay::script_time( script::VM* vm, const char* )
{
	vm->pushNumber( m_time );
	return 1;
}

int OverlayDisplay::script_pause( script::VM* vm, const char* )
{
	if ( m_paused )
		vm->pushNil();
	else
		vm->pushNumber( 1.f );
	return 1;
}

int	OverlayDisplay::script_setPause( script::VM* vm, const char* )
{
	m_paused = !vm->isNil( 1 );
	return 0;
}

int OverlayDisplay::script_enabled( script::VM* vm, const char* )
{
	if ( m_enabled )
		vm->pushNil();
	else
		vm->pushNumber( 1.f );
	return 1;
}

int	OverlayDisplay::script_setEnabled( script::VM* vm, const char* )
{
	m_enabled = !vm->isNil( 1 );
	return 0;
}

int	OverlayDisplay::script_clear( script::VM*, const char* )
{
	remove( "signalRefresh" );

	for ( int i = 0 ; i < m_fonts.size() ; ++i )
		m_fonts[i]->clear();

	for ( int i = 0 ; i < m_bitmaps.size() ; ++i )
		m_bitmaps[i]->clear();

	return 0;
}

int OverlayDisplay::script_crosshairPos( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects axis number", funcName) );

	int axis = (int)vm->toNumber(1);

	if ( axis == 1 ) 
		vm->pushNumber( m_gameController->crosshairPos().x );
	else if ( axis == 2 )
		vm->pushNumber( m_gameController->crosshairPos().y );
	else
		throw ScriptException( Format("{0} was requested an invalid axis! Specify 1 or 2", funcName ) );

	return 1;
}

