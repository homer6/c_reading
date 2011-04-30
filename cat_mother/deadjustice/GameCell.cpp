#include "GameCell.h"
#include "GameBSPTree.h"
#include "GamePortal.h"
#include "GameObjectListItem.h"
#include "GameBSPTree.h"
#include "GameObject.h"
#include "GameLevel.h"
#include "CollisionInfo.h"
#include <sg/Light.h>
#include <sg/VertexLock.h>
#include <bsp/BSPFile.h>
#include <bsp/BSPNode.h>
#include <bsp/BSPTreeBuilder.h>
#include <bsp/BSPBalanceSplitSelector.h>
#include <bsp/BSPBoxSplitSelector.h>
#include <io/File.h>
#include <io/FileOutputStream.h>
#include <pix/Color.h>
#include <pix/Colorf.h>
#include <lang/Float.h>
#include <lang/Debug.h>
#include <lang/Exception.h>
#include <lang/Character.h>
#include <math/Vector3.h>
#include <sg/Camera.h>
#include <sg/Node.h>
#include <sg/Mesh.h>
#include <sg/Model.h>
#include <sg/VertexAndIndexLock.h>
#include <sgu/NodeUtil.h>
#include <snd/SoundManager.h>
#include <ps/ParticleSystemManager.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace ps;
using namespace sg;
using namespace sgu;
using namespace pix;
using namespace snd;
using namespace bsp;
using namespace math;
using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

GameCell::GameCell( script::VM* vm, io::InputStreamArchive* arch, 
	snd::SoundManager* soundMgr, ps::ParticleSystemManager* particleMgr,
	const lang::String& name, sg::Node* geometry,
	const lang::String& bspFileName, int bspBuildPolySkip,
	const Vector<P(GameSurface)>& collisionMaterialTypes ) :
	GameScriptable( vm, arch, soundMgr, particleMgr ),
	m_name( name ),
	m_geometry( geometry ),
	m_soundMgr( soundMgr ),
	m_particleMgr( particleMgr ),
	m_portals( Allocator<P(GamePortal)>(__FILE__) ),
	m_bsptree( 0 ),
	m_level( 0 ),
	m_lights( Allocator<P(Light)>(__FILE__) ),
	m_background( 0 ),
	m_visible( true )
{
	// Find all lights in the cell
	Vector<P(Light)> unusedLights( Allocator<P(Light)>(__FILE__) );
	for ( Node* it = m_geometry ; it ; it = it->nextInHierarchy() )
	{
		Light* light = dynamic_cast<Light*>( it );
		if ( light )
		{
			if ( light->name().indexOf("CASTSHADOW") != -1 )
			{
				Debug::println( "Added light {0} to cell {1}", light->name(), name );
				light->worldTransform(); // updates cachedWorldTransform()
				m_lights.add( light );
			}
			else
			{
				Debug::println( "Removed light {0} from cell {1}", light->name(), name );
				unusedLights.add( light );
			}
		}
	}
	for ( int i = 0 ; i < unusedLights.size() ; ++i )
		unusedLights[i]->unlink();

	m_bsptree = new GameBSPTree( arch, m_geometry, bspFileName, bspBuildPolySkip, collisionMaterialTypes, 0 );
}

GameCell::~GameCell()
{
	for ( GameObjectListItem* it = m_objectsInCell.begin() ; it ; )
	{
		GameObjectListItem* next = it->next();
		it->object()->setPosition( 0, Vector3(0,0,0) );
		it = next;
	}
}

void GameCell::setBackground( Node* background )
{
	// disable Z-buffer on background object materials
	for ( Node* node = background ; node ; node = node->nextInHierarchy() )
	{
		Mesh* mesh = dynamic_cast<Mesh*>( node );
		if ( mesh )
		{
			for ( int i = 0 ; i < mesh->primitives() ; ++i )
			{
				Primitive* prim = mesh->getPrimitive(i);
				Shader* shader = prim->shader();
				Material* mat = dynamic_cast<Material*>( shader );
				if ( mat )
				{
					Debug::println( "Disabled depth write for material {0} of object {1}", mat->name(), mesh->name() );
					mat->setDepthWrite( false );
					mat->setDepthFunc( Material::CMP_ALWAYS );
				}
			}
		}
	}

	m_background = background;
}

Node* GameCell::background() const
{
	return m_background;
}

Node* GameCell::getRenderObject( sg::Camera* camera ) 
{
	GameScriptable::getRenderObject( camera );

	if ( camera )
	{
		// set shader parameters
		m_geometry->validateHierarchy();
		for ( Node* node = m_geometry ; node ; node = node->nextInHierarchy() )
		{
			Mesh* mesh = dynamic_cast<Mesh*>( node );
			if ( mesh )
			{
				Light* keylight = getClosestLight( mesh->cachedWorldTransform().translation() );
				for ( int i = 0 ; i < mesh->primitives() ; ++i )
				{
					Primitive* prim = mesh->getPrimitive(i);
					Shader* fx = prim->shader();
					if ( fx && fx->parameters() > 0 )
					{
						setShaderParams( fx, mesh, keylight );
					}
				}
			}
		}
	}

	return m_geometry;
}

void GameCell::setShaderParams( Shader* fx, Mesh* mesh, Light* keylight )
{
	Matrix4x4 worldTmInv = mesh->cachedWorldTransform().inverse();

	for ( int i = 0 ; i < fx->parameters() ; ++i )
	{
		Shader::ParameterDesc desc;
		fx->getParameterDesc( i, &desc );
		
		if ( desc.dataType == Shader::PT_FLOAT && desc.dataClass == Shader::PC_VECTOR4 && 
			desc.name.length() > 0 && Character::isLowerCase(desc.name.charAt(0)) &&
			desc.name.indexOf("Camera") == -1 )
		{
			bool paramResolved = false;
			bool objSpace = ( desc.name.length() > 2 && desc.name.charAt(1) == 'o' );
			int baseNameOffset = objSpace ? 2 : 1;

			String name = desc.name.substring( baseNameOffset );
			Node* node = 0;
			if ( name == "Light1" )
				node = keylight;

			if ( node )
			{
				Char ch = desc.name.charAt(0);
				if ( ch == 'd' )
				{
					Vector3 v = node->worldTransform().rotation().getColumn(2);
					if ( objSpace )
						v = worldTmInv.rotate(v);
					fx->setVector4( desc.name, Vector4(v.x, v.y, v.z, 0.f) );
					paramResolved = true;
				}
				if ( ch == 'p' )
				{
					Vector3 v = node->worldTransform().translation();
					if ( objSpace )
						v = worldTmInv.transform(v);
					fx->setVector4( desc.name, Vector4(v.x, v.y, v.z, 1.f) );
					paramResolved = true;
				}
			}

			if ( !paramResolved )
			{
				Debug::printlnError( "Mesh {2} shader {0} parameter {1} could not be resolved", fx->name(), desc.name, mesh->name() );
			}
		}
	}
}

void GameCell::update( float dt )
{
	GameScriptable::update( dt );
}

void GameCell::addPortal( GamePortal* portal ) 
{
	m_portals.add( portal );
}

const lang::String& GameCell::name() const 
{
	return m_name;
}

int GameCell::portals() const 
{
	return m_portals.size();
}

GamePortal*	GameCell::getPortal( int index ) const 
{
	return m_portals[index].ptr();
}

GameObjectListItem*	GameCell::objectsInCell() const
{
	return m_objectsInCell.begin();
}

bool GameCell::getVisualByBSPPolygonID( int polyid, sg::Mesh** mesh, sg::Model** model, int* triangleIndex, GameSurface** surface ) const
{
	return m_bsptree->getVisualByBSPPolygonID( polyid, mesh, model, triangleIndex, surface );
}

GameBSPTree* GameCell::bspTree() const
{
	return m_bsptree;
}

Light* GameCell::getClosestLight( const math::Vector3& point ) const
{
	Light* closestLt = 0;
	float minDistSqr = Float::MAX_VALUE;

	for ( int i = 0 ; i < m_lights.size() ; ++i )
	{
		Light* lt = m_lights[i];
		float distSqr = ( point - lt->worldTransform().translation() ).lengthSquared();
		if ( distSqr < minDistSqr )
		{
			closestLt = lt;
			minDistSqr = distSqr;
		}
	}

	return closestLt;
}

int	GameCell::lights() const
{
	return m_lights.size();
}

sg::Light* GameCell::getLight( int i ) const
{
	return m_lights[i];
}

int GameCell::objects() const
{
	int count = 0;
	for ( GameObjectListItem* it = objectsInCell() ; it ; it = it->next() )
		++count;
	return count;
}

int GameCell::collidableObjects() const
{
	int count = 0;
	for ( GameObjectListItem* it = objectsInCell() ; it ; it = it->next() )
		if ( it->object()->collidable() )
			++count;
	return count;
}
