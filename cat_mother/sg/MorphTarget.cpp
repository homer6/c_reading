#include "MorphTarget.h"
#include <sg/Model.h>
#include <lang/Float.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

namespace sg
{


MorphTarget::MorphTarget() :
	m_deltas( Allocator<Delta>(__FILE__) ),
	m_scale(0.f)
{
}

MorphTarget::~MorphTarget() 
{
}

MorphTarget::MorphTarget( const MorphTarget& other, int shareFlags ) :
	Primitive( other, shareFlags ),
	m_deltas( other.m_deltas ),
	m_scale( other.m_scale ),
	m_name( other.m_name ),
	m_materialName( other.m_materialName )
{
}

Primitive* MorphTarget::clone( int shareFlags ) const
{
	return new MorphTarget( *this, shareFlags );
}

void MorphTarget::load() 
{
}

void MorphTarget::unload() 
{
}

void MorphTarget::draw()
{
}

void MorphTarget::destroy() 
{
	m_deltas.clear();
	m_deltas.trimToSize();
	m_scale = 0.f;
	Primitive::destroy();
}

void MorphTarget::addDelta( int vertexIndex, const Vector3& delta, float scale ) 
{
	assert( scale > 0.f );

#ifdef _DEBUG
	float scaleDiff = scale*(1.f/16383.f) - m_scale;
	scaleDiff = Math::abs( scaleDiff );
#endif
	assert( m_deltas.size() == 0 || scaleDiff < 1e-10f );
	assert( vertexIndex >= 0 && vertexIndex < 32768 );
	assert( scale >= Float::MIN_VALUE );

	m_scale = scale*(1.f/16383.f);
	float inverseScale = 1.f / m_scale;

	Delta d;
	d.vertexIndex = (uint16_t)vertexIndex;
	// d -> [-1,1]
	d.dx = (uint16_t)(delta.x*inverseScale + 16384.f);
	d.dy = (uint16_t)(delta.y*inverseScale + 16384.f);
	d.dz = (uint16_t)(delta.z*inverseScale + 16384.f);

	m_deltas.add( d );
}

void MorphTarget::addDeltas( Delta* deltas, int count, float scale )
{
	assert( scale > 0.f );

	m_scale = scale*(1.f/16383.f);
	for ( int i = 0 ; i < count ; ++i )
		m_deltas.add( deltas[i] );
}

void MorphTarget::apply( Model* model, float weight ) 
{
	assert( m_scale > 0.f || m_deltas.size() == 0 );
	assert( isValidBase(model) );

	if ( weight > Float::MIN_VALUE && m_scale > Float::MIN_VALUE )
	{
		float* vdata = 0;
		int vpitch = 0;
		model->getVertexPositionData( &vdata, &vpitch );

		float s = m_scale * weight;
		int deltaCount = m_deltas.size();
		for ( int i = 0 ; i < deltaCount ; ++i )
		{
			const Delta& delta = m_deltas[i];
			Vector3 d( 
				((float)delta.dx - 16384.f) * s, 
				((float)delta.dy - 16384.f) * s, 
				((float)delta.dz - 16384.f) * s );

			float* p = vdata + vpitch*(int)delta.vertexIndex;
			p[0] += d.x;
			p[1] += d.y;
			p[2] += d.z;
		}
	}
}

void MorphTarget::setName( const String& name )
{
	m_name = name;
}

const String& MorphTarget::name() const
{
	return m_name;
}

void MorphTarget::setMaterialName( const String& name )
{
	m_materialName = name;
}

const String& MorphTarget::materialName() const
{
	return m_materialName;
}

bool MorphTarget::isValidBase( const Model* model ) const
{
	int verts = model->vertices();
	for ( int i = 0 ; i < m_deltas.size() ; ++i )
	{
		const Delta& delta = m_deltas[i];
		if ( delta.vertexIndex >= verts )
			return false;
	}
	return true;
}

VertexFormat MorphTarget::vertexFormat() const
{
	return VertexFormat();
}


} // sg
