#include "GameWeapon.h"
#include "GameCell.h"
#include "GameCharacter.h"
#include "GameProjectile.h"
#include "GameRenderPass.h"
#include "GameLevel.h"
#include "ProjectileManager.h"
#include "GamePointObject.h"
#include "ScriptUtil.h"
#include <io/InputStreamArchive.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Debug.h>
#include <lang/String.h>
#include <lang/Character.h>
#include <math/Vector3.h>
#include <math/Matrix4x4.h>
#include <pix/Colorf.h>
#include <sg/Camera.h>
#include <sg/Light.h>
#include <sg/LOD.h>
#include <sg/Light.h>
#include <sg/Mesh.h>
#include <sg/Node.h>
#include <sg/Primitive.h>
#include <sgu/MeshUtil.h>
#include <sgu/NodeUtil.h>
#include <sgu/SceneManager.h>
#include <util/Vector.h>
#include <script/VM.h>
#include <script/ClassTag.h>
#include <script/ScriptException.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace sgu;
using namespace pix;
using namespace lang;
using namespace math;
using namespace util;
using namespace script;

//-----------------------------------------------------------------------------

ScriptMethod<GameWeapon> GameWeapon::sm_methods[] =
{
	ScriptMethod<GameWeapon>( "owner", script_owner ),
	ScriptMethod<GameWeapon>( "getShellsPerClip", script_getShellsPerClip ),
	ScriptMethod<GameWeapon>( "fireAt", script_fireAt ),
	ScriptMethod<GameWeapon>( "fireWithoutBullet", script_fireWithoutBullet ),
 	ScriptMethod<GameWeapon>( "getShellsRemaining", script_getShellsRemaining ),
	ScriptMethod<GameWeapon>( "reload", script_reload ),
	ScriptMethod<GameWeapon>( "setMesh", script_setMesh ),
	ScriptMethod<GameWeapon>( "setBullet", script_setBullet ),
	ScriptMethod<GameWeapon>( "setEmptyShell", script_setEmptyShell ),
	ScriptMethod<GameWeapon>( "setFireRate", script_setFireRate ),
	ScriptMethod<GameWeapon>( "setFireMode", script_setFireMode ),
	ScriptMethod<GameWeapon>( "setShellEjectDelay", script_setShellEjectDelay ),
	ScriptMethod<GameWeapon>( "setShellsPerClip", script_setShellsPerClip ),
	ScriptMethod<GameWeapon>( "setShellsRemaining", script_setShellsRemaining ),
	ScriptMethod<GameWeapon>( "setShotsPerLaunch", script_setShotsPerLaunch ),
	ScriptMethod<GameWeapon>( "setSpreadConeAngle", script_setSpreadConeAngle ),
	ScriptMethod<GameWeapon>( "setRecoilErrorPerShot", script_setRecoilErrorPerShot ),
	ScriptMethod<GameWeapon>( "setRecoilErrorCorrectionPerSec", script_setRecoilErrorCorrectionPerSec ),
	ScriptMethod<GameWeapon>( "setRecoilErrorMax", script_setRecoilErrorMax ),
	ScriptMethod<GameWeapon>( "setRecoilErrorMin", script_setRecoilErrorMin ),
};

//-----------------------------------------------------------------------------

GameWeapon::GameWeapon( script::VM* vm, io::InputStreamArchive* arch, snd::SoundManager* soundMgr, ps::ParticleSystemManager* particleMgr,
	sgu::SceneManager* sceneMgr, ProjectileManager* projectileMgr, GameNoiseManager* noiseMgr ) :
	GameObject( vm, arch, soundMgr, particleMgr, noiseMgr ),
	m_mesh( 0 ),
	m_sceneMgr( sceneMgr ),
	m_owner( 0 ),
	m_groundLightmapColor( 1, 1, 1 ),
	m_projectileMgr( projectileMgr ),
	m_bullet( "bullet.lua" ),
	m_emptyShell( "" ),
	m_fireRate( 10 ),
	m_fireDeltaTime( 0 ),
	m_shellEjectDelay( 0 ),
	m_shellEjectTime( 0 ),
	m_shellInChamber( false ),
	m_shells( 1 ),
	m_shellsPerClip( 1 ),
	m_shotsPerLaunch( 1 ),
	m_spreadConeLimitAngle( 0 ),
	m_clipEmpty( true ),
	m_fireMode( FIRE_AUTO ),
	m_accumulatedRecoilError( 0 ),
	m_recoilErrorPerShot( 0 ),
	m_recoilErrorCorrection( 1 ),
	m_maxRecoilError( 0 ),
	m_minRecoilError( 0 ),
	m_methodBase( -1 )
{
	m_methodBase = ScriptUtil<GameWeapon,GameObject>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

GameWeapon::~GameWeapon()
{
	assert( !m_owner );

	if ( m_projectileMgr )
		m_projectileMgr->removeWeaponProjectiles( this );
}

void GameWeapon::setShaderParams( Shader* fx, Mesh* mesh, Light* keylight )
{
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

void GameWeapon::updateShaderParameters()
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
					setShaderParams( fx, mesh, keylight );
			}
		}
	}
}

Node* GameWeapon::getRenderObject( Camera* camera )
{
	assert( m_mesh );
	assert( !m_mesh->parent() );

	//Debug::println( "getRenderObject({0}, owner={1})", name(), m_owner?m_owner->name():"none" );

	GameObject::getRenderObject( camera );

	if ( m_mesh && camera )
	{
		// owner visible?
		if ( m_owner && !m_owner->visible() )
			return 0;

		// set object properties by owner
		if ( !m_owner )
		{
			m_mesh->setPosition( position() );
			m_mesh->setRotation( rotation() * CHARACTER_MESH_PRE_ROTATION );
		}
		else
		{
			m_groundLightmapColor = m_owner->groundLightmapColor();
		}

		updateShaderParameters();
	}

	return m_mesh;
}

void GameWeapon::setOwner( GameCharacter* character ) 
{
	m_owner = character;
}

void GameWeapon::setShellsRemaining( int val )
{
	assert( val >= 0 && val <= m_shellsPerClip );

	m_shells = val;
	m_clipEmpty = (val == 0);
}

void GameWeapon::setFireRate( float rate )
{
	assert( rate > 0.f );

	m_fireRate = rate;
}

void GameWeapon::update( float dt ) 
{
	if ( m_owner )
		setPosition( m_owner->cell(), m_owner->center() );

	// do not update weapons which are in inventory
	if ( !inCell() )
		return;

	GameObject::update( dt );

	m_fireDeltaTime += dt;

	// Correct recoil error
	m_accumulatedRecoilError -= m_recoilErrorCorrection * dt;
	if ( m_accumulatedRecoilError < m_minRecoilError )
		m_accumulatedRecoilError = m_minRecoilError;

	if ( m_shellInChamber )
	{
		m_shellEjectTime -= dt;
		if ( m_shellEjectTime <= 0.f )
		{
			ejectShell();
		}
	}
}

Vector3 GameWeapon::perturbVector( const Vector3& src, float limit, const Vector3& up, const Vector3& right )
{
	Vector3 dir( src );		
	
	Vector3 leftLimit( dir.rotate( up, random() * -limit ) );
	Vector3 rightLimit( dir.rotate( up, random() * limit ) );
	Vector3 upLimit( dir.rotate( right, random() * -limit ) );
	Vector3 downLimit( dir.rotate( right, random() * limit ) );

	// Create 4 random numbers, normalize sum to one, take a weighted average of limit vectors with them and assign it as direction
	float r1 = random();
	float r2 = random();
	float r3 = random();
	float r4 = random();
	float normalizer = 1.f / ( r1 + r2 + r3 + r4 );
	r1 *= normalizer;
	r2 *= normalizer;
	r3 *= normalizer;
	r4 *= normalizer;

	return leftLimit * r1 + rightLimit * r2 + upLimit * r3 + downLimit * r4;	
}

Vector3 GameWeapon::launchPosition() const
{
	Node* launchNode = NodeUtil::findNodeByName( m_mesh, "Dummy_flame" );
	if ( !launchNode )
		throw Exception( Format( "Weapon {0} does not have node Dummy_flame", m_mesh->name()) );
	Vector3 launchPos = launchNode->worldTransform().translation();
	return launchPos;
}

void GameWeapon::fireVisuals( GameCharacter* character )
{
	// Call script function for firing effects
	pushMethod( "signalFire" );
	vm()->pushTable( character );
	call(1, 0);

	// Prepare for shell eject
	m_shellInChamber = true;
	m_shellEjectTime = m_shellEjectDelay;
}

void GameWeapon::fire( GameCharacter* character, const Vector3& direction, bool applyRecoilError )
{
	assert( m_mesh );

	if ( m_shells > 0 )
	{
		// find bullet launch position
		Vector3 launchPos = launchPosition();

		// check that we are not too close any obstacle
		GamePointObject* pointCollisionChecker = character->visibilityCollisionChecker();
		pointCollisionChecker->setPosition( character->cell(), character->position() );
		Vector3 startpoint = character->headWorldPosition();
		pointCollisionChecker->moveWithoutColliding( startpoint - pointCollisionChecker->position() );
		CollisionInfo cinfo;
		pointCollisionChecker->move( launchPos - pointCollisionChecker->position(), &cinfo );
		bool removeInFirstUpdate = cinfo.isCollision(CollisionInfo::COLLIDE_ALL);

		fireVisuals( character );

		// Apply recoil error
		Vector3 launchVector = direction;
		if ( applyRecoilError )
			launchVector = Vector3( perturbVector( direction, m_accumulatedRecoilError, character->up(), character->right() ) );

		// Create projectile(s)
		float limitangle = m_spreadConeLimitAngle;
		for ( int i = 0; i < m_shotsPerLaunch; ++i )
		{
			GameProjectile* projectile = m_projectileMgr->createProjectile( m_bullet, cell(), this, launchPos, perturbVector( launchVector, limitangle, character->up(), character->right() ) );
			if ( removeInFirstUpdate )
				projectile->removeInNextUpdate();
		}

		// Accumulate recoil error
		m_accumulatedRecoilError += m_recoilErrorPerShot;

		if ( m_accumulatedRecoilError > m_maxRecoilError )
			m_accumulatedRecoilError = m_maxRecoilError;

		// Decrease shells
		m_shells--;
	}
	else
	{
		// Call script function for empty clip
		pushMethod( "signalEmpty" );
		vm()->pushTable( character );
		call(1, 0);	

		m_clipEmpty = true;
	}

	// reset timer for weapon becoming ready for next shot
	m_fireDeltaTime = 0.f;

}

void GameWeapon::ejectShell()
{
	m_shellInChamber = false;
	
	if ( m_mesh && m_emptyShell != "" && cell() ) 
	{
		Node* ejectNode = NodeUtil::findNodeByName( m_mesh, "Dummy_shell_exit" );
		if ( !ejectNode )
			throw Exception( Format( "Weapon {0} scene does not contain node Dummy_shell_exit", m_mesh->name()) );

		Vector3 rgt = ejectNode->worldTransform().rotation().getColumn(0);
		Vector3 _up = ejectNode->worldTransform().rotation().getColumn(1);
		Vector3 fwd = ejectNode->worldTransform().rotation().getColumn(2);

		GameProjectile* shell = m_projectileMgr->createProjectile( m_emptyShell, cell(), this, ejectNode->worldTransform().translation(), perturbVector( -fwd, Math::PI / 2.5f, _up, rgt )  );
		shell->setGroundLightmapColor( m_groundLightmapColor );
	}
}

void GameWeapon::looseClip() 
{
	pushMethod( "signalLooseClip" );
	call(0, 0);

	m_shells = 0;
}

void GameWeapon::reload() 
{
	m_shells = m_shellsPerClip;
	m_clipEmpty = false;
}

sg::Light* GameWeapon::keylight() const
{
	if ( m_owner )
		return m_owner->keylight();
	else
		return GameObject::keylight();
}

GameCharacter*	GameWeapon::owner() const 
{
	return m_owner;
}

sg::Node* GameWeapon::mesh() const 
{
	return m_mesh;
}

bool GameWeapon::ready() const 
{
	return m_fireDeltaTime >= 1.f / m_fireRate;
}

const lang::String&	GameWeapon::bullet() const 
{
	return m_bullet;
}

float GameWeapon::fireRate() const 
{
	return m_fireRate;
}

int GameWeapon::shellsPerClip() const 
{
	return m_shellsPerClip;
}

int GameWeapon::shellsRemaining() const 
{
	return m_shells;
}

int GameWeapon::shotsPerLaunch() const 
{
	return m_shotsPerLaunch;
}

float GameWeapon::spreadConeLimitAngle() const 
{
	return m_spreadConeLimitAngle;
}

bool GameWeapon::clipEmpty() const 
{
	return m_clipEmpty;
}

float GameWeapon::accumulatedRecoilError() const 
{
	return m_accumulatedRecoilError;
}

float GameWeapon::recoilErrorPerShot() const 
{
	return m_recoilErrorPerShot;
}

float GameWeapon::recoilErrorCorrection() const 
{
	return m_recoilErrorCorrection;
}

float GameWeapon::maxRecoilError() const 
{
	return m_maxRecoilError;
}

float GameWeapon::minRecoilError() const 
{
	return m_minRecoilError;
}

GameWeapon::FireMode GameWeapon::fireMode() const
{
	return m_fireMode;
}

int GameWeapon::methodCall( script::VM* vm, int i ) 
{
	return ScriptUtil<GameWeapon, GameObject>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );	
}

int GameWeapon::script_reload( VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} fills weapon ammo", funcName) );
	reload();
	return 0;
}

int GameWeapon::script_owner( VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns owner character of the weapon", funcName) );

	vm->pushTable( m_owner );
	return 1;
}

int GameWeapon::script_setMesh( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects a mesh file name", funcName) );

	int		param = 1;
	String	sceneName = vm->toString(param++);

	m_mesh = m_sceneMgr->getScene( sceneName, SceneFile::LOAD_GEOMETRY )->clone();
	setRenderPasses( m_mesh, GameRenderPass::RENDERPASS_WEAPON_SOLID, GameRenderPass::RENDERPASS_WEAPON_TRANSPARENT );
	removeLightsAndCameras( m_mesh );
	setRenderPasses( m_mesh, GameRenderPass::RENDERPASS_WEAPON_SOLID, GameRenderPass::RENDERPASS_WEAPON_TRANSPARENT );
	return 0;
}

int GameWeapon::script_setBullet( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects string", funcName) );

	m_bullet = vm->toString(1);
	return 0;
}

int GameWeapon::script_setFireRate( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects number", funcName) );

	setFireRate( vm->toNumber(1) );
	return 0;
}

int GameWeapon::script_setFireMode( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects AUTO or SINGLE", funcName) );

	String fireMode = vm->toString(1);
	if ( fireMode == "AUTO" )
		m_fireMode = FIRE_AUTO;
	else if ( fireMode == "SINGLE" )
		m_fireMode = FIRE_SINGLE;
	else
		throw ScriptException( Format("{2}: Invalid weapon {0} fire mode: {1}", name(), fireMode, funcName) );
	return 0;
}

int GameWeapon::script_setShellsPerClip( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects number", funcName) );

	m_shellsPerClip = (int)vm->toNumber(1);
	return 0;
}

int GameWeapon::script_setShellsRemaining( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects number", funcName) );

	setShellsRemaining( (int)vm->toNumber(1) );
	return 0;
}

int GameWeapon::script_setShotsPerLaunch( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects number", funcName) );

	m_shotsPerLaunch = (int)vm->toNumber(1);
	return 0;
}

int GameWeapon::script_setSpreadConeAngle( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects angle", funcName) );

	m_spreadConeLimitAngle = Math::toRadians(vm->toNumber(1)) / 2.f;
	return 0;
}

int GameWeapon::script_setRecoilErrorPerShot( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects angle", funcName) );

	m_recoilErrorPerShot = Math::toRadians(vm->toNumber(1));
	return 0;
}

int GameWeapon::script_setRecoilErrorCorrectionPerSec( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects degrees/s", funcName) );

	m_recoilErrorCorrection = Math::toRadians(vm->toNumber(1));
	return 0;
}

int GameWeapon::script_setRecoilErrorMax( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects angle", funcName) );

	m_maxRecoilError = Math::toRadians(vm->toNumber(1));
	return 0;
}

int GameWeapon::script_setRecoilErrorMin( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects angle", funcName) );

	m_minRecoilError = Math::toRadians(vm->toNumber(1));
	return 0;
}

int GameWeapon::script_setEmptyShell( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects string", funcName) );

	m_emptyShell = vm->toString(1);
	return 0;
}

int GameWeapon::script_setShellEjectDelay( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects time in seconds", funcName) );

	m_shellEjectDelay = vm->toNumber(1);
	return 0;
}

int GameWeapon::script_getShellsRemaining( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns count of shells remaining", funcName ) );

	vm->pushNumber( (float)m_shells );
	return 1;
}

int GameWeapon::script_getShellsPerClip( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns number of shells per clip", funcName ) );

	vm->pushNumber( (float)m_shellsPerClip );
	return 1;
}

int GameWeapon::script_fireAt( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_TABLE, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects shooter character and x, y, z shooting target.", funcName) );
	
	GameCharacter*	character	= dynamic_cast<GameCharacter*>( getThisPtr(vm,1) );
	float			posx		= vm->toNumber( 2 );
	float			posy		= vm->toNumber( 3 );
	float			posz		= vm->toNumber( 4 );
	Vector3			dir			= Vector3(posx,posy,posz) - launchPosition();

	if ( !character )
		throw ScriptException( Format("{0} expects shooter character and x, y, z shooting direction. First parameter was not a character.", funcName) );

	if ( dir.length() > Float::MIN_VALUE )
		dir = dir.normalize();

	fire( character, dir, false );
	return 0;
}

int GameWeapon::script_fireWithoutBullet( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_TABLE};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects shooter character and x, y, z shooting direction.", funcName) );
	
	GameCharacter*	character	= dynamic_cast<GameCharacter*>( getThisPtr(vm,1) );

	if ( !character )
		throw ScriptException( Format("{0} expects shooter character.", funcName) );

	fireVisuals( character );
	return 0;
}
