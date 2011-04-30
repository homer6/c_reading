#include "Dummy.h"
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace math;

//-----------------------------------------------------------------------------

namespace sg
{


Dummy::Dummy() :
	m_boxMin( 0, 0, 0 ),
	m_boxMax( 0, 0, 0 )
{
}

Dummy::Dummy( const Dummy& other ) :
	Node( other ),
	m_boxMin( other.m_boxMin ),
	m_boxMax( other.m_boxMax )
{
}

Node* Dummy::clone() const
{
	return new Dummy( *this );
}

void Dummy::setBox( const Vector3& boxMin, const Vector3& boxMax )
{
	m_boxMin = boxMin;
	m_boxMax = boxMax;
}

const Vector3& Dummy::boxMin() const
{
	return m_boxMin;
}

const Vector3& Dummy::boxMax() const
{
	return m_boxMax;
}


} // sg
