#include "OBBox.h"
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace math
{


OBBox::OBBox() :
	m_tm(1.f), m_dim(0.f,0.f,0.f)
{
}

void OBBox::setRotation( const Matrix3x3& rot )
{
	m_tm.setRotation( rot );
}

void OBBox::setTranslation( const Vector3& center )
{
	m_tm.setTranslation( center );
}

void OBBox::setDimensions( const Vector3& dim )
{
	m_dim = dim;
}

int OBBox::getVertices( const Matrix4x4& tm, 
	Vector3* buffer, int bufferSize ) const
{
	assert( bufferSize >= 8 ); bufferSize=bufferSize;

	Matrix4x4 tm2 = tm * m_tm;
	Vector3 x = tm2.rotate( Vector3(m_dim.x,0,0) );
	Vector3 y = tm2.rotate( Vector3(0,m_dim.y,0) );
	Vector3 z = tm2.rotate( Vector3(0,0,m_dim.z) );
	Vector3 pos = tm2.translation();

	int count = 0;
	buffer[count++] = pos + -x + y - z;
	buffer[count++] = pos + x + y - z;
	buffer[count++] = pos + x - y - z;
	buffer[count++] = pos + -x - y - z;
	buffer[count++] = pos + -x + y + z;
	buffer[count++] = pos + x + y + z;
	buffer[count++] = pos + x - y + z;
	buffer[count++] = pos + -x - y + z;

#ifdef _DEBUG
	for ( int i = 0 ; i < count ; ++i )
		assert( buffer[i].finite() );
#endif

	return count;
}

Vector3	OBBox::translation() const
{
	return m_tm.translation();
}

Matrix3x3 OBBox::rotation() const
{
	return m_tm.rotation();
}


} // math
