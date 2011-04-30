#include "GameProjectile.h"
#include "ProjectileManager.h"
#include "GameWeapon.h"
#include "GameBSPTree.h"
#include "GameRenderPass.h"
#include "GameDynamicObject.h"
#include "GameCharacter.h"
#include "GameCell.h"
#include "GameLevel.h"
#include "GameBoxTrigger.h"
#include "CollisionInfo.h"
#include "ScriptUtil.h"
#include <sg/Camera.h>
#include <sg/Light.h>
#include <sg/LOD.h>
#include <sg/Light.h>
#include <sg/Mesh.h>
#include <sg/Node.h>
#include <sg/Primitive.h>
#include <lang/Math.h>
#include <io/InputStreamArchive.h>
#include <ps/ParticleSystemManager.h>
#include <bsp/BSPCollisionUtil.h>
#include <bsp/BSPPolygon.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include <lang/String.h>
#include <lang/Character.h>
#include <math/Vector3.h>
#include <math/Matrix3x3.h>
#include <math/Intersection.h>
#include <script/VM.h>
#include <script/ScriptException.h>
#include <sgu/MeshUtil.h>
#include <sgu/SceneManager.h>
#include <snd/SoundManager.h>

//-----------------------------------------------------------------------------

using namespace sg;
using namespace bsp;
using namespace pix;
using namespace sgu;
using namespace lang;
using namespace util;
using namespace math;
using namespace script;

//-----------------------------------------------------------------------------

ScriptMethod<GameProjectile> GameProjectile::sm_methods[] =
{
	ScriptMethod<GameProjectile>( "damage", script_damage ),
	ScriptMethod<GameProjectile>( "enableKeepOnCollision", script_enableKeepOnCollision ),
	ScriptMethod<GameProjectile>( "enableAlignOnCollision", script_enableAlignOnCollision ),
	ScriptMethod<GameProjectile>( "enableHitCharacter", script_enableHitCharacter ),
	ScriptMethod<GameProjectile>( "getWeapon", script_getWeapon ),
	ScriptMethod<GameProjectile>( "setAgeLimit", script_setAgeLimit ),
	ScriptMethod<GameProjectile>( "setDamage", script_setDamage ),
	ScriptMethod<GameProjectile>( "setGravity", script_setGravity ),
	ScriptMethod<GameProjectile>( "setLaunchVelocity", script_setLaunchVelocity ),
	ScriptMethod<GameProjectile>( "setMesh", script_setMesh ),
	ScriptMethod<GameProjectile>( "setDamageAttenuationStartRange", script_setDamageAttenuationStartRange ),
	ScriptMethod<GameProjectile>( "setMaxRange", script_setMaxRange ),
	ScriptMethod<GameProjectile>( "setSpin", script_setSpin ),
};

//-----------------------------------------------------------------------------

GameProjectile::GameProjectile( ProjectileManager* projectileMgr, SceneManager* sceneMgr, script::VM* vm, 
							   io::InputStreamArchive* arch, snd::SoundManager* soundMgr, 
							   ps::ParticleSystemManager* particleMgr, GameNoiseManager* noiseMgr ) :
	GameObject( vm, arch, soundMgr, particleMgr, noiseMgr ),
	m_projectileMgr( projectileMgr ),
	m_sceneMgr( sceneMgr ),
	m_mesh( 0 ),
	m_weapon( 0 ),
	m_spinAxis( Vector3(0,0,0) ),
	m_spinSpeed( 0 ),
	m_removable( false ),
	m_launchVelocity( 0 ), 
	m_age( 0 ),
	m_ageLimit( 0 ),
	m_damage( 0 ),
	m_gravity( 0 ),
	m_damageAttenuationRange( 1e6f ),
	m_maxRange( 1e7f ),
	m_movedDistance( 0 ),
	m_keepOnCollision( false ),
	m_alignOnCollision( false ),
	m_hitCharacter( true ),
	m_hasHit( false ),
	m_lastBoneHit()
{
	m_methodBase = ScriptUtil<GameProjectile,GameObject>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

void GameProjectile::checkCollisionsAgainstCell( const Vector3& start, const Vector3& delta, bsp::BSPNode* bsptree, GameCell* collisionCell, CollisionInfo* cinfo )
{
	float t = 1.f;
	const BSPPolygon* cpoly = 0;
	BSPCollisionUtil::findLineIntersection( bsptree, start, delta, CollisionInfo::COLLIDE_GEOMETRY_SOLID+CollisionInfo::COLLIDE_OBJECT, &t, &cpoly );

	if ( t < 1.f )
	{
		Vector3 end = start + delta * t;
		Vector4 pl = cpoly->plane();
		Vector3 cnormal = Vector3(pl.x,pl.y,pl.z);
		*cinfo = CollisionInfo( end, cinfo->positionCell(), collisionCell, cpoly, cnormal, end, 0, collisionCell->bspTree() );
	}
}

void GameProjectile::checkCollisionsAgainstObjects( const Vector3& start, const Vector3& delta, const Vector<GameObjectDistance>& objects, CollisionInfo* cinfo )
{
	for ( int i = 0 ; i < objects.size() ; ++i )
	{
		GameObject* obj = objects[i].object;

		// character ( don't check collision against character which holds weapon this projectile was shot with )
		GameCharacter* character = dynamic_cast<GameCharacter*>( obj );
		if ( character && m_hitCharacter && m_weapon != character->weapon() )
		{
			if ( character->rootCollisionBox().findLineBoxIntersection(start,delta,0) )
			{
				for ( int k = 0 ; k < character->boneCollisionBoxes() ; ++k )
				{
					const BoneCollisionBox& box = character->getBoneCollisionBox(k);

					float t;
					if ( box.findLineBoxIntersection(start,delta,&t) )
					{
						Vector3 end = start + delta*t;
						Vector3 cpoint = end;
						*cinfo = CollisionInfo( end, obj->cell(), obj->cell(), 0, obj->forward(), cpoint, obj, 0 );
						m_lastBoneHit = box.bone()->name();
						return;
					}
				}
			}
			continue;
		}

		// bullet trigger
		GameBoxTrigger* trigger = dynamic_cast<GameBoxTrigger*>( obj );
		if ( trigger && m_hitCharacter )
		{
			if ( Intersection::findLineBoxIntersection(start,delta,trigger->transform(),trigger->dimensions(),0) )
			{
				if ( trigger->addAffectedObject(this) && trigger->hasMethod("signalProjectileCollision") )
				{
					trigger->pushMethod( "signalProjectileCollision" );
					vm()->pushTable( this );
					trigger->call( 1, 0 );
				}
			}
			continue;
		}

		// dynamic object
		GameDynamicObject* dynamicObject = dynamic_cast<GameDynamicObject*>( obj );
		if ( dynamicObject )
		{
			if ( !dynamicObject->hidden() && m_hitCharacter &&
				Intersection::findLineSphereIntersection(start, delta, dynamicObject->position(), dynamicObject->boundSphere(), 0) )
			{
				Matrix4x4 tm = dynamicObject->transform();
				Matrix4x4 invtm = tm.inverse();
				Vector3 bspStart = invtm.transform( start );
				Vector3 bspDelta = invtm.rotate( delta );

				float t = 1.f;
				const BSPPolygon* cpoly = 0;
				GameBSPTree* bsptree = dynamicObject->bspTree();
				BSPCollisionUtil::findLineIntersection( bsptree->root(), bspStart, bspDelta, CollisionInfo::COLLIDE_GEOMETRY_SOLID, &t, &cpoly );

				if ( t < 1.f )
				{
					Vector3 end = start + delta * t;
					Vector4 pl = cpoly->plane();
					Vector3 cnormal = tm.rotate( Vector3(pl.x,pl.y,pl.z) );
					*cinfo = CollisionInfo( end, dynamicObject->cell(), dynamicObject->cell(), cpoly, cnormal, end, dynamicObject, bsptree );

					if ( dynamicObject->hasMethod("signalProjectileCollision") )
					{
						dynamicObject->pushMethod( "signalProjectileCollision" );
						vm()->pushTable( this );
						dynamicObject->call( 1, 0 );
					}
				}
			}
			continue;
		}
	}
}

void GameProjectile::setShaderParams( Primitive* /*prim*/, Shader* fx, Mesh* mesh, Camera* camera, Light* keylight )
{
	Vector3 cameraWPos = camera->worldTransform().translation();
	Matrix4x4 worldTm = mesh->cachedWorldTransform();
	Matrix4x4 worldTmInv = worldTm.inverse();

	for ( int i = 0 ; i < fx->parameters() ; ++i )
	{
		Shader::ParameterDesc desc;
		fx->getParameterDesc( i, &desc );
		
		if ( desc.name == "GROUND_COLOR" )
		{
			Vector4 groundColorMin4, groundColorMax4;
			fx->getVector4( "GROUND_COLOR_MIN", &groundColorMin4 );
			fx->getVector4( "GROUND_COLOR_MAX", &groundColorMax4 );
			Colorf groundColorMin( groundColorMin4.x, groundColorMin4.y, groundColorMin4.z, 1.f );
			Colorf groundColorMax( groundColorMax4.x, groundColorMax4.y, groundColorMax4.z, 1.f );
			Colorf c = ( (groundColorMax-groundColorMin) * m_groundLightmapColor + groundColorMin ).saturate();
			fx->setVector4( desc.name, Vector4(c.red(),c.green(),c.blue(),1.f) );
		}
		else if ( desc.name == "SKY_COLOR" )
		{
			Vector4 skyColorMin4, skyColorMax4;
			fx->getVector4( "SKY_COLOR_MIN", &skyColorMin4 );
			fx->getVector4( "SKY_COLOR_MAX", &skyColorMax4 );
			Colorf skyColorMin( skyColorMin4.x, skyColorMin4.y, skyColorMin4.z, 1.f );
			Colorf skyColorMax( skyColorMax4.x, skyColorMax4.y, skyColorMax4.z, 1.f );
			Colorf c = ( (skyColorMax-skyColorMin) * m_groundLightmapColor + skyColorMin ).saturate();
			fx->setVector4( desc.name, Vector4(c.red(),c.green(),c.blue(),1.f) );
		}
		else if ( desc.name == "SPECULAR_COLOR" )
		{
			Vector4 specularColorMin4, specularColorMax4;
			fx->getVector4( "SPECULAR_COLOR_MIN", &specularColorMin4 );
			fx->getVector4( "SPECULAR_COLOR_MAX", &specularColorMax4 );
			Colorf specularColorMin( specularColorMin4.x, specularColorMin4.y, specularColorMin4.z, 1.f );
			Colorf specularColorMax( specularColorMax4.x, specularColorMax4.y, specularColorMax4.z, 1.f );
			Colorf c = ( (specularColorMax-specularColorMin) * m_groundLightmapColor + specularColorMin ).saturate();
			fx->setVector4( desc.name, Vector4(c.red(),c.green(),c.blue(),1.f) );
		}
		else if ( desc.dataType == Shader::PT_FLOAT && desc.dataClass == Shader::PC_VECTOR4 && 
			desc.name.length() > 0 && Character::isLowerCase(desc.name.charAt(0)) &&
			desc.name.indexOf("Camera") == -1 )
		{
			bool paramResolved = false;
			bool objSpace = ( desc.name.length() > 2 && desc.name.charAt(1) == 'o' );
			int baseNameOffset = objSpace ? 2 : 1;

			Node* node = 0;
			String name = desc.name.substring( baseNameOffset );
			if ( keylight && name == "Light1" )
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

void GameProjectile::updateShaderParameters( Camera* camera )
{
	m_mesh->validateHierarchy();

	// set shader parameters
	Light* keylight = this->keylight();
	for ( Node* node = m_mesh ; node ; node = node->nextInHierarchy() )
	{
		Mesh* mesh = dynamic_cast<Mesh*>( node );
		if ( mesh )
		{
			for ( int i = 0 ; i < mesh->primitives() ; ++i )
			{
				Primitive* prim = mesh->getPrimitive(i);
				Shader* fx = prim->shader();
				if ( fx && fx->parameters() > 0 )
					setShaderParams( prim, fx, mesh, camera, keylight );
			}
		}
	}
}

Node* GameProjectile::getRenderObject( Camera* camera )
{
	GameObject::getRenderObject( camera );

	if ( m_mesh )
	{
		assert( !m_mesh->parent() );

		if ( m_mesh && camera )
		{
			m_mesh->setPosition( position() );
			m_mesh->setRotation( rotation() * CHARACTER_MESH_PRE_ROTATION );

			updateShaderParameters( camera );
		}
	}

	return m_mesh;
}

void GameProjectile::update( float dt ) 
{
	GameObject::update( dt );

	if ( !m_hasHit )
	{
		Vector3 vel = velocity();
		vel += Vector3( 0, -9.81f, 0 ) * dt * m_gravity;
		setVelocity( vel );

		m_movedDistance += velocity().length() * dt;

		CollisionInfo cinfo;
		move( velocity() * dt, &cinfo );
		if ( cinfo.isCollision(CollisionInfo::COLLIDE_ALL) )
			hit( &cinfo );

		if ( m_spinSpeed > 0.f )
			setRotation( rotation() * Matrix3x3( m_spinAxis, m_spinSpeed * dt) );
	}

	m_age += dt;
	if ( m_age > m_ageLimit )
		m_removable = true;

	if ( m_movedDistance > m_maxRange )
		m_removable = true;
}

void GameProjectile::setGroundLightmapColor( const pix::Colorf& color ) 
{
	m_groundLightmapColor = color;
}

void GameProjectile::setWeapon( GameWeapon* weapon ) 
{
	m_weapon = weapon;
}

void GameProjectile::launch( const math::Vector3& direction )
{
	setVelocity( direction.normalize() * m_launchVelocity );
	m_removable = false;
	m_hasHit = false;
	m_age = 0;
	m_movedDistance = 0;
}

void GameProjectile::hit( CollisionInfo* cinfo )
{
	if ( cinfo->isCollision(CollisionInfo::COLLIDE_GEOMETRY_SOLID) || cinfo->isCollision(CollisionInfo::COLLIDE_GEOMETRY_SEETHROUGH) )
	{
		hitGeometry( cinfo ); 
	}
	else if ( cinfo->isCollision(CollisionInfo::COLLIDE_OBJECT) )
	{
		if ( dynamic_cast<GameCharacter*>(cinfo->object()) )
			hitCharacter( cinfo, static_cast<GameCharacter*>(cinfo->object()) ); 
	}
}

void GameProjectile::hitGeometry( CollisionInfo* cinfo ) 
{
	// Do damage
	pushMethod( "signalHitGeometry" );
	vm()->pushNumber( cinfo->point().x );
	vm()->pushNumber( cinfo->point().y );
	vm()->pushNumber( cinfo->point().z );
	vm()->pushTable( getCollisionMaterial(*cinfo) );
	call(4,0);

	// Set removal flag
	if ( !m_keepOnCollision )
	{
		m_removable = true;
	}
	else
	{
		m_hasHit = true;
	}
	// Align rotation to match ground normal 
	if ( m_alignOnCollision )
	{
		alignRotation( cinfo->normal() );		
	}
}

void GameProjectile::hitCharacter( CollisionInfo* cinfo, GameCharacter* character ) 
{
	// Do damage
	character->receiveProjectile( this, cinfo->point(), character->getBoneCollisionBox(m_lastBoneHit) );
	
	// Set removal flag
	m_removable = true;
}

void GameProjectile::removeInNextUpdate()
{
	m_removable = true;
}

GameWeapon* GameProjectile::weapon() const
{
	return m_weapon;
}

bool GameProjectile::shouldBeRemoved() const 
{
	return m_removable;
}

sg::Light* GameProjectile::keylight() const
{
	if ( m_weapon )
		return m_weapon->keylight();
	else
		return GameObject::keylight();
}

float GameProjectile::damage() const 
{
	if ( m_movedDistance < m_damageAttenuationRange )
	{
		return m_damage;
	}
	else
	{
		float scale = 1.f - ((m_movedDistance - m_damageAttenuationRange) / ( m_maxRange - m_damageAttenuationRange ));
		return m_damage * scale;
	}
}

int GameProjectile::methodCall( script::VM* vm, int i ) 
{
	return ScriptUtil<GameProjectile,GameObject>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );	
}

int GameProjectile::script_setAgeLimit( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects age limit in seconds", funcName) );
	
	m_ageLimit = vm->toNumber(1);
	return 0;
}

int GameProjectile::script_setDamage( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects a number", funcName) );
	
	m_damage = vm->toNumber(1);
	return 0;
}

int GameProjectile::script_damage( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns amount of damage projectile will do", funcName) );
	
	vm->pushNumber( damage() );
	return 1;
}

int GameProjectile::script_setLaunchVelocity( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects velocity (m/s)", funcName) );

	m_launchVelocity = vm->toNumber(1);
	return 0;
}

int GameProjectile::script_setGravity( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects multiplier to gravity (9.81 m/s^2)", funcName) );

	m_gravity = vm->toNumber(1);
	return 0;
}

int GameProjectile::script_setMesh( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects a mesh file name", funcName) );

	int		param = 1;
	String	sceneName = vm->toString(param++);

	m_mesh = m_sceneMgr->getScene( sceneName, SceneFile::LOAD_GEOMETRY )->clone();

	MeshUtil::restoreBones( m_mesh );
	setRenderPasses( m_mesh, GameRenderPass::RENDERPASS_PROJECTILE_SOLID, GameRenderPass::RENDERPASS_PROJECTILE_TRANSPARENT );
	return 0;
}

int GameProjectile::script_enableKeepOnCollision( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 1 )
		throw ScriptException( Format("{0} enables not removing the projectile on collision", funcName) );

	m_keepOnCollision = !vm->isNil(1);
	return 0;
}

int GameProjectile::script_enableAlignOnCollision( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 1 )
		throw ScriptException( Format("{0} enables aligning the projectile on collision", funcName) );

	m_alignOnCollision = !vm->isNil(1);
	return 0;
}

int GameProjectile::script_enableHitCharacter( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 1 )
		throw ScriptException( Format("{0} enables hitting characters", funcName) );

	m_hitCharacter = !vm->isNil(1);
	return 0;
}

int GameProjectile::script_setSpin( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects spin axis (x,y,z) and angle speed in degrees", funcName) );

	m_spinAxis.x = vm->toNumber(1);
	m_spinAxis.y = vm->toNumber(2);
	m_spinAxis.z = vm->toNumber(3);
	
	m_spinAxis = m_spinAxis.normalize();

	m_spinSpeed = Math::toRadians( vm->toNumber(4) );
	return 0;
}

int GameProjectile::script_getWeapon( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns weapon that fired the projectile", funcName) );

	if ( !m_weapon )
		throw ScriptException( Format("FATAL ERROR in {0}, weapon is NULL", funcName) );

	vm->pushTable( m_weapon );
	return 1;
}

int GameProjectile::script_setMaxRange( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expect maximum range of projectile in meters", funcName) );

	m_maxRange = vm->toNumber(1);
	return 0;
}

int GameProjectile::script_setDamageAttenuationStartRange( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expect distance when damage attenuation starts", funcName) );

	m_damageAttenuationRange = vm->toNumber(1);
	return 0;
}

