#include "GameCamera.h"
#include "GameLevel.h"
#include "GameCell.h"
#include "GameCutScene.h"
#include "GameCharacter.h"
#include "GameWeapon.h"
#include "ScriptUtil.h"
#include "CollisionInfo.h"
#include "GamePointObject.h"
#include <sg/Light.h>
#include <gd/GraphicsDevice.h>
#include <sg/Scene.h>
#include <sg/Camera.h>
#include <sg/Context.h>
#include <sg/Effect.h>
#include <sg/Material.h>
#include <sg/ViewFrustum.h>
#include <sgu/NodeUtil.h>
#include <sgu/SceneManager.h>
#include <bsp/BSPPolygon.h>
#include <bsp/BSPCollisionUtil.h>
#include <dev/Profile.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include <lang/Float.h>
#include <lang/Exception.h>
#include <math/Vector2.h>
#include <math/Quaternion.h>
#include <script/ScriptException.h>
#include <snd/SoundManager.h>
#include <ps/ParticleSystemManager.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace sgu;
using namespace dev;
using namespace bsp;
using namespace lang;
using namespace util;
using namespace math;
using namespace anim;
using namespace script;

//-----------------------------------------------------------------------------

static const float MIN_OCCLUDER_DISTANCE = 0.3f;

//-----------------------------------------------------------------------------

ScriptMethod<GameCamera> GameCamera::sm_methods[] =
{
	ScriptMethod<GameCamera>( "playAnimation", GameCamera::script_playAnimation ),
	ScriptMethod<GameCamera>( "stopAnimation", GameCamera::script_stopAnimation ),
	ScriptMethod<GameCamera>( "getAnimationStartTime", GameCamera::script_getAnimationStartTime ),
	ScriptMethod<GameCamera>( "getAnimationEndTime", GameCamera::script_getAnimationEndTime ),
	ScriptMethod<GameCamera>( "addTargetStateOffset", GameCamera::script_addTargetStateOffset ),
	ScriptMethod<GameCamera>( "setAverageCount", GameCamera::script_setAverageCount ),
	ScriptMethod<GameCamera>( "setBlendTime", GameCamera::script_setBlendTime ),
	ScriptMethod<GameCamera>( "setHorizontalFov", GameCamera::script_setHorizontalFov ),
	ScriptMethod<GameCamera>( "setLookTarget", GameCamera::script_setLookTarget ),
	ScriptMethod<GameCamera>( "setLookSpring", GameCamera::script_setLookSpring ),
	ScriptMethod<GameCamera>( "setLookDamping", GameCamera::script_setLookDamping ),
	ScriptMethod<GameCamera>( "setMoveTarget", GameCamera::script_setMoveTarget ),
	ScriptMethod<GameCamera>( "setMoveSpring", GameCamera::script_setMoveSpring ),
	ScriptMethod<GameCamera>( "setMoveDamping", GameCamera::script_setMoveDamping ),
	ScriptMethod<GameCamera>( "setTimeScale", GameCamera::script_setTimeScale ),
	ScriptMethod<GameCamera>( "setWorldSpaceControl", GameCamera::script_setWorldSpaceControl ),
	ScriptMethod<GameCamera>( "setFront", GameCamera::script_setFront ),
	ScriptMethod<GameCamera>( "setBack", GameCamera::script_setBack ),
	ScriptMethod<GameCamera>( "setPostPitchMove", GameCamera::script_setPostPitchMove ),
	ScriptMethod<GameCamera>( "setPitchAmountUp", GameCamera::script_setPitchAmountUp ),
	ScriptMethod<GameCamera>( "setPitchAmountDown", GameCamera::script_setPitchAmountDown ),
	ScriptMethod<GameCamera>( "setPitchThresholdUp", GameCamera::script_setPitchThresholdUp ),
	ScriptMethod<GameCamera>( "setPitchThresholdDown", GameCamera::script_setPitchThresholdDown ),
	ScriptMethod<GameCamera>( "setTurnThresholdLeft", GameCamera::script_setTurnThresholdLeft ),
	ScriptMethod<GameCamera>( "setTurnThresholdRight", GameCamera::script_setTurnThresholdRight ),
	ScriptMethod<GameCamera>( "setTurnStrengthLeft", GameCamera::script_setTurnStrengthLeft ),
	ScriptMethod<GameCamera>( "setTurnStrengthRight", GameCamera::script_setTurnStrengthRight ),
	ScriptMethod<GameCamera>( "setCrosshairOffset", GameCamera::script_setCrosshairOffset ),
	ScriptMethod<GameCamera>( "setCrosshairLimitLeft", GameCamera::script_setCrosshairLimitLeft ),
	ScriptMethod<GameCamera>( "setCrosshairLimitRight", GameCamera::script_setCrosshairLimitRight ),
	ScriptMethod<GameCamera>( "setCrosshairLimitUp", GameCamera::script_setCrosshairLimitUp ),
	ScriptMethod<GameCamera>( "setCrosshairLimitDown", GameCamera::script_setCrosshairLimitDown ),
	ScriptMethod<GameCamera>( "setCutSceneWideScreenRatio", GameCamera::script_setCutSceneWideScreenRatio ),
	ScriptMethod<GameCamera>( "setMinPortalVisibleSize", GameCamera::script_setMinPortalVisibleSize ),
};

int GameCamera::sm_frameCount = 0;

//-----------------------------------------------------------------------------

static P(Vector<GameObject*>)	s_objs = 0;
static int						s_cameras = 0;

//-----------------------------------------------------------------------------

GameCamera::GameCamera( script::VM* vm, io::InputStreamArchive* arch, 
	SceneManager* sceneMgr, snd::SoundManager* soundMgr, ps::ParticleSystemManager* particleMgr ) :
	GameObject( vm, arch, soundMgr, particleMgr, 0 ),
	m_methodBase( -1 ),
	m_sceneMgr( sceneMgr ),
	m_background( new Node ),
	m_camera( new Camera ),
	m_hfov( 1.f ),
	m_front( 0.10f ),
	m_back( 5000.f ),
	m_target( 0 ),
	m_worldSpaceControl( false ),
	m_timeScale( 1.f ),
	m_moveTargetLocal( 0, 0, 0 ),
	m_lookTargetLocal( 0, 0, 0 ),
	m_worldTargetsDirty( true ),
	m_firstUpdate( true ),
	m_lookPitch( 0 ),
	m_pitchStrengthPos( 0 ),
	m_pitchStrengthNeg( 0 ),
	m_crosshairOffset( 0, 0 ),
	m_posAvg( Allocator<Vector3>(__FILE__) ),
	m_lookAvg( Allocator<Vector3>(__FILE__) ),
	m_posAvgCount( 1 ),
	m_moveTarget( 0, 0, 0 ),
	m_moveTargetVelocity( 0, 0, 0 ),
	m_moveSpringConst( 0, 0, 0 ),
	m_moveDampingConst( 0, 0, 0 ),
	m_postPitchMove( 0, 0, 0 ),
	m_lookTarget( 0, 0, 0 ),
	m_lookTargetVelocity( 0, 0, 0 ),
	m_lookSpringConst( 0, 0, 0 ),
	m_lookDampingConst( 0, 0, 0 ),
	m_blendTime( 0 ),
	m_blendTimeEnd( 0 ),
	m_blendTm( 1 ),
	m_renderNodes( Allocator<P(Node)>(__FILE__) ),
	m_renderObjects( Allocator<GameObject*>(__FILE__) ),
	m_renderCells( Allocator<GameCell*>(__FILE__) ),
	m_minVisiblePortalSize( 5.f/(600.f*.5f) ),
	m_normalizedPixelSize( 0, 0 ),
	m_visibilityChecker( new GamePointObject(CollisionInfo::COLLIDE_GEOMETRY_SOLID) ),
	m_anim( 0 ),
	m_animTime( 0.f ),
	m_cutSceneAspectRatio( 4.f/3.f ),
	m_targetStateOffsets( Allocator<CharacterStateOffset>(__FILE__) ),
	m_newTargetStateOffset(),
	m_previousTargetStateOffset(),
	m_targetStateOffsetBlendTime(0.f),
	m_targetStateOffsetBlendTimeEnd(0.f)
{
	m_methodBase = ScriptUtil<GameCamera,GameObject>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );

	assert( (int)(sizeof(m_turnThreshold)/sizeof(m_turnThreshold[0])) == 2 );
	for ( int i = 0 ; i < (int)(sizeof(m_turnThreshold)/sizeof(m_turnThreshold[0])) ; ++i )
	{
		m_turnThreshold[i] = 0.f;
		m_turnStrength[i] = 0.f;
		m_tiltThreshold[i] = 0.f;
		m_verticalLimit[i] = 0.f;
		m_horizontalLimit[i] = 0.f;
	}

	++s_cameras;
	if ( !s_objs )
		s_objs = new Vector<GameObject*>( Allocator<GameObject*>(__FILE__) );
}

GameCamera::~GameCamera()
{
	if ( --s_cameras == 0 )
		s_objs = 0;
}

void GameCamera::cameraChanged()
{
	m_worldTargetsDirty = true;
	m_posAvg.clear();
	m_lookAvg.clear();
}

void GameCamera::update( float dt0 ) 
{
	float dt = dt0 * m_timeScale;

	GameObject::update( dt );

	if ( m_target && !m_anim )
	{
		Matrix3x3 rot;
		if ( m_worldSpaceControl )
			rot = Matrix3x3( 1.f );
		else
			rot = m_target->rotation();

		Vector3 pos = m_target->position();
		Matrix4x4 tm( rot, pos );
		Vector3 pitchMoveTarget = m_moveTargetLocal - m_lookTargetLocal;
		pitchMoveTarget = m_lookTargetLocal + pitchMoveTarget.rotate( Vector3(1,0,0), m_lookPitch < 0 ? m_lookPitch * m_pitchStrengthNeg : m_lookPitch * m_pitchStrengthPos );
		Vector3 lookTargetLocal = m_lookTargetLocal;

		// evaluate target state based offset
		Vector3 moveOffset( 0, 0, 0 );
		Vector3 lookOffset( 0, 0, 0 );
		getBlendedTargetCharacterStateOffset( dt, &moveOffset, &lookOffset );

		pitchMoveTarget += moveOffset;
		lookTargetLocal += lookOffset;
		Vector3	moveTarget = tm.transform( pitchMoveTarget );
		Vector3	lookTarget = tm.transform( lookTargetLocal );

		// blend if targets have changed radically
		if ( m_worldTargetsDirty )
		{
			if ( !m_firstUpdate )
				m_blendTime = 0.f;
			m_blendTm = transform();
			m_moveTarget = moveTarget;
			m_moveTargetVelocity = Vector3(0,0,0);
			m_lookTarget = lookTarget;
			m_lookTargetVelocity = Vector3(0,0,0);
			m_worldTargetsDirty = false;
		}

		// F = k*x - d*v, 
		// where
		//   k=spring constant
		//   x=distance to rest length
		//   r=rest length
		//   d=damping constant
		//   v=spring velocity

		// compute target springs
		Vector3 distv = moveTarget - m_moveTarget;
		Vector3 forcev( 0.f, 0.f, 0.f );
		for ( int i = 0 ; i < 3 ; ++i )
		{
			Vector3 axis = rot.getColumn(i);
			if ( m_moveSpringConst[i] >= 0.f )
			{
				float x = distv.dot( axis );
				float v = m_moveTargetVelocity.dot( axis );
				float force = 0.f;
				force += m_moveSpringConst[i] * x;
				force -= m_moveDampingConst[i] * v;
				forcev += axis * force;
			}
			else
			{
				m_moveTarget += axis * distv.dot( axis );
			}
		}
		m_moveTargetVelocity += forcev * dt;
		m_moveTarget += m_moveTargetVelocity * dt;
		
		// look target springs
		distv = lookTarget - m_lookTarget;
		forcev = Vector3( 0.f, 0.f, 0.f );
		for ( int i = 0 ; i < 3 ; ++i )
		{
			Vector3 axis = rot.getColumn(i);
			if ( m_lookSpringConst[i] >= 0.f )
			{
				float x = distv.dot( axis );
				float v = m_lookTargetVelocity.dot( axis );
				float force = 0.f;
				force += m_lookSpringConst[i] * x;
				force -= m_lookDampingConst[i] * v;
				forcev += axis * force;
			}
			else
			{
				m_lookTarget += axis * distv.dot( axis );
			}
		}
		m_lookTargetVelocity += forcev * dt;
		m_lookTarget += m_lookTargetVelocity * dt;

		// average
		m_posAvg.add( m_moveTarget );
		m_lookAvg.add( m_lookTarget );
		if ( m_posAvg.size() > m_posAvgCount )
		{
			m_posAvg.remove( 0 );
			m_lookAvg.remove( 0 );
		}
		Vector3 moveTargetAvg(0,0,0);
		Vector3 lookTargetAvg(0,0,0);
		for ( int i = 0 ; i < m_posAvg.size() ; ++i )
		{
			moveTargetAvg += m_posAvg[i];
			lookTargetAvg += m_lookAvg[i];
		}
		moveTargetAvg *= 1.f/m_posAvg.size();
		lookTargetAvg *= 1.f/m_posAvg.size();

		// update camera transform
		m_camera->setPosition( moveTargetAvg );
		m_camera->lookAt( lookTargetAvg, rot.getColumn(1) );

		// blend if requested
		if ( m_blendTime < m_blendTimeEnd )
		{
			float u = m_blendTime/m_blendTimeEnd;
			assert( u >= 0.f && u <= 1.f );
			float v = u;
			
			Quaternion q0( m_blendTm.rotation() );
			Vector3 t0( m_blendTm.translation() );
			Quaternion q1( m_camera->rotation() );
			Vector3 t1( m_camera->transform().translation() );
			m_camera->setRotation( Matrix3x3(q0.slerp(v,q1)) );
			m_camera->setPosition( t0 + (t1-t0)*u );
			m_blendTime += dt0;
		}

		// translate head away from screen center
		rot = m_camera->rotation();
		Vector3 postPitchDelta = rot.getColumn(0) * m_postPitchMove.x + rot.getColumn(1) * m_postPitchMove.y + rot.getColumn(2) * m_postPitchMove.z;

		// set game object tm
		setPosition( m_target->cell(), m_target->position() );
		moveWithoutColliding( m_target->headWorldPosition() - position() );
		CollisionInfo cinfo;
		move( m_camera->position() - position() + postPitchDelta, &cinfo );
		if ( cinfo.isCollision(CollisionInfo::COLLIDE_ALL) )
			moveWithoutColliding( cinfo.normal()*0.01f );
		setRotation( m_camera->rotation() );
	}

	// update camera animation (cut scene)
	if ( m_anim )
	{
		m_animTime += dt;
		applyCameraAnimation();
	}
	else
	{
		m_camera->setHorizontalFov( m_hfov );
		m_camera->setTransform( transform() );
	}

	m_firstUpdate = false;
}

void GameCamera::applyCameraAnimation()
{
	m_anim->setState( m_animTime );
	Matrix4x4 tm = m_anim->worldTransform();
	m_camera->setTransform( tm );
	m_camera->setHorizontalFov( m_anim->horizontalFov() );
	setTransform( cell(), tm );
}

void GameCamera::setTarget( GameCharacter* target )
{
	bool changed = target != m_target;
	m_target = target;
	if ( changed )
		cameraChanged();
}

Camera* GameCamera::getRenderCamera()
{
	if ( !Context::device() )
		throw Exception( Format("Rendering device not initialized before calling GameCamera::getRenderCamera") );

	const int sw = Context::device()->width();
	const int sh = Context::device()->height();

	if ( level()->isActiveCutScene() )
	{
		// wide screen mode
		int h = (int)( sw * (1.f/m_cutSceneAspectRatio) );
		if ( h > sh )
			h = sh;
		m_camera->setViewport( 0, (sh-h)/2, sw, h );

		// cut scene front/back
		GameCutScene* cutScene = level()->activeCutScene();
		m_camera->setFront( cutScene->cameraFront() );
		m_camera->setBack( cutScene->cameraBack() );
	}
	else
	{
		m_camera->setViewport( 0, 0, sw, sh );
		m_camera->setFront( m_front );
		m_camera->setBack( m_back );
	}

	m_camera->setTransform( transform() );
	return m_camera;
}

bool GameCamera::worldSpaceControl() const
{
	return m_worldSpaceControl;
}

bool GameCamera::testPointsVolume( 
	const Vector3* points, int pointCount,
	const Vector4* planes, int planeCount )
{
	assert( planeCount > 0 );

	for ( int i = 0 ; i < planeCount ; ++i )
	{
		const Vector4& plane = planes[i];
		int out = 0;

		const Vector3* end = points + pointCount;
		for ( const Vector3* point = points ; point != end ; ++point )
		{
			Vector4 p( point->x, point->y, point->z, 1.f );
			if ( p.dot(plane) > 0.f )
				++out;
			else
				break;
		}

		if ( out == pointCount )
			return false;
	}
	return true;
}

void GameCamera::checkCollisionsAgainstCell( const Vector3& start, const Vector3& delta, 
	BSPNode* bsptree, GameCell* collisionCell, CollisionInfo* cinfo )
{
	float t = 1.f;
	const BSPPolygon* cpoly = 0;
	BSPCollisionUtil::findLineIntersection( bsptree, start, delta, CollisionInfo::COLLIDE_GEOMETRY_SOLID, &t, &cpoly );

	if ( t < 1.f )
	{
		Vector3 end = start + delta * t;
		Vector4 pl = cpoly->plane();
		*cinfo = CollisionInfo( end, cinfo->positionCell(), collisionCell, cpoly, Vector3(pl.x,pl.y,pl.z), end, 0, collisionCell->bspTree() );
	}
}

void GameCamera::renderRecurse( GameCell* cell, Scene* root, const Viewport& viewport,
	const Matrix4x4& viewTm, const Matrix4x4& projTm )
{
	// add cell geometry
	cell->setVisible( true );
	Node* cellnode = cell->getRenderObject( m_camera );
	cellnode->linkTo( root );
	m_renderNodes.add( cellnode );
	m_renderCells.add( cell );

	// list objects from the cell
	s_objs->clear();
	for ( GameObjectListItem* it = cell->objectsInCell() ; it ; it = it->next() )
	{
		GameObject* obj = it->object();
		if ( !obj->hidden() )
			s_objs->add( obj );
	}

	// get renderable objects
	for ( int i = 0 ; i < s_objs->size() ; ++i )
	{
		GameObject* obj = (*s_objs)[i];
		Node* objnode = obj->getRenderObject( m_camera );
		if ( objnode )
		{
			obj->setVisible( true );
			obj->setRenderedInFrame( sm_frameCount );
			objnode->linkTo( root );
			m_renderNodes.add( objnode );
			m_renderObjects.add( obj );
			obj->incrementRenderCount();
		}
	}
	s_objs->clear();

	// recurse portals
	Vector3 wcorners[GamePortal::NUM_CORNERS]; // world
	Vector3 vcorners[GamePortal::NUM_CORNERS]; // view
	Vector4 scorners[GamePortal::NUM_CORNERS]; // screen
	for ( int i = 0 ; i < cell->portals() ; ++i )
	{
		GamePortal* portal = cell->getPortal( i );
		if ( -1 == m_renderCells.indexOf(portal->target()) )
		{
			// get portal corners in camera space
			Vector3 portalOffs = portal->normal()*0.10f;
			portal->getCorners( wcorners );
			for ( int i = 0 ; i < GamePortal::NUM_CORNERS ; ++i )
				vcorners[i] = viewTm.transform( wcorners[i] - portalOffs );

			// check that portal is (partially) visible
			Vector3 cpos = m_camera->cachedWorldTransform().translation();
			if ( portal->isOnPositiveSide(cpos+portalOffs) &&
				testPointsVolume(vcorners, GamePortal::NUM_CORNERS, m_camera->viewFrustum().planes(), ViewFrustum::PLANE_COUNT) )
			{
				// project portal corners to normalized screen space
				bool intersects = false;
				for ( int i = 0 ; i < GamePortal::NUM_CORNERS ; ++i )
				{
					scorners[i] = projTm * Vector4( vcorners[i].x, vcorners[i].y, vcorners[i].z, 1.f );
					if ( scorners[i].w > Float::MIN_VALUE )
					{
						scorners[i] *= 1.f / scorners[i].w;
					}
					else
					{
						intersects = true;
						break;
					}
				}

				if ( !intersects )
				{
					float sw = Math::abs( scorners[2].x - scorners[0].x );
					float sh = Math::abs( scorners[0].y - scorners[2].y );
					if ( (sw > m_minVisiblePortalSize || sh > m_minVisiblePortalSize) &&
						sw > m_normalizedPixelSize.x && sh > m_normalizedPixelSize.y )
					{
						// get bounding rectangle of the projected portal corners
						Vector2 minc( scorners[0].x, scorners[0].y );
						Vector2 maxc( scorners[0].x, scorners[0].y );
						for ( int i = 1 ; i < GamePortal::NUM_CORNERS ; ++i )
						{
							for ( int k = 0 ; k < 2 ; ++k )
							{
								maxc[k] = Math::max( maxc[k], scorners[i][k] );
								minc[k] = Math::min( minc[k], scorners[i][k] );
							}
						}

						// intersect bounding rectangle with current viewport
						Viewport newViewport;
						newViewport.x0 = Math::max( minc.x, viewport.x0 );
						newViewport.y0 = Math::max( minc.y, viewport.y0 );
						newViewport.x1 = Math::min( maxc.x, viewport.x1 );
						newViewport.y1 = Math::min( maxc.y, viewport.y1 );

						// recurse if new viewport not null
						if ( newViewport.x0 < newViewport.x1 && newViewport.y0 < newViewport.y1 )
						{
							// check if portal is occluded
							const float CORNER_MARGIN = 0.1f;
							Vector3 wcenter = (wcorners[0]+wcorners[1]+wcorners[2]+wcorners[3]) * .25f;
							CollisionInfo cinfo;
							m_visibilityChecker->setPosition( cell, wcenter );
							m_visibilityChecker->move( cpos - m_visibilityChecker->position(), &cinfo );
							bool visible = !cinfo.isCollision(CollisionInfo::COLLIDE_ALL);
							for ( int k = 0 ; k < GamePortal::NUM_CORNERS && !visible ; ++k )
							{
								m_visibilityChecker->setPosition( cell, wcorners[k] + (wcenter-wcorners[k])*CORNER_MARGIN );
								Vector3 delta = cpos - m_visibilityChecker->position();
								float deltalen = delta.lengthSquared();
								if ( deltalen > MIN_OCCLUDER_DISTANCE*MIN_OCCLUDER_DISTANCE )
								{
									deltalen = Math::sqrt( deltalen );
									delta *= (1.f/deltalen) * (deltalen-MIN_OCCLUDER_DISTANCE);
								}
								m_visibilityChecker->move( delta, &cinfo );
								visible = !cinfo.isCollision(CollisionInfo::COLLIDE_ALL);
							}

							if ( visible )
								renderRecurse( portal->target(), root, newViewport, viewTm, projTm );
						}
					}
				}
				else
				{
					renderRecurse( portal->target(), root, viewport, viewTm, projTm );
				}
			}
		}
	}
}

void GameCamera::prepareRender( Context* context, Scene* root, Camera* camera )
{
	// set visible=false for objects which have not been rendered in past few frames
	GameLevel* level = this->level();
	for ( int i = 0 ; i < level->cells() ; ++i )
	{
		GameCell* cell = level->getCell(i);
		cell->setVisible( false );

		for ( GameObjectListItem* it = cell->objectsInCell() ; it ; it = it->next() )
		{
			GameObject* obj = it->object();
			if ( sm_frameCount - obj->renderedInFrame() > 2 )
				obj->setVisible( false );
		}
	}

	// compute pixel size in normalized screen space
	m_normalizedPixelSize.x = .5f / (float)context->width();
	m_normalizedPixelSize.y = .5f / (float)context->height();
	m_normalizedPixelSize *= 3.f; // portal must be at least x pixels wide to be visible

	// prepare render, add camera to list of rendered objects
	m_renderNodes.clear();
	m_renderObjects.clear();
	m_renderNodes.add( camera );
	camera->linkTo( root );
	camera->updateCachedTransforms();

	// collect objects to render
	Viewport viewp;
	viewp.x0 = -1.f;
	viewp.y0 = -1.f;
	viewp.x1 = 1.f;
	viewp.y1 = 1.f;
	Matrix4x4 viewTm = m_camera->viewTransform();
	Matrix4x4 projTm = m_camera->projectionTransform();
	m_renderCells.clear();
	renderRecurse( cell(), root, viewp, viewTm, projTm );
}

void GameCamera::render( Context* context, Scene* root )
{
	Camera* camera = getRenderCamera();

	// prepare scene for rendering
	{dev::Profile pr( "render.prepare" );
	prepareRender( context, root, camera );}

	// render background
	while ( m_background->firstChild() )
		m_background->firstChild()->unlink();
	for ( int i = 0 ; i < m_renderCells.size() ; ++i )
	{
		Node* bg = m_renderCells[i]->background();
		if ( bg )
			bg->linkTo( m_background );
	}
	camera->linkTo( m_background );
	float back = camera->back();
	camera->setBack( 10e3f );
	{dev::Profile pr( "render.background" );
	camera->render();}
	camera->setBack( back );

	// render scene
	{dev::Profile pr( "render.scene" );
	camera->linkTo( root );
	camera->render();}

	// unlink rendered objects
	for ( int i = 0 ; i < m_renderNodes.size() ; ++i )
		m_renderNodes[i]->unlink();

	// increment global frame count
	sm_frameCount++;
}

void GameCamera::setPitch( float pitch ) 
{
	m_lookPitch = pitch;
}

float GameCamera::pitch() const 
{
	return m_lookPitch;
}

float GameCamera::turnThreshold( int side ) const 
{
	assert( side == 0 || side == 1 );
	return m_turnThreshold[side];
}

float GameCamera::turnStrength( int side ) const 
{
	assert( side == 0 || side == 1 );
	return m_turnStrength[side];
}

float GameCamera::tiltThreshold( int side ) const 
{
	assert( side == 0 || side == 1 );
	return m_tiltThreshold[side];
}

float GameCamera::horizontalLimit( int side ) const 
{
	assert( side == 0 || side == 1 );
	return m_horizontalLimit[side];
}

float GameCamera::verticalLimit( int side ) const 
{
	assert( side == 0 || side == 1 );
	return m_verticalLimit[side];
}

Vector2 GameCamera::crosshairCenter() const 
{
	Vector2 crosshairCenter = m_crosshairOffset;

	crosshairCenter.x += ( -turnThreshold(0) + turnThreshold(1) ) * .5f;
	crosshairCenter.y += ( -tiltThreshold(0) + tiltThreshold(1) ) * .5f;

	return crosshairCenter;
}

int GameCamera::frameCount()
{
	return sm_frameCount;
}

void GameCamera::getTargetCharacterStateOffset( CharacterStateOffset* targetOffset ) const
{
	assert( m_target );

	GameCharacter::PrimaryState		primaryState	= m_target->primaryState();
	GameCharacter::SecondaryState	secondaryState	= m_target->secondaryState();
	
	*targetOffset = CharacterStateOffset();
	for ( int i = 0 ; i < m_targetStateOffsets.size() ; ++i )
	{
		const CharacterStateOffset& offs = m_targetStateOffsets[i];
		if ( (offs.primaryState == primaryState || offs.primaryState == GameCharacter::PRIMARY_STATE_UNKNOWN) &&
			(offs.secondaryState == secondaryState || offs.secondaryState == GameCharacter::PRIMARY_STATE_UNKNOWN) )
		{
			*targetOffset = offs;
		}
	}
}

void GameCamera::getBlendedTargetCharacterStateOffset( float dt, math::Vector3* moveOffset, math::Vector3* lookOffset )
{
	CharacterStateOffset targetOffset;
	getTargetCharacterStateOffset( &targetOffset );

	*moveOffset = m_previousTargetStateOffset.moveOffset;
	*lookOffset = m_previousTargetStateOffset.lookOffset;

	if ( m_targetStateOffsetBlendTime <= m_targetStateOffsetBlendTimeEnd )
	{
		// update blend
		float u = 1.f;
		if ( m_targetStateOffsetBlendTimeEnd > Float::MIN_VALUE )
			u = m_targetStateOffsetBlendTime / m_targetStateOffsetBlendTimeEnd;

		*moveOffset = (m_newTargetStateOffset.moveOffset - m_previousTargetStateOffset.moveOffset) * u + m_previousTargetStateOffset.moveOffset;
		*lookOffset = (m_newTargetStateOffset.lookOffset - m_previousTargetStateOffset.lookOffset) * u + m_previousTargetStateOffset.lookOffset;
		m_worldTargetsDirty = true;

		m_targetStateOffsetBlendTime += dt;
		if ( m_targetStateOffsetBlendTime >= m_targetStateOffsetBlendTimeEnd )
			m_previousTargetStateOffset = m_newTargetStateOffset;
	}
	
	if ( m_newTargetStateOffset != targetOffset )
	{
		// new blend
		m_previousTargetStateOffset.moveOffset = *moveOffset;
		m_previousTargetStateOffset.lookOffset = *lookOffset;

		m_targetStateOffsetBlendTime = 0.f;
		m_targetStateOffsetBlendTimeEnd = targetOffset.blendTime;
		if ( m_targetStateOffsetBlendTimeEnd < m_previousTargetStateOffset.blendTime )
			m_targetStateOffsetBlendTimeEnd = m_previousTargetStateOffset.blendTime;
		if ( m_targetStateOffsetBlendTimeEnd < m_newTargetStateOffset.blendTime )
			m_targetStateOffsetBlendTimeEnd = m_newTargetStateOffset.blendTime;

		m_newTargetStateOffset = targetOffset;
	}
}

Vector3 GameCamera::getScreenPoint( const Vector3& worldPoint ) const
{
	Matrix4x4 viewProjTm = m_camera->viewProjectionTransform();
	Vector4 pt1 = viewProjTm * Vector4(worldPoint.x,worldPoint.y,worldPoint.z,1.f);
	pt1 *= 1.f/pt1.w;
	pt1.x = pt1.x * m_camera->viewportWidth()*.5f + m_camera->viewportWidth()*.5f;
	pt1.y = -pt1.y * m_camera->viewportHeight()*.5f + m_camera->viewportHeight()*.5f;
	return Vector3(pt1.x,pt1.y,pt1.z);
}

void GameCamera::printPrimitives() const
{
	Debug::println( "Rendered cells:" );
	Debug::println( "-----------------------------------------------------" );
	for ( int i = 0 ; i < m_renderCells.size() ; ++i )
		Debug::println( "  cell[{0}] = {1}", i, m_renderCells[i]->name() );
	Debug::println( "" );

	Debug::println( "Rendered objects:" );
	Debug::println( "-----------------------------------------------------" );
	for ( int i = 0 ; i < m_renderObjects.size() ; ++i )
		Debug::println( "  obj[{0}] = {1}", i, m_renderObjects[i]->name() );
	Debug::println( "" );

	Debug::println( "Rendered lights:" );
	Debug::println( "-----------------------------------------------------" );
	int numLights = 0;
	for ( int i = 0 ; i < m_renderNodes.size() ; ++i )
	{
		for ( Node* node = m_renderNodes[i].ptr() ; node ; node = node->nextInHierarchy() )
		{
			Light* lt = dynamic_cast<Light*>( node );
			if ( lt )
				Debug::println( "  light[{0}] = {1}", numLights++, lt->name() );
		}
	}
	Debug::println( "" );

	Debug::println( "Rendered primitives:" );
	Debug::println( "-----------------------------------------------------" );

	int objects = 0;
	int meshes = 0;
	int modelPrimitives = 0;
	int modelTriangles = 0;
	int otherPrimitives = 0;
	int lowPolyPrimitives = 0;
	int lowPolyPrimitiveTriangleSpace = 0;
	int lowPolyPrimitiveTriangles = 0;
	for ( int i = 0 ; i < m_renderNodes.size() ; ++i )
	{
		for ( Node* node = m_renderNodes[i] ; node ; node = node->nextInHierarchy() )
		{
			++objects;
			Mesh* mesh = dynamic_cast<Mesh*>( node );
			if ( mesh )
			{
				++meshes;
				Debug::println( "" );
				Debug::println( "Mesh {0} primitives ({1}):", mesh->name(), mesh->primitives() );

				for ( int k = 0 ; k < mesh->primitives() ; ++k )
				{
					Primitive* prim = mesh->getPrimitive(k);
					Model* model = dynamic_cast<Model*>( prim );
					if ( model )
					{
						int triangles = model->indices()/3;
						
						String shaderType = "UNDEFINED";
						Shader* fx = dynamic_cast<Effect*>( prim->shader() );
						if ( fx )
						{
							shaderType = "FX";
						}
						else
						{
							Material* mtl = dynamic_cast<Material*>( prim->shader() );
							if ( mtl )
								shaderType = "MTL_" + mtl->toString();
						}

						Debug::println( "  Material {0} (type={4}, pass={5}) model: {1} triangles, {2} vertices, {3,#.##} vertices/triangle",
							model->shader()->name(), 
							triangles, 
							model->vertices(),
							model->vertices()/(float)(model->indices()/3), 
							shaderType,
							model->shader()->pass() );

						// count how many triangles could be added to low poly primitives
						// so that their triangle count would be at least X polys
						const int MIN_TRIANGLE_BATCH_SIZE = 300;
						if ( triangles < MIN_TRIANGLE_BATCH_SIZE )
						{
							++lowPolyPrimitives;
							lowPolyPrimitiveTriangles += triangles;
							lowPolyPrimitiveTriangleSpace += MIN_TRIANGLE_BATCH_SIZE - triangles;
						}

						modelTriangles += triangles;
						++modelPrimitives;
					}
					else if ( prim->shader() )
					{
						Debug::println( "  Material {0} primitive (pass={1})", prim->shader()->name(), prim->shader()->pass() );
						++otherPrimitives;
					}
				}
			}
		}
	}

	Debug::println( "" );
	Debug::println( "Statistics:" );
	Debug::println( "  Objects = {0}", objects );
	Debug::println( "  Meshes = {0}", meshes );
	Debug::println( "  Model primitives = {0}", modelPrimitives );
	Debug::println( "  Model triangles = {0}", modelTriangles );
	Debug::println( "  Model triangles / primitive = {0}", modelTriangles/modelPrimitives );
	Debug::println( "  Other primitives = {0}", otherPrimitives );
	Debug::println( "  Low poly model primitives = {0} ({1}%)", lowPolyPrimitives, lowPolyPrimitives*100/modelPrimitives );
	Debug::println( "  Low poly model primitive triangles = {0}", lowPolyPrimitiveTriangles );
	Debug::println( "  Triangle space in low poly model primitives = {0}", lowPolyPrimitiveTriangleSpace );
}

int GameCamera::methodCall( script::VM* vm, int i )
{
	return ScriptUtil<GameCamera,GameObject>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

int GameCamera::script_setHorizontalFov( script::VM* vm, const char* funcName )
{
	float v;
	int retv = getParams( vm, funcName, "horizontal view angle in degrees", &v, 1 );
	m_hfov = Math::toRadians( v );
	return retv;
}

int GameCamera::script_setTimeScale( script::VM* vm, const char* funcName )
{
	return getParam( vm, funcName, "time scale", &m_timeScale );
}

int GameCamera::script_setMoveTarget( script::VM* vm, const char* funcName ) 
{
	float v[3];
	int retv = getParams( vm, funcName, "offset (x,y,z) from move target position", v, 3 );
	m_moveTargetLocal = Vector3( v[0], v[1], v[2] );
	m_worldTargetsDirty = true;
	return retv;
}

int GameCamera::script_setLookTarget( script::VM* vm, const char* funcName ) 
{
	float v[3];
	int retv = getParams( vm, funcName, "look target (x,y,z)", v, 3 );
	m_lookTargetLocal = Vector3( v[0], v[1], v[2] );
	m_worldTargetsDirty = true;
	return retv;
}

int GameCamera::script_setMoveSpring( script::VM* vm, const char* funcName ) 
{
	float v[3];
	int retv = getParams( vm, funcName, "spring constants", v, 3 );
	m_moveSpringConst = Vector3( v[0], v[1], v[2] );
	m_worldTargetsDirty = true;
	return retv;
}

int GameCamera::script_setMoveDamping( script::VM* vm, const char* funcName ) 
{
	float v[3];
	int retv = getParams( vm, funcName, "damping constants", v, 3 );
	m_moveDampingConst = Vector3( v[0], v[1], v[2] );
	m_worldTargetsDirty = true;
	return retv;
}

int GameCamera::script_setLookSpring( script::VM* vm, const char* funcName ) 
{
	float v[3];
	int retv = getParams( vm, funcName, "spring constants", v, 3 );
	m_lookSpringConst = Vector3( v[0], v[1], v[2] );
	m_worldTargetsDirty = true;
	return retv;
}

int GameCamera::script_setLookDamping( script::VM* vm, const char* funcName ) 
{
	float v[3];
	int retv = getParams( vm, funcName, "damping constants", v, 3 );
	m_lookDampingConst = Vector3( v[0], v[1], v[2] );
	m_worldTargetsDirty = true;
	return retv;
}

int GameCamera::script_setPostPitchMove( script::VM* vm, const char* funcName ) 
{
	float v[3];
	int retv = getParams( vm, funcName, "post pitch offset", v, 3 );
	m_postPitchMove = Vector3( v[0], v[1], v[2] );
	m_worldTargetsDirty = true;
	return retv;
}

int GameCamera::script_setWorldSpaceControl( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 1 || (!vm->isNumber(1) && !vm->isNil(1)) )
		throw ScriptException( Format("Function {0} expects boolean", funcName) );
	m_worldSpaceControl = !vm->isNil(1);
	return 0;
}

int	GameCamera::script_setAverageCount( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 1 || vm->getType(1) != VM::TYPE_NUMBER )
		throw ScriptException( Format("Function {0} expects average count", funcName) );
	int count = (int)vm->toNumber(1);
	if ( count < 1 )
		throw ScriptException( Format("Function {0} expects average count", funcName) );
	m_posAvgCount = count;
	return 0;
}

int	GameCamera::script_setFront( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 1 || vm->getType(1) != VM::TYPE_NUMBER )
		throw ScriptException( Format("Function {0} expects front plane distance", funcName) );

	m_front = vm->toNumber(1);
	return 0;
}

int GameCamera::script_setBack( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 1 || vm->getType(1) != VM::TYPE_NUMBER )
		throw ScriptException( Format("Function {0} expects back plane distance", funcName) );
	
	m_back = vm->toNumber(1);
	return 0;
}

int GameCamera::script_setBlendTime( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 1 || vm->getType(1) != VM::TYPE_NUMBER )
		throw ScriptException( Format("Function {0} expects time to blend to another camera position", funcName) );
	
	m_blendTime = m_blendTimeEnd = vm->toNumber(1);
	return 0;
}

int GameCamera::script_addTargetStateOffset( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_STRING, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),1) )
		throw ScriptException( Format("{0} expects character primary state name (or UNKNOWN), secondary state name (or UNKNOWN), move offset (x,y,z), look-at offset (x,y,z) and blend time", funcName) );

	int param = 1;
	CharacterStateOffset offs;
	offs.primaryState = GameCharacter::toPrimaryState( vm->toString(param++) );
	offs.secondaryState = GameCharacter::toSecondaryState( vm->toString(param++) );
	for ( int i = 0 ; i < 3 ; ++i )
		offs.moveOffset[i] = vm->toNumber( param++ );
	for ( int i = 0 ; i < 3 ; ++i )
		offs.lookOffset[i] = vm->toNumber( param++ );
	offs.blendTime = vm->toNumber( param++ );

	m_targetStateOffsets.add( offs );
	return 0;
}

int GameCamera::script_setPitchAmountUp( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects pitch amount in degrees", funcName) );

	m_pitchStrengthNeg = Math::toRadians( vm->toNumber(1) );

	return 0;
}

int GameCamera::script_setPitchAmountDown( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects amount in degrees", funcName) );

	m_pitchStrengthPos = Math::toRadians( vm->toNumber(1) );

	return 0;
}

int GameCamera::script_setPitchThresholdUp( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects percent of half screen height", funcName) );

	m_tiltThreshold[0] = vm->toNumber(1);

	return 0;
}

int GameCamera::script_setPitchThresholdDown( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects percent of half screen height", funcName) );

	m_tiltThreshold[1] = vm->toNumber(1);

	return 0;
}

int GameCamera::script_setTurnThresholdLeft( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects percent of half screen width", funcName) );

	m_turnThreshold[0] = vm->toNumber(1);

	return 0;
}

int GameCamera::script_setTurnThresholdRight( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects percent of half screen width", funcName) );

	m_turnThreshold[1] = vm->toNumber(1);

	return 0;
}

int GameCamera::script_setTurnStrengthLeft( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects amount relative to character turning speed", funcName) );

	m_turnStrength[0] = vm->toNumber(1);

	return 0;
}

int GameCamera::script_setTurnStrengthRight( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects amount relative to character turning speed", funcName) );

	m_turnStrength[1] = vm->toNumber(1);

	return 0;
}

int GameCamera::script_setCrosshairOffset( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects crosshair offset aiming rectangle center in projection space coordinates (-1..1)", funcName) );

	float x = vm->toNumber(1);
	float y = vm->toNumber(2);

	if ( !( -1.f < x && x < 1.f && -1.f < y && y < 1.f ) )
		throw ScriptException( Format("{0} expects crosshair offset aiming rectangle center in projection space coordinates (-1..1)", funcName) );

	m_crosshairOffset = Vector2(x,y);

	return 0;
}

int GameCamera::script_setCrosshairLimitLeft( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects limit in projection space coordinates (0..1)", funcName) );

	if ( !( 0.f < vm->toNumber(1) && vm->toNumber(1) < 1.f ) )
		throw ScriptException( Format("{0} expects limit in projection space coordinates (0..1)", funcName) );

	m_horizontalLimit[0] = vm->toNumber(1);

	return 0;
}

int GameCamera::script_setCrosshairLimitRight( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects limit in projection space coordinates (0..1)", funcName) );

	if ( !( 0.f < vm->toNumber(1) && vm->toNumber(1) < 1.f ) )
		throw ScriptException( Format("{0} expects limit in projection space coordinates (0..1)", funcName) );

	m_horizontalLimit[1] = vm->toNumber(1);

	return 0;
}

int GameCamera::script_setCrosshairLimitUp( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects limit in projection space coordinates (0..1)", funcName) );

	if ( !( 0.f < vm->toNumber(1) && vm->toNumber(1) < 1.f ) )
		throw ScriptException( Format("{0} expects limit in projection space coordinates (0..1)", funcName) );

	m_verticalLimit[0] = vm->toNumber(1);

	return 0;
}

int GameCamera::script_setCrosshairLimitDown( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects limit in projection space coordinates (0..1)", funcName) );

	if ( !( 0.f < vm->toNumber(1) && vm->toNumber(1) < 1.f ) )
		throw ScriptException( Format("{0} expects limit in projection space coordinates (0..1)", funcName) );

	m_verticalLimit[1] = vm->toNumber(1);

	return 0;
}

int GameCamera::script_setMinPortalVisibleSize( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects minimum visible portal size in normalized screen space", funcName) );
	
	m_minVisiblePortalSize = vm->toNumber(1);
	return 0;
}

int GameCamera::script_playAnimation( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_STRING, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),1) )
		throw ScriptException( Format("{0} expects camera animation name, scene file and optional start time", funcName) );
	
	int		argc		= vm->top();
	String	cameraName	= vm->toString(1);
	String	sceneName	= vm->toString(2);
	float	startTime	= argc > 2 ? vm->toNumber(3) : 0.f;

	P(Node) root		= m_sceneMgr->getScene( sceneName, SceneFile::LOAD_ANIMATIONS );
	Camera*	camera		= dynamic_cast<Camera*>( NodeUtil::findNodeByName(root, cameraName) );

	if ( !camera )
		throw ScriptException( Format("{0} expects camera animation name and scene file ({1} not found in {2})", funcName, cameraName, sceneName) );

	m_anim = camera;
	m_animTime = startTime;
	m_worldTargetsDirty = true;
	applyCameraAnimation();

	// make sure animation doesn't wrap around
	NodeUtil::setNodePreBehaviour( camera, Interpolator::BEHAVIOUR_CONSTANT );
	NodeUtil::setNodeEndBehaviour( camera, Interpolator::BEHAVIOUR_CONSTANT );

	// DEBUG:
	float endTime = 0.f;
	Interpolator* intp = dynamic_cast<Interpolator*>( camera->positionController() );
	if ( intp && intp->keys() > 0 )
		endTime = intp->getKeyTime( intp->keys()-1 );
	Debug::println( "\"{0}\":playAnimation( {1}, {2}, start={3}, end={4} )", name(), cameraName, sceneName, startTime*30.f, endTime*30.f );

	// inform level that view has changed
	level()->signalViewChanged();
	return 0;
}

int GameCamera::script_stopAnimation( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} stops any active camera animation", funcName) );
	
	m_anim = 0;
	m_worldTargetsDirty = true;
	return 0;
}

int GameCamera::script_getAnimationStartTime( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects camera animation name and scene file", funcName) );
	
	String	cameraName	= vm->toString(1);
	String	sceneName	= vm->toString(2);

	P(Node) root		= m_sceneMgr->getScene( sceneName, SceneFile::LOAD_ANIMATIONS );
	Camera*	camera		= dynamic_cast<Camera*>( NodeUtil::findNodeByName(root, cameraName) );

	if ( !camera )
		throw ScriptException( Format("{0} expects camera animation name and scene file ({1} not found in {2})", funcName, cameraName, sceneName) );

	float t = 0.f;
	Interpolator* intp = dynamic_cast<Interpolator*>( camera->positionController() );
	if ( intp && intp->keys() > 0 )
		t = intp->getKeyTime( 0 );
	vm->pushNumber( t );
	return 1;
}

int GameCamera::script_getAnimationEndTime( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects camera animation name and scene file", funcName) );
	
	String	cameraName	= vm->toString(1);
	String	sceneName	= vm->toString(2);

	P(Node) root		= m_sceneMgr->getScene( sceneName, SceneFile::LOAD_ANIMATIONS );
	Camera*	camera		= dynamic_cast<Camera*>( NodeUtil::findNodeByName(root, cameraName) );

	if ( !camera )
		throw ScriptException( Format("{0} expects camera animation name and scene file ({1} not found in {2})", funcName, cameraName, sceneName) );

	float t = 0.f;
	Interpolator* intp = dynamic_cast<Interpolator*>( camera->positionController() );
	if ( intp && intp->keys() > 0 )
		t = intp->getKeyTime( intp->keys()-1 );
	vm->pushNumber( t );
	return 1;
}

int GameCamera::script_setCutSceneWideScreenRatio( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects width:height ratio used in cut scenes", funcName) );

	float ratio = vm->toNumber(1);
	if ( ratio < 1.f )
		throw ScriptException( Format("{0}: Cut scene wide screen width:height ratio cannot be below 1 (was {1})", funcName, ratio) );

	m_cutSceneAspectRatio = ratio;
	return 0;
}

//-----------------------------------------------------------------------------

GameCamera::CharacterStateOffset::CharacterStateOffset() :
	primaryState( GameCharacter::PRIMARY_STATE_UNKNOWN ),
	secondaryState( GameCharacter::SECONDARY_STATE_UNKNOWN ),
	moveOffset( 0, 0, 0 ),
	lookOffset( 0, 0, 0 ),
	blendTime( 0.f )
{
}

bool GameCamera::CharacterStateOffset::operator==( const CharacterStateOffset& other ) const
{
	return primaryState == other.primaryState && secondaryState == other.secondaryState;
}

bool GameCamera::CharacterStateOffset::operator!=( const CharacterStateOffset& other ) const
{
	return primaryState != other.primaryState || secondaryState != other.secondaryState;
}
