#include "StdAfx.h"
#include "error.h"
#include "toDx9.h"
#include "toString.h"
#include "Dx9Texture.h"
#include "Dx9GraphicsDevice.h"
#include "Dx9Effect.h"
#include <gd/Errors.h>
#include <pix/Color.h>
#include <math/Vector3.h>
#include <math/Matrix4x4.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace pix;
using namespace math;

//-----------------------------------------------------------------------------

Dx9Effect::Dx9Effect() :
	m_refs(0), m_dev(0), m_vf(), m_d3dfvf(0), m_fx(0), m_err(0), m_params(0), m_sortPolygons(false)
{
}

Dx9Effect::~Dx9Effect()
{
	destroy();
}

void Dx9Effect::destroy() 
{
	destroyDeviceObject();

	if ( m_fx )
	{
		m_fx->Release();
		m_fx = 0;
	}

	if ( m_err )
	{
		delete[] m_err;
		m_err = 0;
	}

	m_vf = gd::VertexFormat();
	m_dev = 0;
	m_params = 0;
}

void Dx9Effect::duplicate( const gd::Effect* other )
{
	destroy();
	
	const Dx9Effect* fx = static_cast<const Dx9Effect*>( other );
	ID3DXEffect* fxclone = 0;
	HRESULT hr = fx->m_fx->CloneEffect( fx->m_dev->d3dDevice(), &fxclone );
	if ( hr != D3D_OK )
		error( "Effect cloning failed: %s", toString(hr) );

	m_dev = fx->m_dev;
	m_vf = fx->m_vf;
	m_d3dfvf = fx->m_d3dfvf;
	m_fx = fxclone;
	m_params = fx->m_params;
	m_sortPolygons = fx->m_sortPolygons;
}

void Dx9Effect::addReference() 
{
	InterlockedIncrement( &m_refs );
}

void Dx9Effect::release() 
{
	if ( 0 == InterlockedDecrement( &m_refs ) )
		delete this;
}

int Dx9Effect::create( gd::GraphicsDevice* device, const void* data, int size ) 
{
	assert( device );

	destroy();

	m_dev = static_cast<Dx9GraphicsDevice*>( device );
	m_dev->resetRenderState();

	// compile effect description
	ID3DXBuffer* xerr = 0;
	D3DXMACRO* defines = 0;
	ID3DXInclude* includeLoader = 0;
	ID3DXEffectPool* effectPool = 0;
	DWORD flags = D3DXSHADER_PACKMATRIX_COLUMNMAJOR;
//#ifdef _DEBUG
	flags |= D3DXSHADER_DEBUG;
//#endif
	HRESULT hr = D3DXCreateEffect( m_dev->d3dDevice(), data, size, defines, includeLoader, flags, effectPool, &m_fx, &xerr );

	// get compilation errors
	if ( xerr )
	{
		if ( hr != D3D_OK )
		{
			char* xerrmsg = (char*)xerr->GetBufferPointer();
			int xerrmsglen = xerr->GetBufferSize();
			delete[] m_err; m_err = 0;
			m_err = new char[xerrmsglen+1];
			m_err[xerrmsglen] = 0;
			strncpy( m_err, xerrmsg, xerrmsglen );
		}
		xerr->Release();
		xerr = 0;
	}

	// report errors
	if ( hr != D3D_OK )
	{
		error( "Failed to compile effect description: %s", m_err );
		return gd::ERROR_EFFECTCOMPILATIONERROR;
	}

	// select technique to be used
	D3DXHANDLE tech;
	hr = m_fx->FindNextValidTechnique( 0, &tech );
	if ( hr != D3D_OK )
	{
		error( "Dx9Effect.FindNextValidTechnique failed: %s", toString(hr) );
		setError( "Effect is not supported by the graphics device." );
		return gd::ERROR_EFFECTUNSUPPORTED;
	}

	// select the technique
	hr = m_fx->SetTechnique( tech );
	if ( hr != D3D_OK )
	{
		error( "Dx9Effect.SetTechnique failed: %s", toString(hr) );
		setError( "Effect is not supported by the graphics device." );
		return gd::ERROR_EFFECTUNSUPPORTED;
	}

	// enum parameters
	D3DXEFFECT_DESC fxDesc;
	hr = m_fx->GetDesc( &fxDesc );
	m_params = fxDesc.Parameters;

	return gd::ERROR_NONE;
}

void Dx9Effect::resetDeviceObject()
{
	if ( m_fx )
		m_fx->OnResetDevice();
}

void Dx9Effect::destroyDeviceObject()
{
	if ( m_fx )
		m_fx->OnLostDevice();
}

void Dx9Effect::setBoolean( Parameter* param, bool value ) 
{
	assert( m_fx );
	assert( sizeof(int) == sizeof(DWORD) );
	
	HRESULT hr = m_fx->SetBool( reinterpret_cast<D3DXHANDLE>(param), value );
	if ( hr != D3D_OK )
		error( "Dx9Effect.setBoolean( %s, %i ) failed: %s", getParameterName(param), value, toString(hr) );
}

void Dx9Effect::setInt( Parameter* param, int value ) 
{
	assert( m_fx );
	assert( sizeof(int) == sizeof(DWORD) );
	
	HRESULT hr = m_fx->SetInt( reinterpret_cast<D3DXHANDLE>(param), value );
	if ( hr != D3D_OK )
		error( "Dx9Effect.setInt( %s, %i ) failed: %s", getParameterName(param), value, toString(hr) );
}

void Dx9Effect::setColor( Parameter* param, const Color& value ) 
{
	assert( m_fx );

	HRESULT hr = m_fx->SetInt( reinterpret_cast<D3DXHANDLE>(param), value.toInt32() );
	if ( hr != D3D_OK )
		error( "Dx9Effect.setColor( %s, %x ) failed: %s", getParameterName(param), value.toInt32(), toString(hr) );
}

void Dx9Effect::setFloat( Parameter* param, float value ) 
{
	assert( m_fx );

	HRESULT hr = m_fx->SetFloat( reinterpret_cast<D3DXHANDLE>(param), value );
	if ( hr != D3D_OK )
		error( "Dx9Effect.setFloat( %s, %g ) failed: %s", getParameterName(param), value, toString(hr) );
}

void Dx9Effect::setTexture( Parameter* param, gd::BaseTexture* value )
{
	assert( m_fx );

	IDirect3DBaseTexture9* d3dtex = 0;
	if ( value )
		d3dtex = static_cast<Dx9BaseTextureImplInterface*>( value->impl() )->getDx9Texture( m_dev );

	HRESULT hr = m_fx->SetTexture( reinterpret_cast<D3DXHANDLE>(param), d3dtex );
	if ( hr != D3D_OK )
		error( "Dx9Effect.setTexture( %s ) failed: %s", getParameterName(param), toString(hr) );
}

void Dx9Effect::setMatrix4x4( Parameter* param, const Matrix4x4& value ) 
{
	assert( m_fx );

	HRESULT hr = m_fx->SetMatrix( reinterpret_cast<D3DXHANDLE>(param), (const D3DXMATRIX*)&value );
	if ( hr != D3D_OK )
		error( "Dx9Effect.setMatrix4x4( %s ) failed: %s", getParameterName(param), toString(hr) );
}

void Dx9Effect::setMatrix4x4Array( Parameter* param, const math::Matrix4x4* values, int count )
{
	assert( m_fx );

	HRESULT hr = m_fx->SetMatrixArray( reinterpret_cast<D3DXHANDLE>(param), (const D3DXMATRIX*)values, count );
	if ( hr != D3D_OK )
		error( "Dx9Effect.setMatrix4x4Array( %s, %i ) failed: %s", getParameterName(param), count, toString(hr) );
}

void Dx9Effect::setMatrix4x4PointerArray( Parameter* param, const math::Matrix4x4** values, int count )
{
	assert( m_fx );

	HRESULT hr = m_fx->SetMatrixPointerArray( reinterpret_cast<D3DXHANDLE>(param), (const D3DXMATRIX**)values, count );
	if ( hr != D3D_OK )
		error( "Dx9Effect.setMatrix4x4PointerArray( %s, %i ) failed: %s", getParameterName(param), count, toString(hr) );
}

void Dx9Effect::setVector4( Parameter* param, const Vector4& value ) 
{
	assert( m_fx );

	D3DXVECTOR4 d3dvec( value.x, value.y, value.z, value.w );

	HRESULT hr = m_fx->SetVector( reinterpret_cast<D3DXHANDLE>(param), &d3dvec );
	if ( hr != D3D_OK )
		error( "Dx9Effect.setVector4( %s ) failed: %s", getParameterName(param), toString(hr) );
}

void Dx9Effect::begin( gd::GraphicsDevice* /*dev*/, int* passes ) 
{
	assert( m_fx );

	if ( !m_d3dfvf )
		m_d3dfvf = m_dev->getDeviceFVF( m_vf );
	m_dev->d3dDevice()->SetFVF( m_d3dfvf );

	UINT passcount = 0;
	HRESULT hr = m_fx->Begin( &passcount, 0 );
	if ( hr != D3D_OK )
		error( "Dx9Effect.begin() failed: %s", toString(hr) );

	m_dev->renderingState().polygonSorting = m_sortPolygons;

	*passes = passcount;
}

void Dx9Effect::apply( int pass ) 
{
	assert( m_fx );

	HRESULT hr = m_fx->Pass( pass );
	if ( hr != D3D_OK )
		error( "Dx9Effect.end() failed: %s", toString(hr) );
}

void Dx9Effect::end() 
{
	assert( m_fx );

	HRESULT hr = m_fx->End();
	if ( hr != D3D_OK )
		error( "Dx9Effect.end() failed: %s", toString(hr) );

	m_dev->renderingState().invalidate();
}

const char* Dx9Effect::lastErrorString() const
{
	if ( m_err )
		return m_err;
	else
		return "";
}

void Dx9Effect::setVertexFormat( const gd::VertexFormat& vf ) 
{
	m_vf = vf;
	m_d3dfvf = 0;
}

gd::VertexFormat Dx9Effect::vertexFormat() const 
{
	return m_vf;
}

int Dx9Effect::parameters() const
{
	return m_params;
}

Dx9Effect::Parameter* Dx9Effect::getParameter( int i, Parameter* /*parent*/ ) const
{
	assert( m_fx );
	D3DXHANDLE handle = m_fx->GetParameter( 0, i );
	return (Parameter*)handle;
}

Dx9Effect::Parameter* Dx9Effect::getParameter( const char* name, Parameter* /*parent*/ ) const
{
	assert( m_fx );
	D3DXHANDLE handle = m_fx->GetParameterByName( 0, name );
	return (Parameter*)handle;
}

const char*	Dx9Effect::getParameterName( Parameter* param ) const
{
	ParameterDesc desc;
	getParameterDesc( param, &desc );
	return desc.name;
}

void Dx9Effect::getParameterDesc( Parameter* param, ParameterDesc* desc ) const
{
	D3DXPARAMETER_DESC paramDesc;
	HRESULT hr = m_fx->GetParameterDesc( reinterpret_cast<D3DXHANDLE>(param), &paramDesc );

	if ( hr == D3D_OK )
	{
		desc->name = paramDesc.Name;
		desc->elements = paramDesc.Elements;

		switch ( paramDesc.Type )
		{
		case D3DXPT_BOOL:		desc->dataType = PT_BOOL; break;
		case D3DXPT_INT:		desc->dataType = PT_INT; break;
		case D3DXPT_FLOAT:		desc->dataType = PT_FLOAT; break;
		case D3DXPT_TEXTURE:	
		case D3DXPT_TEXTURE2D:	
		case D3DXPT_TEXTURE3D:	
		case D3DXPT_TEXTURECUBE:
								desc->dataType = PT_TEXTURE; break;
		case D3DXPT_SAMPLER:	
		case D3DXPT_SAMPLER2D:	
		case D3DXPT_SAMPLER3D:	
		case D3DXPT_SAMPLERCUBE:
								desc->dataType = PT_SAMPLER; break;
		default:				desc->dataType = PT_UNSUPPORTED; break;
		}

		switch ( paramDesc.Class )
		{
		case D3DXPC_SCALAR:			desc->dataClass = PC_SCALAR; break;
		case D3DXPC_VECTOR:			desc->dataClass = PC_VECTOR4; break;
		case D3DXPC_MATRIX_ROWS:	
		case D3DXPC_MATRIX_COLUMNS:	
									desc->dataClass = PC_MATRIX4X4; break;
		case D3DXPC_OBJECT:			desc->dataClass = PC_OBJECT; break;
		default:					desc->dataClass = PC_UNSUPPORTED; break;
		}
	}
	else
	{
		desc->name = "";
		desc->dataType = PT_UNSUPPORTED;
		desc->dataClass = PC_UNSUPPORTED;
		desc->elements = 0;
	}
}

void Dx9Effect::setError( const char* str )
{
	delete[] m_err;
	m_err = new char[ strlen(str)+1 ];
	strcpy( m_err, str );
}

void Dx9Effect::getVector4( Parameter* param, math::Vector4* value ) const
{
	assert( m_fx );

	D3DXVECTOR4 d3dvec;
	HRESULT hr = m_fx->GetVector( reinterpret_cast<D3DXHANDLE>(param), &d3dvec );
	if ( hr != D3D_OK )
		error( "Dx9Effect.getVector4( %s ) failed: %s", getParameterName(param), toString(hr) );
	value->x = d3dvec.x;
	value->y = d3dvec.y;
	value->z = d3dvec.z;
	value->w = d3dvec.w;
}

void Dx9Effect::setPolygonSorting( bool enabled )
{
	m_sortPolygons = enabled;
}
