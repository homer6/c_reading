#include "Effect.h"
#include "Context.h"
#include "GdUtil.h"
#include <sg/Texture.h>
#include <io/InputStream.h>
#include <io/IOException.h>
#include <gd/Effect.h>
#include <gd/GraphicsDriver.h>
#include <lang/Array.h>
#include <lang/Debug.h>
#include <lang/Exception.h>
#include <stdint.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

namespace sg
{


Effect::Effect( InputStream* in ) :
	m_fx( 0 ),
	m_params( Allocator<TexParam>(__FILE__) ),
	m_vf(),
	m_sort( false )
{
	Vector<uint8_t> src( Allocator<uint8_t>(__FILE__) );
	src.setSize( in->available() );
	if ( src.size() != in->read( src.begin(), src.size() ) )
		throw IOException( Format("Error while reading surface effect: {0}", "") );

	load( src.begin(), src.size(), in->toString() );
}

Effect::Effect( const Effect& other ) :
	Shader( other ),
	m_fx( 0 ),
	m_params( other.m_params ),
	m_vf( other.m_vf ),
	m_sort( other.m_sort )
{
	if ( other.m_fx )
	{
		m_fx = Context::driver()->createEffect();
		m_fx->duplicate( other.m_fx );
	}
}

Effect::~Effect()
{
}

Shader* Effect::clone() const
{
	return new Effect( *this );
}

void Effect::destroy()
{
	unload();
}

void Effect::load()
{
}

void Effect::load( const void* src, int srcBytes, const lang::String& srcName )
{
	if ( !m_fx )
	{
		if ( !Context::initialized() )
			throw Exception( Format("Shader effect initialization requires rendering device to be initialized first") );

		m_fx = Context::driver()->createEffect();
		int err = m_fx->create( Context::device(), src, srcBytes );
		if ( err )
			throw Exception( Format("Failed to initialize surface effect {0}:\n{1}", srcName, m_fx->lastErrorString()) );

		// DEBUG: print fx info
		/*Debug::println( "Loaded effect file {0}:", srcName );
		for ( int i = 0 ; i < parameters() ; ++i )
		{
			ParameterDesc desc;
			getParameterDesc( i, &desc );
			if ( desc.elements > 0 )
				Debug::println( "  param[{0}]: name={1}, type={2}[{4}], class={3}", i, desc.name, toString(desc.dataType), toString(desc.dataClass), desc.elements );
			else
				Debug::println( "  param[{0}]: name={1}, type={2}, class={3}", i, desc.name, toString(desc.dataType), toString(desc.dataClass) );
		}*/
	}
}

void Effect::unload()
{
	m_fx = 0;
}

int Effect::begin()
{
	assert( m_fx );

	// toggle sorting
	m_fx->setPolygonSorting( m_sort );

	// apply vertex format
	gd::VertexFormat gdvf;
	GdUtil::togd( m_vf, &gdvf );
	m_fx->setVertexFormat( gdvf );

	int passes = 0;
	m_fx->begin( Context::device(), &passes );
	return passes;
}

void Effect::end()
{
	assert( m_fx );

	m_fx->end();
}

void Effect::apply( int pass )
{
	assert( m_fx );

	m_fx->apply( pass );
}

void Effect::setInt( const lang::String& name, int value )
{
	Array<char,512> buf( name.length()+1 );
	name.getBytes( buf.begin(), buf.size(), "ASCII-7" );

	m_fx->setInt( m_fx->getParameter(buf.begin()), value );
}

void Effect::setColor( const lang::String& name, const pix::Color& value ) 
{
	Array<char,512> buf( name.length()+1 );
	name.getBytes( buf.begin(), buf.size(), "ASCII-7" );

	m_fx->setColor( m_fx->getParameter(buf.begin()), value );
}

void Effect::setFloat( const lang::String& name, float value ) 
{
	Array<char,512> buf( name.length()+1 );
	name.getBytes( buf.begin(), buf.size(), "ASCII-7" );

	m_fx->setFloat( m_fx->getParameter(buf.begin()), value );
}

void Effect::setTexture( const lang::String& name, sg::BaseTexture* value ) 
{
	Array<char,512> buf( name.length()+1 );
	name.getBytes( buf.begin(), buf.size(), "ASCII-7" );
	gd::Effect::Parameter* param = m_fx->getParameter( buf.begin() );
	if ( !param )
		throw Exception( Format("Texture parameter {0} not found in shader {1}", name, this->name()) );

	m_fx->setTexture( param, value->baseTexture() );

	// update high level reference

	for ( TexParam* it = m_params.begin() ; it != m_params.end() ; ++it )
	{
		if ( it->name == name )
		{
			it->value = value;
			return;
		}
	}

	TexParam tp;
	tp.name = name;
	tp.value = value;
	m_params.add( tp );
}

sg::BaseTexture* Effect::getTexture( const lang::String& name ) const
{
	for ( const TexParam* it = m_params.begin() ; it != m_params.end() ; ++it )
	{
		if ( it->name == name && it->value )
			return it->value;
	}

	throw Exception( Format("Texture parameter {0} not found in shader {1}", name, this->name()) );
	return 0;
}

void Effect::setMatrix4x4( const lang::String& name, const math::Matrix4x4& value ) 
{
	Array<char,512> buf( name.length()+1 );
	name.getBytes( buf.begin(), buf.size(), "ASCII-7" );

	m_fx->setMatrix4x4( m_fx->getParameter(buf.begin()), value );
}

void Effect::setMatrix4x4Array( const lang::String& name, const math::Matrix4x4* values, int count )
{
	Array<char,512> buf( name.length()+1 );
	name.getBytes( buf.begin(), buf.size(), "ASCII-7" );

	m_fx->setMatrix4x4Array( m_fx->getParameter(buf.begin()), values, count );
}

void Effect::setMatrix4x4PointerArray( const lang::String& name, const math::Matrix4x4** values, int count )
{
	Array<char,512> buf( name.length()+1 );
	name.getBytes( buf.begin(), buf.size(), "ASCII-7" );

	m_fx->setMatrix4x4PointerArray( m_fx->getParameter(buf.begin()), values, count );
}

void Effect::setVector4( const lang::String& name, const math::Vector4& value ) 
{
	Array<char,512> buf( name.length()+1 );
	name.getBytes( buf.begin(), buf.size(), "ASCII-7" );

	m_fx->setVector4( m_fx->getParameter(buf.begin()), value );
}

void Effect::setVertexFormat( const VertexFormat& vf )
{
	m_vf = vf;
}

bool Effect::hasParameter( const String& name ) const
{
	Array<char,512> buf( name.length()+1 );
	name.getBytes( buf.begin(), buf.size(), "ASCII-7" );

	gd::Effect::Parameter* param = m_fx->getParameter( buf.begin() );
	return param != 0;
}

VertexFormat Effect::vertexFormat() const
{
	return m_vf;
}

int Effect::parameters() const 
{
	return m_fx->parameters();
}

void Effect::getParameterDesc( int i, ParameterDesc* desc ) const 
{
	assert( m_fx );
	assert( i >= 0 && i < parameters() );

	gd::Effect::ParameterDesc paramDesc;
	m_fx->getParameterDesc( m_fx->getParameter(i), &paramDesc );

	desc->name = paramDesc.name;
	desc->dataType = (ParameterType)( paramDesc.dataType );
	desc->dataClass = (ParameterClass)( paramDesc.dataClass );
	desc->elements = paramDesc.elements;
}

void Effect::getVector4( const lang::String& name, math::Vector4* value ) const
{
	Array<char,512> buf( name.length()+1 );
	name.getBytes( buf.begin(), buf.size(), "ASCII-7" );

	gd::Effect::Parameter* param = m_fx->getParameter( buf.begin() );
	if ( !param )
		throw Exception( Format("Parameter {0} not found in shader {1}", name, this->name()) );
	m_fx->getVector4( param, value );
}

void Effect::setPolygonSorting( bool enabled )
{
	m_sort = enabled;
}


} // sg
