#include "GameObject.h"
#include "GameLevel.h"
#include "GameRenderPass.h"
#include "GameCell.h"
#include "GameCamera.h"
#include "GameCharacter.h"
#include "GameWeapon.h"
#include "GameNoiseManager.h"
#include "GameBSPTree.h"
#include "ScriptUtil.h"
#include "CollisionInfo.h"
#include <io/InputStream.h>
#include <io/ByteArrayOutputStream.h>
#include <io/InputStreamArchive.h>
#include <ps/ParticleSystemManager.h>
#include <sg/PointLight.h>
#include <sg/ShadowVolume.h>
#include <sg/ShadowShader.h>
#include <pix/Colorf.h>
#include <bsp/BSPCollisionUtil.h>
#include <bsp/BSPNode.h>
#include <bsp/BSPPolygon.h>
#include <sgu/ShadowUtil.h>
#include <dev/Profile.h>
#include <util/Vector.h>
#include <lang/Debug.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Exception.h>
#include <script/VM.h>
#include <script/ClassTag.h>
#include <script/ScriptException.h>
#include <snd/SoundManager.h>
#include <algorithm>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace bsp;
using namespace io;
using namespace sg;
using namespace sgu;
using namespace pix;
using namespace lang;
using namespace math;
using namespace util;
using namespace script;

//-----------------------------------------------------------------------------

const Matrix3x3 GameObject::CHARACTER_MESH_PRE_ROTATION( Vector3(0,1,0), Math::PI );

ScriptMethod<GameObject> GameObject::sm_methods[] =
{
	//ScriptMethod<GameObject>( "funcName", GameObject::script_funcName ),
	ScriptMethod<GameObject>( "addLocalVelocity", script_addLocalVelocity ),
	ScriptMethod<GameObject>( "addWorldVelocity", script_addWorldVelocity ),
	ScriptMethod<GameObject>( "alignRotation", script_alignRotation ),
	ScriptMethod<GameObject>( "boundSphere", script_boundSphere ),
	ScriptMethod<GameObject>( "cell", script_cell ),
	ScriptMethod<GameObject>( "disableDynamicShadow", script_disableDynamicShadow ),
	ScriptMethod<GameObject>( "enableDynamicShadow", script_enableDynamicShadow ),
	ScriptMethod<GameObject>( "getAngleTo", script_getAngleTo ),
	ScriptMethod<GameObject>( "getDistanceTo", script_getDistanceTo ),
	ScriptMethod<GameObject>( "getGameObject", script_getGameObject ),
	ScriptMethod<GameObject>( "getLocalVelocity", script_getLocalVelocity ),
	ScriptMethod<GameObject>( "getPosition", script_getPosition ),
	ScriptMethod<GameObject>( "getSignedAngleTo", script_getSignedAngleTo ),
	ScriptMethod<GameObject>( "getSignedWorldAngleTo", script_getSignedWorldAngleTo ),
	ScriptMethod<GameObject>( "getVelocity", script_getVelocity ),
	ScriptMethod<GameObject>( "getUp", script_getUp ),
	ScriptMethod<GameObject>( "getRight", script_getRight ),
	ScriptMethod<GameObject>( "getForward", script_getForward ),
	ScriptMethod<GameObject>( "hidden", script_hidden ),
	ScriptMethod<GameObject>( "hide", script_hide ),
	ScriptMethod<GameObject>( "isOnRightSide", script_isOnRightSide ),
	ScriptMethod<GameObject>( "isInFront", script_isInFront ),
	ScriptMethod<GameObject>( "lookAt", script_lookAt ),
	ScriptMethod<GameObject>( "projectPositionOnForward", script_projectPositionOnForward ),
	ScriptMethod<GameObject>( "projectPositionOnRight", script_projectPositionOnRight ),
	ScriptMethod<GameObject>( "rotateY", script_rotateY ),
	ScriptMethod<GameObject>( "setDynamicShadow", script_setDynamicShadow ),
	ScriptMethod<GameObject>( "setBoundSphere", script_setBoundSphere ),
	ScriptMethod<GameObject>( "setPosition", script_setPosition ),
	ScriptMethod<GameObject>( "setRotationToIdentity", script_setRotationToIdentity ),
	ScriptMethod<GameObject>( "setVelocity", script_setVelocity ),
	ScriptMethod<GameObject>( "speed", script_speed ),
	ScriptMethod<GameObject>( "unhide", script_unhide ),
};

//-----------------------------------------------------------------------------

P(Vector<GameObject::GameObjectDistance>)	sm_collisionObjects = 0;
int											sm_gameObjects = 0;

//-----------------------------------------------------------------------------

GameObject::MovementState::MovementState() : 
	rot(1.f), 
	pos(0.f,0.f,0.f), 
	vel(0.f,0.f,0.f) 
{
}

//-----------------------------------------------------------------------------

GameObject::GameObject( script::VM* vm, io::InputStreamArchive* arch, snd::SoundManager* soundMgr, 
					   ps::ParticleSystemManager* particleMgr, GameNoiseManager* noiseMgr ) :
	GameScriptable( vm, arch, soundMgr, particleMgr ),
	m_methodBase( -1 ),
	m_ms(),
	m_boundSphere( 0.f ),
	m_cell( 0 ),
	m_noiseMgr( noiseMgr ),
	m_renderCount( 0 ),
	m_renderedInFrame( -1 ),
	m_visible( true ),
	m_hidden( false ),
	m_collidable( false ),
	m_shadowMesh( 0 ),
	m_shadowName( "" ),
	m_shadowLength( 0.f ),
	m_shadowDirty( false ),
	m_keylight( new PointLight )
{
	m_methodBase = -1;
	if ( vm )
		m_methodBase = ScriptUtil<GameObject,GameScriptable>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );

	m_primaryCellItem = GameObjectListItem( this );

	++sm_gameObjects;
	if ( !sm_collisionObjects )
		sm_collisionObjects = new Vector<GameObject::GameObjectDistance>( Allocator<GameObject::GameObjectDistance>(__FILE__) );
}

GameObject::~GameObject()
{
	if ( m_noiseMgr )
		m_noiseMgr->removeNoisesBySource( this );

	if ( m_shadowMesh )
		m_shadowMesh->unlink();

	setCell( 0 );

	if ( --sm_gameObjects == 0 )
		sm_collisionObjects = 0;
}

void GameObject::setCell( GameCell* cell ) 
{
	if ( cell != m_cell )
	{
		if ( m_cell )
			m_cell->m_objectsInCell.remove( &m_primaryCellItem );

		m_cell = cell;

		if ( m_cell )
			m_cell->m_objectsInCell.insert( &m_primaryCellItem );
	}
}

GameCell* GameObject::cell() const 
{
	assert( m_cell );
	return m_cell;
}

bool GameObject::inCell() const 
{
	return 0 != m_cell;
}

GameLevel* GameObject::level() const 
{
	assert( m_cell );
	assert( m_cell->m_level );
	return m_cell->m_level;
}

void GameObject::setMovementState( const MovementState& ms )
{
	m_ms = ms;
}

const GameObject::MovementState& GameObject::movementState() const
{
	return m_ms;
}

void GameObject::setTransform( GameCell* cell, const Matrix4x4& tm )
{
	setCell( cell );
	m_ms.pos	= tm.translation();
	m_ms.rot	= tm.rotation();
}

void GameObject::setPosition( GameCell* cell, const Vector3& pos )
{
	setCell( cell );
	m_ms.pos	= pos;
}

void GameObject::setRotation( const Matrix3x3& rot )
{
	m_ms.rot	= rot;
}

Matrix4x4 GameObject::transform() const
{
	return Matrix4x4( m_ms.rot, m_ms.pos );
}

Vector3 GameObject::right() const
{
	return Vector3( m_ms.rot(0,0), m_ms.rot(1,0), m_ms.rot(2,0) );
}

Vector3 GameObject::up() const
{
	return Vector3( m_ms.rot(0,1), m_ms.rot(1,1), m_ms.rot(2,1) );
}

Vector3 GameObject::forward() const
{
	return Vector3( m_ms.rot(0,2), m_ms.rot(1,2), m_ms.rot(2,2) );
}

Vector3 GameObject::normal() const
{
	return Vector3(0,0,0);
}

float GameObject::speed() const
{
	return m_ms.vel.length();
}

void GameObject::moveWithoutColliding( const Vector3& delta )
{
	GameCell* targetCell = 0;
	moveWithoutCollidingRecurse( cell(), position(), delta, &targetCell );
	setPosition( targetCell, position()+delta );
	assert( cell() );
}

void GameObject::moveWithoutCollidingRecurse( GameCell* cell, const Vector3& position, const Vector3& delta, GameCell** targetCell )
{
	assert( cell );

	*targetCell = cell;
	for ( int i = 0; i < cell->portals(); ++i )
	{
		GamePortal* portal = cell->getPortal(i);
		if ( delta.dot(portal->normal()) < 0.f )
		{
			bool isCrossingPortal = portal->isCrossing( position, delta );
			if ( isCrossingPortal )
				moveWithoutCollidingRecurse( portal->target(), position, delta, targetCell );
		}
	}
}

void GameObject::move( const Vector3& delta, CollisionInfo* cinfo )
{
	dev::Profile pr( "move" );

	assert( cell() );

	// make sure we always have CollisionInfo buffer before moveRecurse
	CollisionInfo cinfo2;
	if ( !cinfo )
		cinfo = &cinfo2;

	// recurse and check collisions
	*cinfo = CollisionInfo( position()+delta, cell() );
	sm_collisionObjects->clear();
	{dev::Profile pr( "move.moveRecurse" );
	moveRecurse( cell(), position(), delta, cinfo );}

	// sort potentially colliding objects by distance
	std::sort( sm_collisionObjects->begin(), sm_collisionObjects->end() );
	sm_collisionObjects->setSize( std::unique( sm_collisionObjects->begin(), sm_collisionObjects->end() ) - sm_collisionObjects->begin() );

	// check dynamic collisions
	{dev::Profile pr( "move.checkCollisionsAgainstObjects" );
	checkCollisionsAgainstObjects( position(), cinfo->position()-position(), *sm_collisionObjects, cinfo );}

	// recurse and update position
	{dev::Profile pr( "move.moveWithoutColliding" );
	moveWithoutColliding( cinfo->position()-position() );}
}

void GameObject::moveRecurse( GameCell* cell, const Vector3& position, const Vector3& delta, CollisionInfo* cinfo )
{
	// check collisions against this cell
	{dev::Profile pr( "move.checkCollisionsAgainstCell" );
	checkCollisionsAgainstCell( position, delta, cell->bspTree()->root(), cell, cinfo );}
	assert( cinfo->positionCell() );

	// list potentially colliding objects
	for ( GameObjectListItem* it = cell->objectsInCell() ; it ; it = it->next() )
	{
		GameObject* obj = it->object();
		if ( obj->collidable() && obj != this )
		{
			float distanceSquared = (obj->position() - position).lengthSquared();

			GameObjectDistance od;
			od.distanceSquared = distanceSquared;
			od.object = obj;
			sm_collisionObjects->add( od );
		}
	}

	// recurse to other cells
	if ( !cinfo->isCollision(CollisionInfo::COLLIDE_ALL) )
	{
		for ( int i = 0; i < cell->portals(); ++i )
		{
			GamePortal* portal = cell->getPortal(i);
			Vector3 delta2 = cinfo->position() - position;
			if ( delta2.dot(portal->normal()) < 0.f )
			{
				bool isCrossingPortal = portal->isCrossing( position, delta2 );
				bool isTouchingPortal = portal->isCrossing( position, portal->normal()*-boundSphere() );
				if ( isCrossingPortal || isTouchingPortal )
				{
					GameCell* targetCell = portal->target();
					if ( isCrossingPortal )
						*cinfo = CollisionInfo( position+delta2, cell );
					moveRecurse( targetCell, position, delta2, cinfo );
				}
			}
		}
	}
}

GameSurface* GameObject::getCollisionMaterial( const CollisionInfo& cinfo ) const
{
	GameSurface* surface = level()->defaultCollisionMaterialType();
	if ( cinfo.isCollision(CollisionInfo::COLLIDE_BSP) )
		cinfo.bspTree()->getVisualByBSPPolygonID( cinfo.polygon()->id(), 0, 0, 0, &surface );
	return surface;
}

void GameObject::checkCollisionsAgainstCell( const Vector3& /*start*/, const Vector3& /*delta*/, BSPNode* /*bsptree*/, GameCell* /*collisionCell*/, CollisionInfo* /*cinfo*/ )
{
}

void GameObject::checkCollisionsAgainstObjects( const Vector3& /*start*/, const Vector3& /*delta*/, const Vector<GameObjectDistance>& /*objects*/, CollisionInfo* /*cinfo*/ )
{
}

void GameObject::alignRotation( const Vector3& up )
{
	assert( Math::abs(up.length()-1.f) < 1e-3f );

	// orthogonalize rotation
	Vector3 xaxis = m_ms.rot.getColumn(0);
	Vector3 right = ( xaxis - up*xaxis.dot(up) ).normalize();
	Vector3 forward = right.cross( up );
	m_ms.rot.setColumn( 0, right );
	m_ms.rot.setColumn( 1, up );
	m_ms.rot.setColumn( 2, forward );
}

void GameObject::update( float dt )
{
	if ( !m_cell )
		throw Exception( Format("Tried to update object {0} which is not in any cell", name()) );

	GameScriptable::update( dt );
}

Node* GameObject::getRenderObject( Camera* camera )
{
	GameScriptable::getRenderObject( camera );

	if ( camera )
	{
		findKeylight();

		// update shadows if needed
		if ( m_shadowDirty && m_shadowMesh )
		{
			Vector3 worldDir( 0,-1,0 );
			Node* node = getRenderObject(0);
			ShadowUtil::updateDynamicShadow( node, m_shadowMesh,
				m_shadowName, worldDir, m_shadowLength, 0.f );

			P(ShadowShader) shadowShader = new ShadowShader;
			shadowShader->setPass( GameRenderPass::RENDERPASS_SHADOW_VOLUME );
			ShadowUtil::setShadowVolumeShaders( node, shadowShader );

			m_shadowDirty = false;
		}

		// update shadow direction
		if ( m_keylight && m_shadowMesh )
		{
			Vector3 worldLightDir = ( position() - m_keylight->position() ).normalize();
			for ( Node* node = m_shadowMesh ; node ; node = node->nextInHierarchy() )
			{
				Mesh* mesh = dynamic_cast<Mesh*>( node );
				if ( mesh )
				{
					for ( int i = 0 ; i < mesh->primitives() ; ++i )
					{
						Primitive* prim = mesh->getPrimitive(i);
						ShadowVolume* shadow = dynamic_cast<ShadowVolume*>( prim );
						if ( shadow )
							shadow->setDynamicShadow( worldLightDir, m_shadowLength );
					}
				}
			}
		}
	}

	return 0;
}

void GameObject::lookAt( const Vector3& target )
{
	// src -> target (world space)
	Vector3 up( 0, 1, 0 );
	Matrix4x4 sourceToWorld = transform();
	Vector3 sourceRotZ = target - sourceToWorld.translation();
	if ( sourceRotZ.lengthSquared() > Float::MIN_VALUE )
	{
		// src->target direction (world space)
		sourceRotZ = sourceRotZ.normalize();

		// src rotation (world space)
		Vector3 sourceRotX = up.cross( sourceRotZ );
		if ( sourceRotX.lengthSquared() > Float::MIN_VALUE )
			sourceRotX = sourceRotX.normalize();
		else
			sourceRotX = Vector3(1,0,0);
		Vector3 sourceRotY = sourceRotZ.cross( sourceRotX );
		Matrix3x3 sourceRot;
		sourceRot.setColumn( 0, sourceRotX );
		sourceRot.setColumn( 1, sourceRotY );
		sourceRot.setColumn( 2, sourceRotZ );

		setRotation( sourceRot );
	}
}

float GameObject::getAngleTo( const Vector3& pos ) const
{
	// turning delta & direction
	Vector3 delta = pos - position();
	float deltaLen = delta.length();
	if ( deltaLen <= Float::MIN_VALUE )
		return 0.f;
	Vector3 dir = delta.normalize();

	// turning delta & direction
	const float RAD_90 = Math::PI * 0.5f;
	float turn = dir.dot( right() );
	float angle = Math::acos( Math::min( 1.f, Math::max(-1.f,turn) ) ) - RAD_90;
	
	if ( dir.dot(forward()) < 0.f )
	{
		if ( angle < 0.f )
			angle -= RAD_90;
		else
			angle += RAD_90;
	}

	return angle;
}

float GameObject::getSignedAngleTo( const Vector3& pos ) const
{
	Vector3 delta = pos - position();
	float deltaLen = delta.length();
	if ( deltaLen <= Float::MIN_VALUE )
		return 0.f;
	Vector3 dir = delta.normalize();

	float turn = dir.dot( forward() );
	bool onright = ( dir.dot( right() ) > 0.f );
	
	float angle = Math::acos( turn );
	if ( onright ) 
		angle = -angle;

	return angle;
}

float GameObject::getSignedWorldAngleTo( const Vector3& pos ) const
{
	Vector3 delta = pos - position();
	float deltaLen = delta.length();
	if ( deltaLen <= Float::MIN_VALUE )
		return 0.f;
	Vector3 dir = delta.normalize();

	float turn = dir.dot( Vector3(0.f, 0.f, 1.f) );
	bool onright = ( dir.dot( Vector3(1.f, 0.f, 0.f) ) > 0.f );
	
	float angle = Math::acos( turn );
	if ( onright ) 
		angle = -angle;

	return angle;
}

Vector3 GameObject::center() const
{
	return m_ms.pos;
}

void GameObject::setBoundSphere( float radius )
{
	m_boundSphere = radius;
}

bool GameObject::isOnRightSide( const Vector3& point) const
{
	Vector3 topoint = point - position();
	if ( right().dot( topoint ) > 0.f )
		return true;
	else
		return false;
}

bool GameObject::isInFront( const Vector3& point) const
{
	Vector3 topoint = point - position();
	if ( forward().dot( topoint ) > 0.f )
		return true;
	else
		return false;
}

void GameObject::incrementRenderCount()
{
	++m_renderCount;
	m_renderCount &= 0x7FFFFFFF;
}

void GameObject::findKeylight()
{
	if ( m_cell )
	{
		if ( m_cell->lights() > 0 )
		{
			float	strengthSum = 0.f;
			Vector3 keylightPos	(0,0,0);
			Vector3 pos			= position();
			float	strengthMax = 0.f;

			for ( int i = 0 ; i < m_cell->lights() ; ++i )
			{
				Light* lt = m_cell->getLight(i);

				// lit vertex (with inverse distance attenuation)
				Vector3 worldLight = lt->worldTransform().translation() - pos;
				float lenSqr = worldLight.lengthSquared();
				lenSqr *= lenSqr;
				if ( lenSqr > Float::MIN_VALUE )
					worldLight *= 1.f / lenSqr;
				float dif = worldLight.dot( Vector3(0,1,0) );
				if ( dif > 0.f )
				{
					// update weighted key light position
					float strength = dif;
					keylightPos += lt->worldTransform().translation() * strength;
					strengthSum += strength;

					if ( strength > strengthMax )
					{
						strengthMax = strength;
						m_keylight->setName( lt->name() );
					}
				}
			}

			if ( strengthSum > Float::MIN_VALUE )
			{
				keylightPos *= 1.f/strengthSum;
				m_keylight->setPosition( keylightPos );
			}
		}
		else
		{
			Debug::printlnWarning( "Cell {0} does not have CASTSHADOW tagged lights", m_cell->name() );

			GameLevel* level = this->level();
			for ( int i = 0 ; i < level->cells() ; ++i )
			{
				GameCell* cell = level->getCell(i);
				if ( cell->lights() > 0 )
				{
					m_keylight->setPosition( cell->getLight(0)->worldTransform().translation() );
					break;
				}
			}
		}
	}
}

void GameObject::forceVisible()
{
	setVisible( true );
	m_renderedInFrame = GameCamera::frameCount();
}

Light* GameObject::keylight() const
{
	return m_keylight;
}

GameNoiseManager* GameObject::noiseManager() const
{
	return m_noiseMgr;
}

int GameObject::methodCall( VM* vm, int i )
{
	return ScriptUtil<GameObject,GameScriptable>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

int GameObject::script_setBoundSphere( script::VM* vm, const char* funcName ) 
{
	return getParam( vm, funcName, "meters", &m_boundSphere );
}

int GameObject::script_boundSphere( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns object bounding sphere radius", funcName) );
	vm->pushNumber( m_boundSphere );
	return 1;
}

int	GameObject::script_getPosition( script::VM* vm, const char* funcName )
{
	int index = getIndex( vm, funcName, 0, 3 );
	vm->pushNumber( m_ms.pos[index] );
	return 1;
}


int	GameObject::script_getVelocity( script::VM* vm, const char* funcName )
{
	int index = getIndex( vm, funcName, 0, 3 );
	vm->pushNumber( m_ms.vel[index] );
	return 1;
}

int GameObject::script_getLocalVelocity( script::VM* vm, const char* funcName )
{
	int index = getIndex( vm, funcName, 0, 3 );
	Vector3 lvel = m_ms.rot.inverse() * m_ms.vel;
	vm->pushNumber( lvel[index] );
	return 1;
}

int	GameObject::script_setVelocity( script::VM* vm, const char* funcName )
{
	float v[3];
	int rv = getParams( vm, funcName, "m/s", v, 3 );
	setVelocity( Vector3(v[0],v[1],v[2]) );
	return rv;
}

int	GameObject::script_lookAt( script::VM* vm, const char* funcName )
{
	GameObject* obj = 0;
	Vector3 pos( 0.f, 0.f, 0.f );
	getGameObjectOrVector3( vm, funcName, &obj, &pos );

	if ( obj )
		pos = obj->position();
	lookAt( pos );
	return 0;
}

int	GameObject::script_addLocalVelocity( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects object space delta velocity (x,y,z)", funcName) );
	
	float x = vm->toNumber( 1 );
	float y = vm->toNumber( 2 );
	float z = vm->toNumber( 3 );
	Vector3 xaxis = right();
	Vector3 yaxis = up();
	Vector3 zaxis = forward();
	
	Vector3 dv(0,0,0);
	dv += xaxis * x;
	dv += yaxis * y;
	dv += zaxis * z;
	setVelocity( velocity() + dv );

	return 0;
}

int	GameObject::script_addWorldVelocity( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects world space delta velocity (x,y,z)", funcName) );
	
	float x = vm->toNumber( 1 );
	float y = vm->toNumber( 2 );
	float z = vm->toNumber( 3 );
	Vector3 xaxis = Vector3(1,0,0);
	Vector3 yaxis = Vector3(0,1,0);
	Vector3 zaxis = Vector3(0,0,1);
	
	Vector3 dv(0,0,0);
	dv += xaxis * x;
	dv += yaxis * y;
	dv += zaxis * z;
	setVelocity( velocity() + dv );

	return 0;
}

int	GameObject::script_alignRotation( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects world space vector (x,y,z) representing up", funcName) );
	
	float x = vm->toNumber( 1 );
	float y = vm->toNumber( 2 );
	float z = vm->toNumber( 3 );
	Vector3 xaxis = Vector3(1,0,0);
	Vector3 yaxis = Vector3(0,1,0);
	Vector3 zaxis = Vector3(0,0,1);

	Vector3 up(0,0,0);
	up += xaxis * x;
	up += yaxis * y;
	up += zaxis * z;
	alignRotation( up );

	return 0;
}

int GameObject::script_setRotationToIdentity( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} sets rotation matrix to identity", funcName) );

	setRotation( Matrix3x3(1) );
	return 0;
}

int GameObject::script_getDistanceTo( script::VM* vm, const char* funcName )
{
	GameObject* obj = 0;
	Vector3 pos( 0.f, 0.f, 0.f );
	getGameObjectOrVector3( vm, funcName, &obj, &pos );

	if ( obj )
		pos = obj->position();

	float distance = (pos - position()).length();
	vm->pushNumber( distance );
	return 1;
}

int	GameObject::script_getGameObject( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects string", funcName) );

	GameObject* obj = 0;
	GameObjectListItem* cellobj = cell()->objectsInCell();

	String objname = vm->toString(1);

	while ( cellobj && !obj )
	{
		if ( cellobj->object()->name() == objname )
			obj = cellobj->object();

		cellobj = cellobj->next();
	}

	if ( obj )
		vm->pushTable( obj );
	else 
		vm->pushNil();

	return 1;
}

int GameObject::getGameObjectOrVector3( script::VM* vm, const char* funcName,
	GameObject** obj, Vector3* pos, int param )
{
	if ( !(vm->top() >= param && vm->isTable(param)) &&
		!(vm->top() >= param+2 && vm->isNumber(param) && vm->isNumber(param+1) && vm->isNumber(param+2)) )
		throw ScriptException( Format("{0} expects game object or (x,y,z) position as {1}.parameter", funcName, param) );

	if ( vm->isTable(param) )
	{
		*obj = dynamic_cast<GameObject*>( getThisPtr(vm,param) );
		if ( !*obj )
			throw ScriptException( Format("{0} expects (C++) GameObject, not regular table", funcName) );
		*pos = Vector3(0,0,0);
		return param+1;
	}
	else
	{
		pos->x = vm->toNumber(param);
		pos->y = vm->toNumber(param+1);
		pos->z = vm->toNumber(param+2);
		*obj = 0;
		return param+3;
	}
}

int GameObject::script_getUp( script::VM* vm, const char* funcName ) 
{
	int index = getIndex( vm, funcName, 0, 3 );
	vm->pushNumber( up()[index] );
	return 1;	
}

int GameObject::script_getRight( script::VM* vm, const char* funcName ) 
{
	int index = getIndex( vm, funcName, 0, 3 );
	vm->pushNumber( right()[index] );
	return 1;	
}

int GameObject::script_getForward( script::VM* vm, const char* funcName )
{
	int index = getIndex( vm, funcName, 0, 3 );
	vm->pushNumber( forward()[index] );
	return 1;	
}

int GameObject::script_rotateY( script::VM* vm, const char* funcName )
{
	float ang = 0.f;
	getParam( vm, funcName, "angle", &ang );
	Matrix3x3 rot( Vector3(0,1,0), Math::toRadians( ang ) );
	m_ms.rot = m_ms.rot * rot;
	return 0;
}

int GameObject::script_getAngleTo( script::VM* vm, const char* funcName )
{
	// target obj/pos
	GameObject* obj = 0;
	Vector3 pos;
	getGameObjectOrVector3( vm, funcName, &obj, &pos );
	if ( obj )
		pos = obj->position();

	float angle = Math::toDegrees( getAngleTo(pos) );
	vm->pushNumber( angle );
	return 1;
}

int	GameObject::script_getSignedAngleTo( script::VM* vm, const char* funcName )
{
	// target obj/pos
	GameObject* obj = 0;
	Vector3 pos;
	getGameObjectOrVector3( vm, funcName, &obj, &pos );
	if ( obj )
		pos = obj->position();

	float angle = getSignedAngleTo( pos );
	vm->pushNumber( Math::toDegrees( angle ) );
	return 1;
}

int	GameObject::script_getSignedWorldAngleTo( script::VM* vm, const char* funcName )
{
	// target obj/pos
	GameObject* obj = 0;
	Vector3 pos;
	getGameObjectOrVector3( vm, funcName, &obj, &pos );
	if ( obj )
		pos = obj->position();

	float angle = getSignedWorldAngleTo( pos );
	vm->pushNumber( angle );
	return 1;
}

int	GameObject::script_speed( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns object speed in m/s", funcName) );
	vm->pushNumber( speed() );
	return 1;
}

int GameObject::script_isOnRightSide( script::VM* vm, const char* funcName ) 
{
	int tags1[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	int tags2[] = {VM::TYPE_TABLE};
	if ( !hasParams(tags1,sizeof(tags1)/sizeof(tags1[0])) &&
		 !hasParams(tags2,sizeof(tags2)/sizeof(tags2[0])) )
		throw ScriptException( Format("{0} returns true if object or position is on right side", funcName) );

	GameObject* obj = 0;
	Vector3 pos;
	getGameObjectOrVector3( vm, funcName, &obj, &pos );
	if ( obj )
		pos = obj->position();

	vm->pushBoolean( isOnRightSide( pos ) );
	return 1;
}

int GameObject::script_isInFront( script::VM* vm, const char* funcName ) 
{	
	int tags1[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	int tags2[] = {VM::TYPE_TABLE};
	if ( !hasParams(tags1,sizeof(tags1)/sizeof(tags1[0])) &&
		 !hasParams(tags2,sizeof(tags2)/sizeof(tags2[0])) )
		throw ScriptException( Format("{0} returns true if object or position is in front", funcName) );

	GameObject* obj = 0;
	Vector3 pos;
	getGameObjectOrVector3( vm, funcName, &obj, &pos );
	if ( obj )
		pos = obj->position();
	
	vm->pushBoolean( isInFront( pos ) );
	return 1;
}

int GameObject::script_projectPositionOnForward( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} returns length of (x,y,z) projected on forward vector", funcName) );
	
	Vector3 point;
	point.x = vm->toNumber(1);
	point.y = vm->toNumber(2);
	point.z = vm->toNumber(3);

	Vector3 topoint = point - position();

	vm->pushNumber( forward().dot( topoint ) );
	return 1;
}
	
int GameObject::script_projectPositionOnRight( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} returns length of (x,y,z) projected on right vector", funcName) );
	
	Vector3 point;
	point.x = vm->toNumber(1);
	point.y = vm->toNumber(2);
	point.z = vm->toNumber(3);

	Vector3 topoint = point - position();

	vm->pushNumber( right().dot( topoint ) );
	return 1;
}

int GameObject::script_setPosition( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_TABLE, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects cell object table & (x,y,z) position", funcName) );

	GameCell* cell = dynamic_cast<GameCell*>(getThisPtr(vm, 1));
	if ( !cell )
		throw ScriptException( Format("First parameter passed to {0} must be a GameCell object table.", funcName) );

	setPosition( cell, Vector3( vm->toNumber(2), vm->toNumber(3), vm->toNumber(4) ) );
	
	vm->pushTable( this );
	return 1;
}

int	GameObject::script_cell( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns the cell where the object is", funcName) );

	vm->pushTable( m_cell );
	return 1;
}

void GameObject::receiveExplosion( GameObject* source, float damage )
{
	if ( hasMethod("signalReceiveExplosion") )
	{
		Vector3 delta = position() - source->position();
		Vector3 direction = delta.normalize();

		pushMethod( "signalReceiveExplosion" );
		vm()->pushNumber( damage );
		vm()->pushNumber( direction.x );
		vm()->pushNumber( direction.y );
		vm()->pushNumber( direction.z );
		vm()->pushNumber( delta.length() );
		call( 5, 0 );
	}
}

int GameObject::script_hide( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} hides object.", funcName) );
	m_hidden = true;
	//Debug::println( "hide( {0} )", name() );
	return 0;
}

int GameObject::script_hidden( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns true if object is hidden.", funcName) );
	vm->pushBoolean( m_hidden );
	return 1;
}

int GameObject::script_unhide( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} unhides object.", funcName) );
	m_hidden = false;
	//Debug::println( "unhide( {0} )", name() );
	return 0;
}

int GameObject::script_setDynamicShadow( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects shadow object name and shadow length", funcName) );

	if ( m_shadowMesh )
		m_shadowMesh->unlink();
	m_shadowMesh = new Mesh;
	m_shadowName = vm->toString(1);
	m_shadowLength = vm->toNumber(2);
	m_shadowDirty = true;
	return 0;
}

int GameObject::script_enableDynamicShadow( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} enables dynamic shadow", funcName) );

	if ( !m_shadowMesh )
		throw ScriptException( Format("{0}: Shadow must be set with setDynamicShadow before it can be enabled/disabled", funcName) );

	m_shadowMesh->setEnabled( true );
	return 0;
}

int GameObject::script_disableDynamicShadow( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} disables dynamic shadow", funcName) );

	if ( !m_shadowMesh )
		throw ScriptException( Format("{0}: Shadow must be set with setDynamicShadow before it can be enabled/disabled", funcName) );

	m_shadowMesh->setEnabled( false );
	return 0;
}

