#include "LOD.h"
#include "Node.h"
#include "Camera.h"
#include "BoundVolume.h"
#include <sg/ViewFrustum.h>
#include <lang/Float.h>
#include <util/Vector.h>
#include <algorithm>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

namespace sg
{


class LODItem
{
public:
	P(Node)		node;
	float		lodMin;
	float		lodMax;

	LODItem() 
	{
		node = 0;
		lodMin = 0.f;
		lodMax = Float::MAX_VALUE;
	}

	LODItem( Node* node, float lodMin, float lodMax )
	{
		this->node = node;
		this->lodMin = lodMin;
		this->lodMax = lodMax;
	}

	bool operator<( const LODItem& other ) const
	{
		return lodMax > other.lodMax;
	}
};

class LOD::LODImpl :
	public Object
{
public:
	Vector<LODItem>		lods;
	float				radius;
	int					level;

	LODImpl() :
		lods( Allocator<LODItem>(__FILE__,__LINE__) ),
		radius( 0 ),
		level( -1 )
	{
	}
};

//-----------------------------------------------------------------------------

LOD::LOD()
{
	m_this = new LODImpl;
}

LOD::~LOD()
{
}

LOD::LOD( const LOD& other )
{
	m_this = new LODImpl( *other.m_this );
}

bool LOD::updateVisibility( Camera* camera )
{
	selectLOD( cachedWorldTransform().translation(), camera );
	return false; // container not visible
}

bool LOD::selectLOD( const Vector3& lodWorldPos, Camera* camera )
{
	Vector3 lodPosInCamera = camera->cachedWorldToCamera().transform( lodWorldPos );

	// check container visibility 
	bool visible = BoundVolume::testSphereVolume(lodPosInCamera, m_this->radius, 
		camera->viewFrustum().planes(), ViewFrustum::PLANE_COUNT);

	int level = -1;
	if ( visible )
	{
		// screen-size in pixels
		float sizePix = camera->getProjectedSize( lodPosInCamera.z, m_this->radius*2.f );

		int lods = m_this->lods.size();
		for ( int i = 0 ; i < lods ; ++i )
		{
			const LODItem& lod = m_this->lods[i];
			bool enabled = (sizePix >= lod.lodMin && sizePix < lod.lodMax);
			if ( enabled && level == -1 )
				level = i;
			m_this->lods[i].node->setEnabled( enabled );
		}
	}
	else
	{
		// disable LODs by default
		int lods = m_this->lods.size();
		for ( int i = 0 ; i < lods ; ++i )
			m_this->lods[i].node->setEnabled( false );
	}

	m_this->level = level;
	return visible;
}

void LOD::render( Camera*, int )
{
}

void LOD::add( Node* lod, float lodMin, float lodMax )
{
	assert( lodMin >= 0.f );
	assert( lodMax > 0.f );
	assert( lodMin <= lodMax );
	assert( lod );

	lod->setEnabled( false );
	m_this->lods.add( LODItem(lod,lodMin,lodMax) );
	std::sort( m_this->lods.begin(), m_this->lods.end() );
}

void LOD::remove( int index )
{
	assert( index >= 0 && index < m_this->lods.size() );

	m_this->lods[index].node->setEnabled( true );
	m_this->lods.remove( index );
}

Node* LOD::get( int index ) const
{
	assert( index >= 0 && index < m_this->lods.size() );

	m_this->lods[index].node->setEnabled( true );
	return m_this->lods[index].node;
}

int	LOD::lods() const
{
	return m_this->lods.size();
}

void LOD::setRadius( float r )
{
	m_this->radius = r;
}

float LOD::radius() const
{
	return m_this->radius;
}

int LOD::level() const
{
	return m_this->level;
}

float LOD::boundSphere() const
{
	return m_this->radius;
}


} // sg
