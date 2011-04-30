#include "OverlayFont.h"
#include "ScriptUtil.h"
#include <io/InputStream.h>
#include <io/InputStreamArchive.h>
#include <sg/Mesh.h>
#include <sg/Font.h>
#include <sg/Context.h>
#include <sg/TriangleList.h>
#include <gd/GraphicsDevice.h>
#include <dev/Profile.h>
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

ScriptMethod<OverlayFont> OverlayFont::sm_methods[] =
{
	//ScriptMethod<OverlayFont>( "funcName", OverlayFont::script_funcName ),
	ScriptMethod<OverlayFont>( "addText", script_addText ),
	ScriptMethod<OverlayFont>( "getHeight", script_getHeight ),
	ScriptMethod<OverlayFont>( "getWidth", script_getWidth ),
	ScriptMethod<OverlayFont>( "getTextPosition", script_getTextPosition ),
	ScriptMethod<OverlayFont>( "replaceText", script_replaceText ),
	ScriptMethod<OverlayFont>( "removeText", script_removeText ),
	ScriptMethod<OverlayFont>( "removeTexts", script_removeTexts ),
	ScriptMethod<OverlayFont>( "setTextPosition", script_setTextPosition ),
	ScriptMethod<OverlayFont>( "getGlyphString", script_getGlyphString ),
	ScriptMethod<OverlayFont>( "getGlyphIndex", script_getGlyphIndex ),
	ScriptMethod<OverlayFont>( "numGlyphs", script_numGlyphs ),
};

//-----------------------------------------------------------------------------

const float OverlayFont::ALIGN_CENTER	= -1e10f;
const float OverlayFont::ALIGN_RIGHT	= -1e11f;
const float OverlayFont::ALIGN_BOTTOM	= -1e12f;
const float OverlayFont::ALIGN_LEFT		= -1e13f;

//-----------------------------------------------------------------------------

OverlayFont::OverlayFont( VM* vm, io::InputStreamArchive* arch, const String& fontName, TriangleList* tri ) :
	GameScriptable( vm, arch, 0, 0 ),
	m_arch( arch ),
	m_font( 0 ),
	m_texts( Allocator<TextItem>(__FILE__) ),
	m_textId( -1 )
{
	m_methodBase = ScriptUtil<OverlayFont,GameScriptable>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );

	// load font
	P(InputStream) imgin = m_arch->getInputStream( fontName+".tga" );
	P(InputStream) csin = m_arch->getInputStream( fontName+".txt" );
	m_font = new Font( imgin, csin, tri );
	csin->close();
	imgin->close();
}

OverlayFont::~OverlayFont()
{
}

void OverlayFont::update( float dt )
{
	GameScriptable::update( dt );
}

Node* OverlayFont::getRenderObject( Camera* camera )
{
	GameScriptable::getRenderObject( camera );
	return 0;
}

void OverlayFont::render()
{
	assert( m_font );

	// render text items
	for ( int k = 0 ; k < m_texts.size() ; ++k )
	{
		TextItem& item = m_texts[k];
		m_font->drawText( item.x, item.y, item.str );
	}
}

void OverlayFont::clear()
{
	m_texts.clear();
	m_textId = 0;
}

bool OverlayFont::hasTextItemIndex( float id ) const
{
	for ( int i = 0 ; i < m_texts.size() ; ++i )
	{
		if ( m_texts[i].id == id )
			return true;
	}
	return false;
}

int OverlayFont::getTextItemIndex( float id ) const
{
	for ( int i = 0 ; i < m_texts.size() ; ++i )
	{
		if ( m_texts[i].id == id )
			return i;
	}
	throw ScriptException( Format("Text item id not found") );
	return -1;
}

int OverlayFont::methodCall( VM* vm, int i )
{
	return ScriptUtil<OverlayFont,GameScriptable>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

int	OverlayFont::script_addText( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects text top left x, y and text string", funcName) );

	int			param		= 1;
	float		x			= vm->toNumber( param++ );
	float		y			= vm->toNumber( param++ );
	String		str			= vm->toString( param++ );

	if ( x == ALIGN_CENTER )
		x = screenWidth()*.5f - m_font->getWidth(str)*.5f;
	if ( y == ALIGN_CENTER )
		y = screenHeight()*.5f - m_font->height()*.5f;
	if ( x == ALIGN_RIGHT )
		x = screenWidth() - m_font->getWidth(str);
	if ( y == ALIGN_BOTTOM )
		y = screenHeight() - m_font->height();

	TextItem item;
	item.str = str;
	item.x = x;
	item.y = y;
	item.id = (float)m_textId++;
	m_texts.add( item );

	vm->pushNumber( item.id );
	return 1;
}

int	OverlayFont::script_replaceText( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects text id and string", funcName) );

	int			param		= 1;
	float		id			= vm->toNumber( param++ );
	String		str			= vm->toString( param++ );

	TextItem& item = m_texts[ getTextItemIndex(id) ];
	item.str = str;

	return 0;
}

int	OverlayFont::script_removeText( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects text id number returned by addText", funcName) );

	int			param		= 1;
	float		id			= vm->toNumber( param++ );

	if ( !hasTextItemIndex(id) )
		return 0;

	m_texts.remove( getTextItemIndex(id) );
	return 0;
}

int	OverlayFont::script_removeTexts( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} removes all added texts", funcName) );

	m_texts.clear();
	return 0;
}

int	OverlayFont::script_getWidth( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects text string", funcName) );

	int			param		= 1;
	String		str			= vm->toString( param++ );

	vm->pushNumber( m_font->getWidth(str) );
	return 1;
}

int	OverlayFont::script_getHeight( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects text string", funcName) );

	vm->pushNumber( m_font->height() );
	return 1;
}

int OverlayFont::script_setTextPosition( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects text id and x, y", funcName) );
	
	float id = vm->toNumber(1);
	float x = vm->toNumber(2);
	float y = vm->toNumber(3);

	TextItem& item = m_texts[ getTextItemIndex( id ) ];
	item.x = x;
	item.y = y;
	return 0;
}

int OverlayFont::script_getTextPosition( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects text id and axis index", funcName) );

	float id = vm->toNumber(1);
	int index = getIndex( vm, funcName, 0, 2, 2 );

	TextItem& item = m_texts[ getTextItemIndex( id ) ];
	if ( index == 0 )
		vm->pushNumber( item.x );
	else
		vm->pushNumber( item.y );
	return 1;
}

int OverlayFont::script_getGlyphIndex( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} gets glyph index by string, pass 1 string value", funcName) );

	String str = vm->toString(1);

	vm->pushNumber( (float)m_font->getGlyphIndex(str) );
	return 1;
}

int OverlayFont::script_getGlyphString( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} gets glyph string by index, pass 1 number value", funcName) );

	int index = (int)vm->toNumber(1);

	vm->pushString( m_font->getGlyphString(index) );
	return 1;
}

int OverlayFont::script_numGlyphs( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns number of glyphs", funcName) );
	
	vm->pushNumber( (float)m_font->numGlyphs() );
	return 1;
}

