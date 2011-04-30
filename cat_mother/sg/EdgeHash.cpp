#include "Edge.h"
#include "EdgeHash.h"
#include <lang/Math.h>
#include <lang/Float.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace sg
{


EdgeHash::EdgeHash( float zeroDistance ) :
	m_zeroDistance(zeroDistance),
	m_zeroDistanceSquared(zeroDistance*zeroDistance)
{
	assert( zeroDistance >= 0.f );

	if ( m_zeroDistance < Float::MIN_VALUE )
		m_zeroDistanceInverse2 = 1.f;
	else
		m_zeroDistanceInverse2 = .5f/zeroDistance;
}

bool EdgeHash::operator()( const Edge& a, const Edge& b ) const
{
	//return a.v0() == b.v0() && a.v1() == b.v1();
	float d = (a.v0() - b.v0()).lengthSquared();
	if ( d > m_zeroDistanceSquared )
		return false;
	d = (a.v1() - b.v1()).lengthSquared();
	if ( d > m_zeroDistanceSquared )
		return false;
	return true;
}

int EdgeHash::operator()( const Edge& e ) const
{
	/*return 
		*reinterpret_cast<const int*>(&e.v0().x) +
		*reinterpret_cast<const int*>(&e.v0().y) +
		*reinterpret_cast<const int*>(&e.v0().z) +
		*reinterpret_cast<const int*>(&e.v1().x) +
		*reinterpret_cast<const int*>(&e.v1().y) +
		*reinterpret_cast<const int*>(&e.v1().z);*/
	int hash = 0;
	for ( int i = 0 ; i < 3 ; ++i ) 
		hash += Math::round(e.v0()[i]*m_zeroDistanceInverse2) + 
			Math::round(e.v1()[i]*m_zeroDistanceInverse2);
	return hash;
}


} // sg
