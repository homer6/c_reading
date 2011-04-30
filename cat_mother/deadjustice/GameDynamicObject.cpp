#include "GameDynamicObject.h"
#include "GamePointObject.h"
#include "GameBSPTree.h"
#include "GameLevel.h"
#include "GameRenderPass.h"
#include "ScriptUtil.h"
#include <sg/Mesh.h>
#include <sg/Light.h>
#include <sg/Camera.h>
#include <dev/Profile.h>
#include <sgu/NodeUtil.h>
#include <sgu/SceneManager.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include <lang/Character.h>
#include <script/ScriptException.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace sgu;
using namespace lang;
using namespace util;
using namespace math;
using namespace anim;
using namespace script;

//-----------------------------------------------------------------------------

ScriptMethod<GameDynamicObject> GameDynamicObject::sm_methods[] =
{
	//ScriptMethod<GameDynamicObject>( "funcName", script_funcName ),
	ScriptMethod<GameDynamicObject>( "playWorldSpaceAnimation", script_playWorldSpaceAnimation ),
	ScriptMethod<GameDynamicObject>( "setSpin", script_setSpin ),
	ScriptMethod<GameDynamicObject>( "stopWorldSpaceAnimation", script_stopWorldSpaceAnimation ),
};

//-----------------------------------------------------------------------------

GameDynamicObject::GameDynamicObject( script::VM* vm, io::InputStreamArchive* arch, 
	sgu::SceneManager* sceneMgr, snd::SoundManager* soundMgr, 
	ps::ParticleSystemManager* particleMgr, ProjectileManager* projectileMgr, GameNoiseManager* noiseMgr,
	sg::Node* geometry,	const lang::String& bspFileName, int bspBuildPolySkip,
	const Vector<P(GameSurface)>& collisionMaterialTypes, bsp::BSPTree* cachedBSPTree ) :
	GameObject( vm, arch, soundMgr, particleMgr, noiseMgr ),
	m_sceneMgr( sceneMgr ),
	m_geometry( geometry ),
	m_projectileMgr( projectileMgr ),
	m_bsptree( 0 ),
	m_visibilityChecker( new GamePointObject(CollisionInfo::COLLIDE_GEOMETRY_SOLID) ),
	m_spinAxis( 0, 0, 0 ),
	m_spinSpeed( 0 ),
	m_worldAnim( 0 ),
	m_worldAnimTime( 0.f )
{
	m_methodBase = ScriptUtil<GameDynamicObject,GameObject>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
	setCollidable( true );

	// store BSP tree in game object space so reset root transform before computing BSP tree
	m_geometry->setTransform( Matrix4x4(1.f) );
	m_bsptree = new GameBSPTree( arch, m_geometry, bspFileName, bspBuildPolySkip, collisionMaterialTypes, cachedBSPTree );

	// get bound sphere from visual hierarchy or BSP
	float r = Math::max( m_bsptree->boundSphere(), NodeUtil::getHierarchyBoundSphere(m_geometry) );
	Debug::println( "Dynamic object {0} bound sphere is {1}", name(), r );
	setBoundSphere( r );

	setRenderPasses( m_geometry, GameRenderPass::RENDERPASS_DYNAMIC_OBJECT_SOLID, GameRenderPass::RENDERPASS_DYNAMIC_OBJECT_TRANSPARENT );
}

void GameDynamicObject::update( float dt )
{
	GameObject::update( dt );

	if ( m_spinSpeed > 0.f )
		setRotation( rotation() * Matrix3x3( m_spinAxis, m_spinSpeed * dt) );

	if ( m_worldAnim )
	{
		m_worldAnimTime += dt;

		//Animatable* anims[] = {m_worldAnim};
		//float weight = 1.f;
		//m_geometry->blendState( anims, &m_worldAnimTime, &weight, 1 );
		m_geometry->setTransform( m_worldAnim->transform() );

		Matrix4x4 tm = m_geometry->worldTransform();
		moveWithoutColliding( tm.translation() - position() );
		setRotation( tm.rotation() );
	}

	m_geometry->setTransform( transform() );
}

void GameDynamicObject::applyWorldSpaceAnimations()
{
	if ( m_worldAnim )
	{
		for ( Node* node = m_geometry ; node ; node = node->nextInHierarchy() )
		{
			Node* animNode = NodeUtil::findNodeByName( m_worldAnim, node->name() );
			if ( animNode )
			{
				//Animatable* anims[] = {animNode};
				//float weight = 1.f;
				//node->blendState( anims, &m_worldAnimTime, &weight, 1 );
				node->setTransform( animNode->transform() );
			}
		}
	}
}

bool GameDynamicObject::isVisibleFrom( const Vector3& worldPos ) const
{
	dev::Profile pr( "DO.isVisibleFrom" );

	float r = boundSphere();
	Vector3 p0 = position();
	Vector3 points[] = 
	{
		p0,
		p0 + right()*r,
		p0 + right()*-r,
		p0 + up()*r,
		p0 + up()*-r,
		p0 + forward()*r,
		p0 + forward()*-r
	};

	for ( int i = 0 ; i < (int)sizeof(points)/sizeof(points[0]) ; ++i )
	{
		if ( isVisiblePoint(worldPos, points[i]) )
			return true;
	}

	return false;
}

bool GameDynamicObject::isVisiblePoint( const Vector3& worldPos, const Vector3& point ) const
{
	// check if character is occluded by level geometry
	m_visibilityChecker->setPosition( cell(), position() );
	m_visibilityChecker->moveWithoutColliding( point-m_visibilityChecker->position() );
	CollisionInfo cinfo;
	m_visibilityChecker->move( worldPos-m_visibilityChecker->position(), &cinfo );
	return !cinfo.isCollision( CollisionInfo::COLLIDE_ALL );
}

Node* GameDynamicObject::getRenderObject( sg::Camera* camera ) 
{
	GameObject::getRenderObject( camera );

	if ( camera )
	{
		// view frustum test
		if ( !camera->isInView(position(), boundSphere()) )
			return 0;

		// occlusion test
		/*const Matrix4x4& camtm = camera->cachedWorldTransform();
		if ( !isVisibleFrom(camtm.translation()) )
			return 0;*/

		// apply animations
		if ( m_worldAnim )
			applyWorldSpaceAnimations();

		// set shader parameters
		Light* keylight = this->keylight();
		m_geometry->validateHierarchy();
		for ( Node* node = m_geometry ; node ; node = node->nextInHierarchy() )
		{
			Mesh* mesh = dynamic_cast<Mesh*>( node );
			if ( mesh )
			{
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

void GameDynamicObject::setShaderParams( Shader* fx, Mesh* mesh, Light* keylight )
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

GameBSPTree* GameDynamicObject::bspTree() const
{
	return m_bsptree;
}

int GameDynamicObject::methodCall( script::VM* vm, int i ) 
{
	return ScriptUtil<GameDynamicObject,GameObject>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

int GameDynamicObject::script_setSpin( script::VM* vm, const char* funcName ) 
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

int GameDynamicObject::script_playWorldSpaceAnimation( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_STRING, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),2) )
		throw ScriptException( Format("{0} expects animation file name (.sg included), optional end behaviour (REPEAT,CONSTANT,OSCILLATE,RESET) and start time", funcName) );

	int		argc			= vm->top();
	String	sceneName		= vm->toString(1);
	String	endBehaviour	= argc > 1 ? vm->toString(2) : "REPEAT";
	float	startTime		= argc > 2 ? vm->toNumber(3) : 0.f;
	P(Node)	root			= m_sceneMgr->getScene( sceneName, SceneFile::LOAD_ANIMATIONS );

	m_worldAnim = NodeUtil::findNodeByName( root, m_geometry->name() );
	if ( !m_worldAnim )
		throw ScriptException( Format("{0}: Scene {1} does not contain animation for dynamic object {2}", funcName, sceneName, m_geometry->name()) );
	NodeUtil::setHierarchyEndBehaviour( m_worldAnim, Interpolator::toBehaviour(endBehaviour) );

	m_worldAnimTime = startTime;
	applyWorldSpaceAnimations();
	return 0;
}

int GameDynamicObject::script_stopWorldSpaceAnimation( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} stops any active animation", funcName) );

	m_worldAnim = 0;
	m_worldAnimTime = 0.f;
	return 0;
}
