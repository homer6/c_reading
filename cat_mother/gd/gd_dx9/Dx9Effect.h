#ifndef _DX8EFFECT_H
#define _DX8EFFECT_H


#include <lang/Object.h>
#include "DrvObject.h"
#include <gd/Effect.h>


class Dx9GraphicsDevice;


/**
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Dx9Effect :
	public gd::Effect,
	public DrvObject
{
public:
	Dx9Effect();
	~Dx9Effect();

	int			create( gd::GraphicsDevice* device, const void* data, int size );
	void		destroy();
	void		duplicate( const gd::Effect* other );
	void		resetDeviceObject();
	void		destroyDeviceObject();
	void		addReference();
	void		release();
	void		setBoolean( Parameter* param, bool value );
	void		setInt( Parameter* param, int value );
	void		setColor( Parameter* param, const pix::Color& value );
	void		setFloat( Parameter* param, float value );
	void		setTexture( Parameter* param, gd::BaseTexture* value );
	void		setMatrix4x4( Parameter* param, const math::Matrix4x4& value );
	void		setMatrix4x4Array( Parameter* param, const math::Matrix4x4* values, int count );
	void		setMatrix4x4PointerArray( Parameter* param, const math::Matrix4x4** values, int count );
	void		setVector4( Parameter* param, const math::Vector4& value );
	void		begin( gd::GraphicsDevice* device, int* passes );
	void		apply( int pass );
	void		end();
	void		setVertexFormat( const gd::VertexFormat& vf );
	void		setPolygonSorting( bool enabled );
	gd::VertexFormat vertexFormat() const;
	const char*	lastErrorString() const;
	int			parameters() const;
	void		getVector4( Parameter* param, math::Vector4* value ) const;
	Parameter*	getParameter( int i, Parameter* parent ) const;
	Parameter*	getParameter( const char* name, Parameter* parent ) const;
	void		getParameterDesc( Parameter* param, ParameterDesc* desc ) const;
	const char*	getParameterName( Parameter* param ) const;

private:
	long					m_refs;
	P(Dx9GraphicsDevice)	m_dev;
	gd::VertexFormat		m_vf;
	int						m_d3dfvf;
	ID3DXEffect*			m_fx;
	char*					m_err;
	int						m_params;
	bool					m_sortPolygons;

	void	setError( const char* str );

	Dx9Effect( const Dx9Effect& );
	Dx9Effect& operator=( const Dx9Effect& );
};


#endif // _DX8EFFECT_H
