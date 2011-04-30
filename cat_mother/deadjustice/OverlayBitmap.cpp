#include "OverlayBitmap.h"
#include "ScriptUtil.h"
#include <io/InputStream.h>
#include <io/InputStreamArchive.h>
#include <sg/Mesh.h>
#include <sg/Font.h>
#include <sg/Sprite.h>
#include <sg/TriangleList.h>
#include <dev/Profile.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include <script/VM.h>
#include <script/ScriptException.h>
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

ScriptMethod<OverlayBitmap> OverlayBitmap::sm_methods[] =
{
	//ScriptMethod<OverlayBitmap>( "funcName", OverlayBitmap::script_funcName ),
	ScriptMethod<OverlayBitmap>( "addSprite", script_addSprite ),
	ScriptMethod<OverlayBitmap>( "getSpritePosition", script_getSpritePosition ),
	ScriptMethod<OverlayBitmap>( "getSpriteRotation", script_getSpriteRotation ),
	ScriptMethod<OverlayBitmap>( "getSpriteScale", script_getSpriteScale ),
	ScriptMethod<OverlayBitmap>( "height", script_height ),
	ScriptMethod<OverlayBitmap>( "removeSprite", script_removeSprite ),
	ScriptMethod<OverlayBitmap>( "setSpritePosition", script_setSpritePosition ),
	ScriptMethod<OverlayBitmap>( "setSpriteRotation", script_setSpriteRotation ),
	ScriptMethod<OverlayBitmap>( "setSpriteScale", script_setSpriteScale ),
	ScriptMethod<OverlayBitmap>( "width", script_width )
};

//-----------------------------------------------------------------------------

const float OverlayBitmap::ALIGN_CENTER	= -1e10f;
const float OverlayBitmap::ALIGN_RIGHT	= -1e11f;
const float OverlayBitmap::ALIGN_BOTTOM	= -1e12f;
const float OverlayBitmap::ALIGN_LEFT	= -1e13f;

//-----------------------------------------------------------------------------

OverlayBitmap::OverlayBitmap( VM* vm, io::InputStreamArchive* arch, const String& filename, TriangleList* tri ) :
	GameScriptable( vm, arch, 0, 0 ),
	m_methodBase( -1 ),
	m_arch( arch ),
	m_tri( tri ),
	m_bmp( 0 ),
	m_sprites( new Mesh )
{
	m_methodBase = ScriptUtil<OverlayBitmap,GameScriptable>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );

	// load bitmap
	P(InputStream) imgin = m_arch->getInputStream( filename );
	P(Texture) tex = new Texture( m_arch->getInputStream(filename) );
	imgin->close();

	// create sprite bitmap
	m_bmp = new Sprite( tex, Sprite::createMaterial(), tri );
}

OverlayBitmap::~OverlayBitmap()
{
}

void OverlayBitmap::update( float dt )
{
	GameScriptable::update( dt );
}

Node* OverlayBitmap::getRenderObject( Camera* camera )
{
	GameScriptable::getRenderObject( camera );
	return m_sprites;
}

void OverlayBitmap::render()
{
	// render sprites
	for ( int i = 0 ; i < m_sprites->primitives() ; ++i )
		m_sprites->getPrimitive(i)->draw();
}

void OverlayBitmap::clear()
{
	m_sprites->removePrimitives();
}

int OverlayBitmap::getSpriteIndex( float id )
{
	for ( int i = 0 ; i < m_sprites->primitives() ; ++i )
	{
		void* spritePtr = m_sprites->getPrimitive(i);
		float spriteId = *reinterpret_cast<float*>(&spritePtr);
		if ( spriteId == id )
			return i;
	}
	throw ScriptException( Format("Sprite id not found") );
	return -1;
}

int OverlayBitmap::methodCall( VM* vm, int i )
{
	return ScriptUtil<OverlayBitmap,GameScriptable>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

int	OverlayBitmap::script_addSprite( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects sprite top left x,y", funcName) );
	
	int		param	= 1;
	float	x		= vm->toNumber( param++ );
	float	y		= vm->toNumber( param++ );

	if ( x == ALIGN_CENTER )
		x = screenWidth()*.5f - m_bmp->width()*.5f;
	if ( y == ALIGN_CENTER )
		y = screenHeight()*.5f - m_bmp->height()*.5f;
	if ( x == ALIGN_RIGHT )
		x = screenWidth() - m_bmp->width();
	if ( y == ALIGN_BOTTOM )
		y = screenHeight() - m_bmp->height();

	// create sprite
	P(Sprite) sprite = new Sprite( *m_bmp, Sprite::SHARE_GEOMETRY+Sprite::SHARE_SHADER );
	sprite->setPosition( Vector2(x, y) );
	m_sprites->addPrimitive( sprite );

	void* spritePtr = sprite;
	float id = *reinterpret_cast<float*>(&spritePtr);
	vm->pushNumber( id );
	return 1;
}

int	OverlayBitmap::script_removeSprite( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects sprite id", funcName) );

	float id = vm->toNumber(1);
	int spriteIndex = getSpriteIndex( id );
	m_sprites->removePrimitive( spriteIndex );
	return 0;
}

int OverlayBitmap::script_height( script::VM* vm, const char* )
{
	vm->pushNumber( (float)m_bmp->height() );
	return 1;
}

int OverlayBitmap::script_width( script::VM* vm, const char* )
{
	vm->pushNumber( (float)m_bmp->width() );
	return 1;
}

int OverlayBitmap::script_getSpritePosition( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects sprite id and coordinate axis index [0,2)", funcName) );

	int		param	= 1;
	float	id		= vm->toNumber( param++ );
	int		axis	= getIndex( vm, funcName, 0, 2, 2 );
	Sprite*	sprite	= dynamic_cast<Sprite*>( m_sprites->getPrimitive(getSpriteIndex(id)) );
	assert( sprite );

	vm->pushNumber( sprite->position()[axis] );
	return 1;
}

int OverlayBitmap::script_setSpritePosition( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects sprite id and position x,y", funcName) );

	int		param	= 1;
	float	id		= vm->toNumber( param++ );
	float	x		= vm->toNumber( param++ );
	float	y		= vm->toNumber( param++ );
	Sprite*	sprite	= dynamic_cast<Sprite*>( m_sprites->getPrimitive(getSpriteIndex(id)) );
	assert( sprite );

	sprite->setPosition( Vector2(x, y) );
	return 0;
}

int OverlayBitmap::script_getSpriteRotation( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects sprite id and returns sprite angle in degrees", funcName) );

	int		param	= 1;
	float	id		= vm->toNumber( param++ );
	Sprite*	sprite	= dynamic_cast<Sprite*>( m_sprites->getPrimitive(getSpriteIndex(id)) );
	assert( sprite );

	vm->pushNumber( Math::toDegrees(sprite->rotation()) );
	return 1;
}

int OverlayBitmap::script_setSpriteRotation( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects sprite id and angle in degrees", funcName) );

	int		param	= 1;
	float	id		= vm->toNumber( param++ );
	float	angle	= Math::toRadians( vm->toNumber( param++ ) );
	Sprite*	sprite	= dynamic_cast<Sprite*>( m_sprites->getPrimitive(getSpriteIndex(id)) );
	assert( sprite );

	sprite->setRotation( angle );
	return 0;
}

int OverlayBitmap::script_setSpriteScale( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects sprite id and scale x,y", funcName) );

	int		param	= 1;
	float	id		= vm->toNumber( param++ );
	float	x		= vm->toNumber( param++ );
	float	y		= vm->toNumber( param++ );
	Sprite*	sprite	= dynamic_cast<Sprite*>( m_sprites->getPrimitive(getSpriteIndex(id)) );
	assert( sprite );

	sprite->setScale( Vector2(x, y) );
	return 0;
}

int OverlayBitmap::script_getSpriteScale( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects sprite id and coordinate axis index [0,2)", funcName) );

	int		param	= 1;
	float	id		= vm->toNumber( param++ );
	int		axis	= getIndex( vm, funcName, 0, 2, 2 );
	Sprite*	sprite	= dynamic_cast<Sprite*>( m_sprites->getPrimitive(getSpriteIndex(id)) );
	assert( sprite );

	vm->pushNumber( sprite->scale()[axis] );
	return 1;
}
