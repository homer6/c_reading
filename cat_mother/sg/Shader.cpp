#include "Shader.h"
#include <lang/Exception.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace sg
{


Shader::Shader()
{
	m_pass = 1;
}

Shader::~Shader()
{
}

Shader::Shader( const Shader& other )
{
	m_pass	= other.m_pass;
	m_name	= other.m_name;
}

void Shader::setName( const String& name )
{
	m_name = name;
}

void Shader::setPass( int pass )
{
	m_pass = pass;
}

const String& Shader::name() const
{
	return m_name;
}

int Shader::pass() const
{
	return m_pass;
}

int	Shader::parameters() const
{
	return 0;
}

void Shader::getParameterDesc( int i, ParameterDesc* desc ) const
{
	assert( i >= 0 && i < parameters() ); i=i;
	desc->name = "";
	desc->dataType = PT_UNSUPPORTED;
	desc->dataClass = PC_UNSUPPORTED;
}

void Shader::setBoolean( const lang::String& /*name*/, bool /*value*/ ) 
{
}

void Shader::setInt( const lang::String& /*name*/, int /*value*/ ) 
{
}

void Shader::setFloat( const lang::String& /*name*/, float /*value*/ ) 
{
}


void Shader::setVector4( const lang::String& /*name*/, const math::Vector4& /*value*/ ) 
{
}

void Shader::setMatrix4x4( const lang::String& /*name*/, const math::Matrix4x4& /*value*/ ) 
{
}

void Shader::setMatrix4x4Array( const lang::String& /*name*/, const math::Matrix4x4* /*values*/, int /*count*/ ) 
{
}

void Shader::setMatrix4x4PointerArray( const lang::String& /*name*/, const math::Matrix4x4** /*values*/, int /*count*/ ) 
{
}

void Shader::setColor( const lang::String& /*name*/, const pix::Color& /*value*/ ) 
{
}

void Shader::setTexture( const lang::String& /*name*/, sg::BaseTexture* /*value*/ ) 
{
}

sg::BaseTexture* Shader::getTexture( const lang::String& name ) const
{
	throw Exception( Format("Parameter {0} not found in shader {1}", name, this->name()) );
	return 0;
}

bool Shader::hasParameter( const lang::String& /*name*/ ) const
{
	return false;
}

String Shader::toString( ParameterType pt )
{
	switch ( pt )
	{
	case PT_UNSUPPORTED:return "UNSUPPORTED";
	case PT_BOOL:		return "BOOL";
	case PT_INT:		return "INT";
	case PT_FLOAT:		return "FLOAT";
	case PT_COLOR:		return "COLOR";
	case PT_TEXTURE:	return "TEXTURE";
	case PT_SAMPLER:	return "SAMPLER";
	default:			return "INVALID";
	}
}

String Shader::toString( ParameterClass pc )
{
	switch ( pc )
	{
	case PC_UNSUPPORTED:return "UNSUPPORTED";
	case PC_SCALAR:		return "SCALAR";
	case PC_VECTOR4:	return "VECTOR4";
	case PC_MATRIX4X4:	return "MATRIX4X4";
	case PC_OBJECT:		return "OBJECT";
	default:			return "INVALID";
	}
}

void Shader::getVector4( const lang::String& name, math::Vector4* ) const
{
	throw Exception( Format("Parameter {0} not found in shader {1}", name, this->name()) );
}


} // sg
