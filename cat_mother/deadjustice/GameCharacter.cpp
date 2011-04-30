#include "GameCharacter.h"
#include "GameLevel.h"
#include "GameCell.h"
#include "GameWeapon.h"
#include "GameBSPTree.h"
#include "GameRenderPass.h"
#include "GameNoiseManager.h"
#include "ScriptUtil.h"
#include "CollisionInfo.h"
#include "GameProjectile.h"
#include "GamePointObject.h"
#include "GameSphereObject.h"
#include "MovementAnimation.h"
#include "ProjectileManager.h"
#include "UserControl.h"
#include "ComputerControl.h"
#include <io/InputStreamArchive.h>
#include <snd/SoundManager.h>
#include <bsp/BSPPolygon.h>
#include <fsm/StateMachine.h>
#include <sg/LOD.h>
#include <sg/Mesh.h>
#include <sg/Scene.h>
#include <sg/Light.h>
#include <sg/Model.h>
#include <sg/Camera.h>
#include <sg/MorphTarget.h>
#include <sg/DirectLight.h>
#include <sg/LineList.h>
#include <sg/VertexLock.h>
#include <ps/ParticleSystemManager.h>
#include <bsp/BSPCollisionUtil.h>
#include <sgu/NodeUtil.h>
#include <sgu/MeshUtil.h>
#include <sgu/NodeGroupSet.h>
#include <sgu/SceneManager.h>
#include <dev/Profile.h>
#include <pix/Color.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include <lang/Float.h>
#include <lang/Character.h>
#include <math/lerp.h>
#include <math/Intersection.h>
#include <anim/VectorInterpolator.h>
#include <script/VM.h>
#include <script/ScriptException.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace pix;
using namespace sgu;
using namespace bsp;
using namespace lang;
using namespace util;
using namespace math;
using namespace anim;
using namespace script;

//-----------------------------------------------------------------------------

/** Early culling optimization for close combat checks. */
static const float CHARACTER_WAY_TOO_FAR_FOR_CLOSE_COMBAT = 2.f;

/** Don't morph face if character is looking away from camera. */
static const float CHARACTER_FACING_AWAY_FROM_CAMERA_ANGLE = Math::toRadians(90.f);

//-----------------------------------------------------------------------------

ScriptMethod<GameCharacter> GameCharacter::sm_methods[] =
{
	//ScriptMethod<GameCharacter>( "funcName", script_funcName ),
	ScriptMethod<GameCharacter>( "activateMovementAnimation", script_activateMovementAnimation ),
	ScriptMethod<GameCharacter>( "activateSecondaryMovementAnimation", script_activateSecondaryMovementAnimation ),
	ScriptMethod<GameCharacter>( "addAnimation", script_addAnimation ),
	ScriptMethod<GameCharacter>( "addAnimationListener", script_addAnimationListener ),
	ScriptMethod<GameCharacter>( "addControlSector", script_addControlSector ),
	ScriptMethod<GameCharacter>( "addCollisionBone", script_addCollisionBone ),
	ScriptMethod<GameCharacter>( "addMorphAnimation", script_addMorphAnimation ),
	ScriptMethod<GameCharacter>( "addPhysicalCombatMove", script_addPhysicalCombatMove ),
	ScriptMethod<GameCharacter>( "addPrimaryStateListener", script_addPrimaryStateListener ),
	ScriptMethod<GameCharacter>( "addSecondaryAnimation", script_addSecondaryAnimation ),
	ScriptMethod<GameCharacter>( "addSecondaryStateListener", script_addSecondaryStateListener ),
	ScriptMethod<GameCharacter>( "addWeaponToInventory", script_addWeaponToInventory ),
	ScriptMethod<GameCharacter>( "aimAt", script_aimAt ),
	ScriptMethod<GameCharacter>( "blendAnimation", script_blendAnimation ),
	ScriptMethod<GameCharacter>( "blendMorphAnimation", script_blendMorphAnimation ),
	ScriptMethod<GameCharacter>( "blendSecondaryAnimation", script_blendSecondaryAnimation ),
	ScriptMethod<GameCharacter>( "canSee", script_canSee ),
	ScriptMethod<GameCharacter>( "canMove", script_canMove ),
	ScriptMethod<GameCharacter>( "clearSecondaryAnimations", script_clearSecondaryAnimations ),
	ScriptMethod<GameCharacter>( "createMovementAnimation", script_createMovementAnimation ),
	ScriptMethod<GameCharacter>( "createSecondaryMovementAnimation", script_createSecondaryMovementAnimation ),
	ScriptMethod<GameCharacter>( "cycleWeapon", script_cycleWeapon ),
	ScriptMethod<GameCharacter>( "computerControl", script_computerControl ),
	ScriptMethod<GameCharacter>( "setHeadTurnFixBlendDelay", script_setHeadTurnFixBlendDelay ),
	ScriptMethod<GameCharacter>( "evaluatePrimaryState", script_evaluatePrimaryState ),
	ScriptMethod<GameCharacter>( "evaluateSecondaryState", script_evaluateSecondaryState ),
	ScriptMethod<GameCharacter>( "getAnimLength", script_getAnimLength ),
	ScriptMethod<GameCharacter>( "getComputerControl", script_getComputerControl ),
	ScriptMethod<GameCharacter>( "getHealth", script_getHealth ),
	ScriptMethod<GameCharacter>( "getPrimaryStateEndTime", script_getPrimaryStateEndTime ),
	ScriptMethod<GameCharacter>( "getSecondaryStateEndTime", script_getSecondaryStateEndTime ),
	ScriptMethod<GameCharacter>( "groundMaterial", script_groundMaterial ),
	ScriptMethod<GameCharacter>( "isAiming", script_isAiming ),
	ScriptMethod<GameCharacter>( "isCrouched", script_isCrouched ),
	ScriptMethod<GameCharacter>( "isDead", script_isDead ),
	ScriptMethod<GameCharacter>( "isHurting", script_isHurting ),
	ScriptMethod<GameCharacter>( "isReloading", script_isReloading ),
	ScriptMethod<GameCharacter>( "isRolling", script_isRolling ),
	ScriptMethod<GameCharacter>( "isSneaking", script_isSneaking ),
	ScriptMethod<GameCharacter>( "isWalking", script_isWalking ),
	ScriptMethod<GameCharacter>( "isRunning", script_isRunning ),
	ScriptMethod<GameCharacter>( "playMorphAnimation", script_playMorphAnimation ),
	ScriptMethod<GameCharacter>( "playMorphAnimationOnce", script_playMorphAnimationOnce ),
	ScriptMethod<GameCharacter>( "playWorldSpaceAnimation", script_playWorldSpaceAnimation ),
	ScriptMethod<GameCharacter>( "resetInputState", script_resetInputState ),
	ScriptMethod<GameCharacter>( "setSecondaryMovementAnimation", script_setSecondaryMovementAnimation ),
	ScriptMethod<GameCharacter>( "setAimingTimeAfterShooting", script_setAimingTimeAfterShooting ),
	ScriptMethod<GameCharacter>( "setAnimationFolder", script_setAnimationFolder ),
	ScriptMethod<GameCharacter>( "setAnimation", script_setAnimation ),
	ScriptMethod<GameCharacter>( "setBalanceLostThreshold", script_setBalanceLostThreshold ),
	ScriptMethod<GameCharacter>( "setCharacterCollisionRadius", script_setCharacterCollisionRadius ),
	ScriptMethod<GameCharacter>( "setCrouchWalkingSpeed", script_setCrouchWalkingSpeed ),
	ScriptMethod<GameCharacter>( "setCrouchStrafingSpeed", script_setCrouchStrafingSpeed ),
	ScriptMethod<GameCharacter>( "setCrouchBackwardSpeed", script_setCrouchBackwardSpeed ),
	ScriptMethod<GameCharacter>( "setFriction", script_setFriction ),
	ScriptMethod<GameCharacter>( "setFallingFriction", script_setFallingFriction ),
	ScriptMethod<GameCharacter>( "setGravity", script_setGravity ),
	ScriptMethod<GameCharacter>( "setHealth", script_setHealth ),
	ScriptMethod<GameCharacter>( "setLOD", script_setLOD ),
	ScriptMethod<GameCharacter>( "setLookingHeadBoneTransformFix", script_setLookingHeadBoneTransformFix ),
	ScriptMethod<GameCharacter>( "setMaxAnimSlewRatePrimary", script_setMaxAnimSlewRatePrimary ),
	ScriptMethod<GameCharacter>( "setMaxAnimSlewRateSecondary", script_setMaxAnimSlewRateSecondary ),
	ScriptMethod<GameCharacter>( "setMesh", script_setMesh ),
	ScriptMethod<GameCharacter>( "setMorphBase", script_setMorphBase ),
	ScriptMethod<GameCharacter>( "setPhysicalHitRange", script_setPhysicalHitRange ),	
	ScriptMethod<GameCharacter>( "setRotationSpeed", script_setRotationSpeed ),
	ScriptMethod<GameCharacter>( "setRollingSpeedForward", script_setRollingSpeedForward ),
	ScriptMethod<GameCharacter>( "setRollingSpeedBackward", script_setRollingSpeedBackward ),
	ScriptMethod<GameCharacter>( "setRollingSpeedSideways", script_setRollingSpeedSideways ),
	ScriptMethod<GameCharacter>( "setPrimaryState", script_setPrimaryState ),
	ScriptMethod<GameCharacter>( "setSecondaryState", script_setSecondaryState ),
	ScriptMethod<GameCharacter>( "setSteepSurface", script_setSteepSurface ),
	ScriptMethod<GameCharacter>( "setWeapon", script_setWeapon ),
	ScriptMethod<GameCharacter>( "stateControllable", script_stateControllable ),
	ScriptMethod<GameCharacter>( "stateFalling", script_stateFalling ),
	ScriptMethod<GameCharacter>( "stateInAir", script_stateInAir ),
	ScriptMethod<GameCharacter>( "stopWorldSpaceAnimation", script_stopWorldSpaceAnimation ),
	ScriptMethod<GameCharacter>( "weapon", script_weapon ),
	ScriptMethod<GameCharacter>( "receiveDamage", script_receiveDamage ),
	ScriptMethod<GameCharacter>( "setSneakingSpeedRange", script_setSneakingSpeedRange ),
	ScriptMethod<GameCharacter>( "setWalkingSpeedRange", script_setWalkingSpeedRange ),
	ScriptMethod<GameCharacter>( "setRunningSpeedRange", script_setRunningSpeedRange ),
	ScriptMethod<GameCharacter>( "setPeekMoveCheckDistance", script_setPeekMoveCheckDistance ),
	ScriptMethod<GameCharacter>( "minSneakingSpeed", script_minSneakingSpeed ),
	ScriptMethod<GameCharacter>( "minWalkingSpeed", script_minWalkingSpeed ),
	ScriptMethod<GameCharacter>( "minRunningSpeed", script_minRunningSpeed ),
	ScriptMethod<GameCharacter>( "maxSneakingSpeed", script_maxSneakingSpeed ),
	ScriptMethod<GameCharacter>( "maxWalkingSpeed", script_maxWalkingSpeed ),
	ScriptMethod<GameCharacter>( "maxRunningSpeed", script_maxRunningSpeed ),
	ScriptMethod<GameCharacter>( "setSneakingControlRange", script_setSneakingControlRange ),
	ScriptMethod<GameCharacter>( "setWalkingControlRange", script_setWalkingControlRange ),
	ScriptMethod<GameCharacter>( "setRunningControlRange", script_setRunningControlRange ),
	ScriptMethod<GameCharacter>( "minSneakingControl", script_minSneakingControl ),
	ScriptMethod<GameCharacter>( "minWalkingControl", script_minWalkingControl ),
	ScriptMethod<GameCharacter>( "minRunningControl", script_minRunningControl ),
	ScriptMethod<GameCharacter>( "maxSneakingControl", script_maxSneakingControl ),
	ScriptMethod<GameCharacter>( "maxWalkingControl", script_maxWalkingControl ),
	ScriptMethod<GameCharacter>( "maxRunningControl", script_maxRunningControl ),
	ScriptMethod<GameCharacter>( "physicalAttack", script_physicalAttack ),
	ScriptMethod<GameCharacter>( "getPhysicalAnim", script_getPhysicalAnim ),
	ScriptMethod<GameCharacter>( "endPhysicalAttack", script_endPhysicalAttack ),
	ScriptMethod<GameCharacter>( "throwProjectile", script_throwProjectile ),
	ScriptMethod<GameCharacter>( "setThrowBone", script_setThrowBone ),
	ScriptMethod<GameCharacter>( "setThrowAngle", script_setThrowAngle ),
	ScriptMethod<GameCharacter>( "userControl", script_userControl ),
	ScriptMethod<GameCharacter>( "primaryState", script_primaryState ),
	ScriptMethod<GameCharacter>( "secondaryState", script_secondaryState ),
};

const GameCharacter::StateInfo GameCharacter::sm_primaryStates[] =
{
	{"UNKNOWN",						0},
	{"STANDING",					0},
	{"WALKING",						0},
	{"WALKING_BACKWARD",			0},
	{"STRAFING",					0},
	{"JUMPING",						STATE_WAIT},
	{"ROLLING_FORWARD",				STATE_WAIT},
	{"ROLLING_BACKWARD",			STATE_WAIT},
	{"ROLLING_LEFT",				STATE_WAIT},
	{"ROLLING_RIGHT",				STATE_WAIT},
	{"CROUCHING_DOWN",				STATE_WAIT},
	{"CROUCHED",					0},
	{"CROUCHED_WALKING",			0},
	{"UNCROUCHING",					STATE_WAIT},
	{"PEEKING_LEFT",				0},
	{"UNPEEKING_LEFT",				STATE_WAIT},
	{"PEEKING_RIGHT",				0},
	{"UNPEEKING_RIGHT",				STATE_WAIT},
	{"FALLING",						STATE_WAIT},
	{"INAIR",						0},
	{"PROJECTILE_HURT",				STATE_WAIT},
	{"PHYSICAL_CONTACT_STRIKE",		STATE_WAIT},
	{"PHYSICAL_KICK",				STATE_WAIT},
	{"PHYSICAL_LOCK",				0},
	{"PHYSICAL_HURT",				STATE_WAIT},
	{"PHYSICAL_INLOCK",				0},
	{"DEAD",						STATE_WAIT},
	{"CUSTOM",						STATE_WAIT},
};

const GameCharacter::StateInfo GameCharacter::sm_secondaryStates[] =
{
	{"UNKNOWN",						0},
	{"IDLING",						0},
	{"LOOKING",						0},
	{"ATTACKING",					0},
	{"AIMING",						0},
	{"HOLDING_AIM",					0},
	{"CHANGING_CLIP",				STATE_WAIT},
	{"CYCLING_WEAPON",				STATE_WAIT},
	{"PHYSICAL_CONTACT_STRIKE",		STATE_WAIT},
	{"PHYSICAL_STRIKE",				STATE_WAIT},
	{"PHYSICAL_LOCK",				0},
	{"PROJECTILE_HURT_WHILE_AIMING",STATE_WAIT},
	{"THROWING_EMPTY_SHELL",		STATE_WAIT},
	{"DEAD",						0},
};

//-----------------------------------------------------------------------------

/** Buffer used for applying world space animations. */
static P(Vector<Node*>)		s_animNodes = 0;

/** Buffer used for applying world space animations. */
static P(Vector<String>)	s_animNodeNames = 0;

/** Number of characters. */
static int					s_characters = 0;

//-----------------------------------------------------------------------------

GameCharacter::GameCharacter( VM* vm, io::InputStreamArchive* arch, sgu::NodeGroupSet* anims, 
	SceneManager* sceneMgr, snd::SoundManager* soundMgr, ps::ParticleSystemManager* particleMgr,
	ProjectileManager* projectileMgr, GameNoiseManager* noiseMgr ) :
	GameObject( vm, arch, soundMgr, particleMgr, noiseMgr ),
	m_methodBase( -1 ),
	m_arch( arch ),
	m_soundMgr( soundMgr ),
	m_particleMgr( particleMgr ),
	m_sceneMgr( sceneMgr ),
	m_projectileMgr( projectileMgr ),
	m_noiseMgr( noiseMgr ),
	m_movementNoise( 0 ),
	m_anims( anims ),
	m_animationFolderName(),
	m_primaryBlender( new Blender ),
	m_secondaryBlender( new Blender ),
	m_primaryState( PRIMARY_STATE_UNKNOWN ),
	m_primaryStateTime( 0.f ),
	m_secondaryState( SECONDARY_STATE_UNKNOWN ),
	m_secondaryStateTime( 0.f ),
	m_steepAngleCos( 0.f ),
	m_groundPlane( 0, 0, 0, 0 ),
	m_slideNormal( 0, 0, 0 ),
	m_maxStepHeight( 0.f ),
	m_inAirCounter( 0 ),
	m_groundCollisionInfo(),
	m_visibilityChecker( new GamePointObject(CollisionInfo::COLLIDE_GEOMETRY_SOLID) ),
	m_collisionChecker( new GameSphereObject(CollisionInfo::COLLIDE_GEOMETRY_SOLID+CollisionInfo::COLLIDE_GEOMETRY_SEETHROUGH+CollisionInfo::COLLIDE_OBJECT) ),
	m_collisionCheckerPoint( new GamePointObject(CollisionInfo::COLLIDE_GEOMETRY_SOLID+CollisionInfo::COLLIDE_GEOMETRY_SEETHROUGH+CollisionInfo::COLLIDE_OBJECT) ),
	m_boneCollisionBoxes( Allocator<BoneCollisionBox>(__FILE__) ),
	m_rootCollisionBox(),
	m_characterCollisionRadius( 0.f ),
	m_mesh( 0 ),
	m_meshHeadBone( 0 ),
	m_meshLeftFootBone( 0 ),
	m_meshRightFootBone( 0 ),
	m_meshSpineBone( 0 ),
	m_meshLeftForeArmBone( 0 ),
	m_meshRightForeArmBone( 0 ),
	m_meshLeftUpperArmBone( 0 ),
	m_meshRightUpperArmBone( 0 ),
	m_meshThrowBone( 0 ),
	m_lod( 0 ),
	m_groundLightmapColor(1.f,1.f,1.f,1.f),
	m_primaryStateListeners( Allocator<P(Vector<StateListener>)>(__FILE__) ),
	m_secondaryStateListeners( Allocator<P(Vector<StateListener>)>(__FILE__) ),
	m_animParamBuffer( Allocator<AnimationParams>(__FILE__) ),
	m_nodeBuffer( Allocator<anim::Animatable*>(__FILE__) ),
	m_timeBuffer( Allocator<float>(__FILE__) ),
	m_weightBuffer( Allocator<float>(__FILE__) ),
	m_nameBuffer( Allocator<String>(__FILE__) ),
	m_controlPtrTable( Allocator<HashtablePair<String, Vector3*> >(__FILE__) ),
	m_primaryTime( 0.f ),
	m_primarySpeed( 0.f ),
	m_primaryAnimBlenderID( Blender::INVALIDID ),
	m_animListeners( Allocator<AnimationListener>(__FILE__) ),
	m_movementAnimations( Allocator<HashtablePair<String, P(MovementAnimation)> >(__FILE__) ),
	m_activeMovementAnimation( "" ),
	m_secondaryTime( 0.f ),
	m_secondarySpeed( 0.f ),
	m_secondaryAnimBlenderID( Blender::INVALIDID ),
	m_secondaryMovementAnimations( Allocator<HashtablePair<String, P(MovementAnimation)> >(__FILE__) ),
	m_activeSecondaryMovementAnimation( "" ),
	m_preparedSecondaryAnimations( Allocator<HashtablePair<String, SecondaryAnimationParams> >(__FILE__) ),
	m_headLookTransformFix( Matrix3x3(1) ),
	m_headTurnFixBlendDelay( 0 ),
	m_headTurnFixBlendTime( 0 ),
	m_morphAnims( Allocator<P(MorphAnimation)>(__FILE__) ),
	m_morphBases( Allocator<P(Mesh)>(__FILE__) ),
	m_rotationSpeed( 0.f ),
	m_friction( 0.f ),		
	m_fallingFriction( 0.f ),			
	m_maxBackwardSpeed( 0.f ),
	m_gravity( 0.f ),
	m_crouchWalkingSpeed( 0.f ),
	m_crouchStrafingSpeed( 0.f ),
	m_crouchBackwardSpeed( 0.f ),
	m_movementVector( 0, 0, 0 ),
	m_movementControlVector( 0, 0, 0 ),
	m_aimVector( 0, 0, 0 ),
	m_lookVector( 0, 0, 1 ),
	m_aimCrosshairNormalized( 0, 0, 0 ),
	m_rollingSpeedForward( 0 ),		
	m_rollingSpeedBackward( 0 ),
	m_rollingSpeedSideways( 0 ),
	m_aimingTimeAfterShooting( 0.f ),
	m_peekMoveCheckDistance( 0 ),
	m_throwAngle( 0 ),
	m_balance( 1.f ),
	m_balanceLostThreshold( 0 ),
	m_hitRange( 0 ),
	m_physicalCombatMoves( Allocator<HashtablePair<String, P(PhysicalCombatMove)> >(__FILE__) ),
	m_currentPhysicalAttack( "" ),
	m_minSneakingSpeed(-1),
	m_maxSneakingSpeed(-1),
	m_minWalkingSpeed(-1),
	m_maxWalkingSpeed(-1),
	m_minRunningSpeed(-1),
	m_maxRunningSpeed(-1),
	m_minSneakingControl(-1),
	m_maxSneakingControl(-1),
	m_minWalkingControl(-1),
	m_maxWalkingControl(-1),
	m_minRunningControl(-1),
	m_maxRunningControl(-1),
	m_movementControlSectors( Allocator<ControlSector>(__FILE__) ),
	m_controlSource( CONTROL_COMPUTER ),
	m_computerControl( 0 ),
	m_userControl( 0 ),
	m_none( 0, 0, 0 ),
	m_weapon( 0 ),
	m_weaponInventory( Allocator<P(GameWeapon)>(__FILE__) ),
	m_currentWeapon( 0 ),
	m_invulnerable( false ),
	m_health( 100 ),
	m_hitBone( 0 ),
	m_timeSinceLastProjectile( 0  ),
	m_timeSinceLastTaunt( 0  ),
	m_worldAnim( 0 ),
	m_worldAnimTime( 0 )
{
	m_computerControl = new ComputerControl( vm, arch, this );
	m_userControl = new UserControl( vm, arch, this );

	m_methodBase = ScriptUtil<GameCharacter,GameObject>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
	setCollidable( true );

	// set vector sizes
	m_primaryStateListeners.setSize( PRIMARY_STATE_COUNT, 0 );
	m_secondaryStateListeners.setSize( SECONDARY_STATE_COUNT, 0 );

	// control pointers
	m_controlPtrTable["NONE"] = &m_none;
	m_controlPtrTable["MOVEMENT"] = &m_movementVector;
	m_controlPtrTable["MOVEMENTCONTROL"] = &m_movementControlVector;
	m_controlPtrTable["AIM"] = &m_aimVector;
	m_controlPtrTable["LOOK"] = &m_lookVector;
	m_controlPtrTable["AIMCROSSHAIR"] = &m_aimCrosshairNormalized;

	// check state names
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_UNKNOWN].name, "UNKNOWN" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_STANDING].name, "STANDING" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_WALKING].name, "WALKING" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_WALKING_BACKWARD].name, "WALKING_BACKWARD" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_STRAFING].name, "STRAFING" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_JUMPING].name, "JUMPING" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_ROLLING_FORWARD].name, "ROLLING_FORWARD" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_ROLLING_BACKWARD].name, "ROLLING_BACKWARD" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_ROLLING_LEFT].name, "ROLLING_LEFT" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_ROLLING_RIGHT].name, "ROLLING_RIGHT" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_CROUCHING_DOWN].name, "CROUCHING_DOWN" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_CROUCHED].name, "CROUCHED" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_CROUCHED_WALKING].name, "CROUCHED_WALKING" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_UNCROUCHING].name, "UNCROUCHING" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_PEEKING_LEFT].name, "PEEKING_LEFT" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_UNPEEKING_LEFT].name, "UNPEEKING_LEFT" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_PEEKING_RIGHT].name, "PEEKING_RIGHT" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_UNPEEKING_RIGHT].name, "UNPEEKING_RIGHT" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_FALLING].name, "FALLING" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_INAIR].name, "INAIR" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_PROJECTILE_HURT].name, "PROJECTILE_HURT" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_PHYSICAL_CONTACT_STRIKE].name, "PHYSICAL_CONTACT_STRIKE" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_PHYSICAL_KICK].name, "PHYSICAL_KICK" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_PHYSICAL_LOCK].name, "PHYSICAL_LOCK" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_PHYSICAL_HURT].name, "PHYSICAL_HURT" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_PHYSICAL_INLOCK].name, "PHYSICAL_INLOCK" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_DEAD].name, "DEAD" ) );
	assert( !strcmp( sm_primaryStates[PRIMARY_STATE_CUSTOM].name, "CUSTOM" ) );
	assert( sizeof(sm_primaryStates)/sizeof(sm_primaryStates[0]) == PRIMARY_STATE_COUNT );

	assert( !strcmp( sm_secondaryStates[SECONDARY_STATE_UNKNOWN].name, "UNKNOWN" ) );
	assert( !strcmp( sm_secondaryStates[SECONDARY_STATE_IDLING].name, "IDLING" ) );
	assert( !strcmp( sm_secondaryStates[SECONDARY_STATE_LOOKING].name, "LOOKING" ) );
	assert( !strcmp( sm_secondaryStates[SECONDARY_STATE_ATTACKING].name, "ATTACKING" ) );
	assert( !strcmp( sm_secondaryStates[SECONDARY_STATE_AIMING].name, "AIMING" ) );
	assert( !strcmp( sm_secondaryStates[SECONDARY_STATE_HOLDING_AIM].name, "HOLDING_AIM" ) );
	assert( !strcmp( sm_secondaryStates[SECONDARY_STATE_CHANGING_CLIP].name, "CHANGING_CLIP" ) );
	assert( !strcmp( sm_secondaryStates[SECONDARY_STATE_CYCLING_WEAPON].name, "CYCLING_WEAPON" ) );
	assert( !strcmp( sm_secondaryStates[SECONDARY_STATE_PHYSICAL_CONTACT_STRIKE].name, "PHYSICAL_CONTACT_STRIKE" ) );
	assert( !strcmp( sm_secondaryStates[SECONDARY_STATE_PHYSICAL_STRIKE].name, "PHYSICAL_STRIKE" ) );
	assert( !strcmp( sm_secondaryStates[SECONDARY_STATE_PHYSICAL_LOCK].name, "PHYSICAL_LOCK" ) );
	assert( !strcmp( sm_secondaryStates[SECONDARY_STATE_PROJECTILE_HURT_WHILE_AIMING].name, "PROJECTILE_HURT_WHILE_AIMING" ) );
	assert( !strcmp( sm_secondaryStates[SECONDARY_STATE_THROWING_EMPTY_SHELL].name, "THROWING_EMPTY_SHELL" ) );
	assert( !strcmp( sm_secondaryStates[SECONDARY_STATE_DEAD].name, "DEAD" ) );
	assert( sizeof(sm_secondaryStates)/sizeof(sm_secondaryStates[0]) == SECONDARY_STATE_COUNT );

	++s_characters;
	if ( !s_animNodes )
	{
		s_animNodes = new Vector<Node*>( Allocator<Node*>(__FILE__) );
		s_animNodeNames = new Vector<String>( Allocator<String>(__FILE__) );
	}
}

GameCharacter::~GameCharacter()
{
	if ( m_movementNoise )
	{
		m_noiseMgr->removeNoise( m_movementNoise );
		m_movementNoise = 0;
	}

	if ( m_weapon )
		m_weapon->setOwner( 0 );

	if ( --s_characters == 0 )
	{
		s_animNodes = 0;
		s_animNodeNames = 0;
	}
}

void GameCharacter::removeAnimationListeners()
{
	for ( int i = 0 ; i < m_animListeners.size() ; ++i )
		vm()->unref( m_animListeners[i].scriptFuncRef );
	m_animListeners.clear();
}

void GameCharacter::removeStateListeners()
{
	for ( int j = 0 ; j < m_primaryStateListeners.size() ; ++j )
	{
		if ( m_primaryStateListeners[j] )
		{
			for ( int i = 0 ; i < m_primaryStateListeners[j]->size() ; ++i )
				vm()->unref( (*m_primaryStateListeners[j])[i].scriptFuncRef );
		}
	}
	m_primaryStateListeners.clear();

	for ( int j = 0 ; j < m_secondaryStateListeners.size() ; ++j )
	{
		if ( m_secondaryStateListeners[j] )
		{
			for ( int i = 0 ; i < m_secondaryStateListeners[j]->size() ; ++i )
				vm()->unref( (*m_secondaryStateListeners[j])[i].scriptFuncRef );
		}
	}
	m_secondaryStateListeners.clear();
}

const ControlSector& GameCharacter::getMovementControlSector( int i ) const
{
	return m_movementControlSectors[i];
}

int GameCharacter::movementControlSectors() const
{
	return m_movementControlSectors.size();
}

GameCharacter::PrimaryState GameCharacter::findPrimaryState( const String& stateName )
{
	PrimaryState state = PRIMARY_STATE_UNKNOWN;

	for ( int i = 0 ; i < PRIMARY_STATE_COUNT ; ++i )
	{
		if ( stateName == sm_primaryStates[i].name )
		{
			state = (PrimaryState)i;
			break;
		}
	}

	if ( state == PRIMARY_STATE_UNKNOWN )
		throw ScriptException( Format("Invalid player state name ({0})", stateName) );
	return state;
}

GameCharacter::SecondaryState GameCharacter::findSecondaryState( const String& stateName )
{
	SecondaryState state = SECONDARY_STATE_UNKNOWN;

	for ( int i = 0 ; i < SECONDARY_STATE_COUNT ; ++i )
	{
		if ( stateName == sm_secondaryStates[i].name )
		{
			state = (SecondaryState)i;
			break;
		}
	}

	if ( state == SECONDARY_STATE_UNKNOWN )
		throw ScriptException( Format("Invalid player state name ({0})", stateName) );
	return state;
}

void GameCharacter::resetInputState()
{
	m_userControl->resetInputState();
	m_computerControl->resetInputState();
}

void GameCharacter::setInvulnerable( bool state ) 
{
	m_invulnerable = state;
}

void GameCharacter::aimAt( const Vector3& target )
{
	assert( m_mesh );

	Vector3 targettogun = target - aimCenter();
	rotation().inverse().rotate( targettogun, &m_aimVector );

	//float angx = Math::atan2( m_aimVector.x, m_aimVector.z );
	//float angy = Math::atan2( m_aimVector.y, m_aimVector.z );
	//if ( name() == "Hero" ) Debug::println( "angx={0}, angy={1}", Math::toDegrees(angx), Math::toDegrees(angy) );
}

void GameCharacter::lookTo( const Vector3& target )
{
	assert( m_mesh );

	Vector3 targettowaist = target - m_meshHeadBone->cachedWorldTransform().translation();
	rotation().inverse().rotate( targettowaist, &m_lookVector );
}

void GameCharacter::setCrosshair( const math::Vector2& pos ) 
{
	m_aimCrosshairNormalized.x = pos.x;
	m_aimCrosshairNormalized.y = pos.y;
	m_aimCrosshairNormalized.z = 0;
}

void GameCharacter::updateNoise( float /*dt*/ )
{
	if ( !m_movementNoise )
		m_movementNoise = m_noiseMgr->createNoise( this, 0.f, 0.f, 0.f, Float::MAX_VALUE, Float::MAX_VALUE );

	float noiseLevel = 0.f;
	float noiseDistance = 0.f;
	GameSurface* ground = groundMaterial();

	float ctrl = m_movementControlVector.length();
	if ( ground )
	{
		// default movement noise
		if ( ctrl > ControlBase::ZERO_CONTROL_VALUE )
		{
			if ( ctrl <= maxSneakingControl() )
			{
				noiseLevel = ground->getNumber( "sneakingNoiseLevel" );
				noiseDistance = ground->getNumber( "movementNoiseDistance" );
			}
			else if ( ctrl <= maxWalkingControl() )
			{
				noiseLevel = ground->getNumber( "walkingNoiseLevel" );
				noiseDistance = ground->getNumber( "movementNoiseDistance" );
			}
			else if ( ctrl <= maxRunningControl() )
			{
				noiseLevel = ground->getNumber( "runningNoiseLevel" );
				noiseDistance = ground->getNumber( "movementNoiseDistance" );
			}
		}

		// crouched walking -> sneaking noise
		if ( m_primaryState == PRIMARY_STATE_CROUCHED_WALKING )
		{
			noiseLevel = ground->getNumber( "sneakingNoiseLevel" );
			noiseDistance = ground->getNumber( "movementNoiseDistance" );
		}

		// rolling -> sneak noise
		if ( isRolling(m_primaryState) )
		{
			noiseLevel = ground->getNumber( "sneakingNoiseLevel" );
			noiseDistance = ground->getNumber( "movementNoiseDistance" );
		}
	}

	m_movementNoise->setNoiseLevel( noiseLevel );
	m_movementNoise->setFadeDistance( 0.f, noiseDistance );
	m_movementNoise->setPosition( cell(), position() );
}

void GameCharacter::update( float dt, bool firstUpdateInFrame )
{
	// update characters less frequently which are not in visible cell
	if ( !cell()->visible() || hidden() )
		return;

	GameObject::update( dt );

	if ( !m_worldAnim )
	{
		updateState( dt );

		if ( !isDead( m_primaryState ) )
		{	
			if ( m_controlSource == CONTROL_COMPUTER && m_computerControl->stateMachine()->states() > 0 )
			{
				// update AI's state machine
				m_computerControl->stateMachine()->update( dt );

				// Adjust look 
				m_computerControl->adjustLook( &m_lookVector, dt );

				// Listen to noise
				listenNoise();
			}
			else if ( this == level()->mainCharacter() )
			{
				updateNoise( dt );
			}

			updatePhysicalCombat( dt );
			updateWeapon( dt );
			updateVelocity( dt );
			updateTransform( dt, firstUpdateInFrame );
		}	
	}

	updateAnimationState( dt );

	m_timeSinceLastProjectile += dt;
}

GameCharacter::PrimaryState GameCharacter::evaluatePrimaryState()
{
	PrimaryState state = m_primaryState;

	if ( m_primaryStateTime >= getStateEndTime(state) && state != PRIMARY_STATE_DEAD )
	{
		if ( m_controlSource == CONTROL_USER )
			state = m_userControl->evaluatePrimaryState( m_primaryState );
		else if ( m_controlSource == CONTROL_COMPUTER )
			state = m_computerControl->evaluatePrimaryState( m_primaryState );

		if ( m_groundPlane == Vector4(0,0,0,0) )
		{
			if ( ++m_inAirCounter >= 10 )
				state = PRIMARY_STATE_INAIR;
		}
		else 
		{
			m_inAirCounter = 0;
		}
	}

	return state;
}

GameCharacter::SecondaryState GameCharacter::evaluateSecondaryState()
{
	SecondaryState state = m_secondaryState;

	if ( m_secondaryStateTime >= getStateEndTime(state) && m_primaryState != PRIMARY_STATE_DEAD )
	{
		if ( m_controlSource == CONTROL_USER )
			state = m_userControl->evaluateSecondaryState( m_secondaryState );
		else if ( m_controlSource == CONTROL_COMPUTER )
			state = m_computerControl->evaluateSecondaryState( m_secondaryState );
	}

	return state;
}


float GameCharacter::getStateEndTime( PrimaryState state ) const
{
	float end = 0.f;

	if ( m_primaryStateListeners[state] )
	{
		for ( int i = 0 ; i < m_primaryStateListeners[state]->size() ; ++i )
		{
			StateListener& listener = (*m_primaryStateListeners[state])[i];

			if ( listener.flags & STATE_WAIT )
			{
				float time = listener.time;
				if ( time > end )
					end = time;
			}
		}
	}

	return end;
}

float GameCharacter::getStateEndTime( SecondaryState state ) const
{
	float end = 0.f;

	if ( m_secondaryStateListeners[state] )
	{
		for ( int i = 0 ; i < m_secondaryStateListeners[state]->size() ; ++i )
		{
			StateListener& listener = (*m_secondaryStateListeners[state])[i];

			if ( listener.flags & STATE_WAIT )
			{
				float time = listener.time;
				if ( time > end )
					end = time;
			}
		}
	}

	return end;
}

void GameCharacter::signalStateListeners( PrimaryState state, int flagsMask, float timeStart, float timeEnd )
{
	if ( m_primaryStateListeners[state] )
	{
		VM* vm = this->vm();

		for ( int i = 0 ; i < m_primaryStateListeners[state]->size() ; ++i )
		{
			const StateListener& listener = (*m_primaryStateListeners[state])[i];

			if ( listener.time >= timeStart && 
				listener.time < timeEnd && 
				0 != (listener.flags & flagsMask) )
			{
				bool removeListener = 0 != (listener.flags & STATE_REMOVE);

				if ( !vm->getRef( listener.scriptFuncRef ) )
				{
					// listener is obsolete, no longer exist
					removeListener = true;
				}
				else
				{
					vm->pushTable( this );
					vm->call( 1, 0 );
				}

				if ( removeListener )
				{
					vm->unref( listener.scriptFuncRef );
					m_primaryStateListeners[state]->remove(i);
					--i;
					continue;
				}
			}
		}

		// remove all rest STATE_REMOVE listeners on STATE_EXIT
		if ( flagsMask & STATE_EXIT )
		{
			for ( int i = 0 ; i < m_primaryStateListeners[state]->size() ; ++i )
			{
				const StateListener& listener = (*m_primaryStateListeners[state])[i];
				if ( listener.flags & STATE_REMOVE )
				{
					vm->unref( listener.scriptFuncRef );
					m_primaryStateListeners[state]->remove(i);
					--i;
				}
			}
		}
	}
}

void GameCharacter::signalStateListeners( SecondaryState state, int flagsMask, float timeStart, float timeEnd )
{
	if ( m_secondaryStateListeners[state] )
	{
		VM* vm = this->vm();

		for ( int i = 0 ; i < m_secondaryStateListeners[state]->size() ; ++i )
		{
			const StateListener& listener = (*m_secondaryStateListeners[state])[i];

			if ( listener.time >= timeStart && 
				listener.time < timeEnd && 
				0 != (listener.flags & flagsMask) )
			{
				bool removeListener = 0 != (listener.flags & STATE_REMOVE);

				if ( !vm->getRef( listener.scriptFuncRef ) )
				{
					// listener is obsolete, no longer exist
					removeListener = true;
				}
				else
				{
					vm->pushTable( this );
					vm->call( 1, 0 );
				}

				if ( removeListener )
				{
					vm->unref( listener.scriptFuncRef );
					m_secondaryStateListeners[state]->remove(i);
					--i;
					continue;
				}
			}
		}

		// remove all rest STATE_REMOVE listeners on STATE_EXIT
		if ( flagsMask & STATE_EXIT )
		{
			for ( int i = 0 ; i < m_secondaryStateListeners[state]->size() ; ++i )
			{
				const StateListener& listener = (*m_secondaryStateListeners[state])[i];
				if ( listener.flags & STATE_REMOVE )
				{
					vm->unref( listener.scriptFuncRef );
					m_secondaryStateListeners[state]->remove(i);
					--i;
				}
			}
		}
	}
}

void GameCharacter::updateState( float dt )
{
	// primary state changed?
	PrimaryState state = evaluatePrimaryState();
	if ( state != m_primaryState )
		setPrimaryState( state );

	// signal ENTRY state listeners in range [m_stateTime,m_stateTime+dt)
	signalStateListeners( m_primaryState, STATE_ENTRY, m_primaryStateTime, m_primaryStateTime+dt );
	m_primaryStateTime += dt;

	// secondary state changed?
	SecondaryState state2 = evaluateSecondaryState();
	if ( state2 != m_secondaryState )
		setSecondaryState( state2 );

	// signal ENTRY state listeners in range [m_stateTime,m_stateTime+dt)
	signalStateListeners( m_secondaryState, STATE_ENTRY, m_secondaryStateTime, m_secondaryStateTime+dt );
	m_secondaryStateTime += dt;

}

void GameCharacter::setPrimaryState( PrimaryState state )
{
	PrimaryState old = m_primaryState;
	m_primaryStateTime = 0.f;
	m_primaryState = state;
	signalStateListeners( old, STATE_EXIT, 0.f, Float::MAX_VALUE );
}

void GameCharacter::setSecondaryState( SecondaryState state )
{
	SecondaryState old = m_secondaryState;
	m_secondaryStateTime = 0.f;
	m_secondaryState = state;
	signalStateListeners( old, STATE_EXIT, 0.f, Float::MAX_VALUE );

	if ( isHeadTurnFixEnabled(state) && !isHeadTurnFixEnabled(old) )
	{
		m_headTurnFixBlendTime = 0.f;
		m_headTurnFixBlendTimeDirection = +1;
	}
	else if ( !isHeadTurnFixEnabled(state) && isHeadTurnFixEnabled(old) )
	{
		m_headTurnFixBlendTime = m_headTurnFixBlendDelay;
		m_headTurnFixBlendTimeDirection = -1;		
	}
}

GameCharacter::PrimaryState GameCharacter::primaryState() const
{
	return m_primaryState;
}

GameCharacter::SecondaryState GameCharacter::secondaryState() const
{
	return m_secondaryState;
}

float GameCharacter::characterCollisionRadius() const
{
	return m_characterCollisionRadius;
}

void GameCharacter::applyMorphAnimations( Camera* camera )
{
	// check that character is not completely facing away from camera (like hero is most of the time)
	bool faceVisible = true;
	Matrix4x4 headtm = headWorldTransform();
	Vector3 forward( headtm(0,2), headtm(1,2), headtm(2,2) );
	Vector3 forwardInCamera = camera->cachedViewTransform().rotate( forward );
	forwardInCamera.y = 0.f;
	float len = forwardInCamera.length();
	if ( len > Float::MIN_VALUE )
	{
		forwardInCamera *= 1.f/len;

		float ang = Math::atan2( forwardInCamera.z, forwardInCamera.x );
		if ( ang < 0.f )
			ang += Math::PI * 2.f;

		faceVisible = 
			ang < Math::PI*.5f - CHARACTER_FACING_AWAY_FROM_CAMERA_ANGLE*.5f ||
			ang > Math::PI*.5f + CHARACTER_FACING_AWAY_FROM_CAMERA_ANGLE*.5f;
	}

	if ( faceVisible )
	{
		//Debug::println( "Applying morph animations ({0}) for {1}", m_morphAnims.size(), name() );
		for ( int i = 0 ; i < m_morphAnims.size() ; ++i )
		{
			MorphAnimation* morphAnim = m_morphAnims[i];
			morphAnim->apply();
		}
	}
}

MorphAnimation* GameCharacter::addMorphAnimation( const String& baseName, const String& animName, Interpolator::BehaviourType behaviour )
{
	const char funcName[] = "addMorphAnimation";
	P(MorphAnimation) morphAnim = new MorphAnimation( baseName, animName );

	// find output (=morph result) mesh
	Mesh* outputMesh = dynamic_cast<Mesh*>( NodeUtil::findNodeByName( m_mesh, baseName ) );
	if ( !outputMesh )
		throw ScriptException( Format("{0} excepts valid object name as second parameter, object {1} not found", funcName, baseName) );

	// find base model container
	Mesh* baseMesh = 0;
	for ( int i = 0 ; i < m_morphBases.size() ; ++i )
	{
		Mesh* mesh = m_morphBases[i];
		if ( mesh->name() == baseName )
		{
			baseMesh = mesh;
			break;
		}
	}

	// for each (material) morpher, find corresponding output model and base model
	P(ModelFile) modelFile = m_sceneMgr->getModelFile( animName, 0, 0, pix::Colorf(0,0,0), ModelFile::LOAD_MORPH );
	for ( int k = 0 ; k < modelFile->primitives() ; ++k )
	{
		P(Morpher) morpher = dynamic_cast<Morpher*>( modelFile->getPrimitive(k) );
		if ( morpher )
		{
			morpher = dynamic_cast<Morpher*>( morpher->clone( Primitive::SHARE_NOTHING ) );
			assert( morpher );

			// find match from base mesh primitives
			bool baseFound = false;
			for ( int i = 0 ; i < baseMesh->primitives() ; ++i )
			{
				Model* model = dynamic_cast<Model*>( baseMesh->getPrimitive(i) );
				if ( model && morpher->getTarget(0)->materialName() == model->shader()->name() )
				{
					// model matches morpher
					if ( !morpher->isValidBase(model) )
						throw Exception( Format("Function {0} excepts valid morpher, {1} has morph target vertices outside vertex range of base mesh {2}", funcName, animName, baseName ) );
					morpher->setBase( model );
					baseFound = true;
				}
			}
			if ( !baseFound )
				throw Exception( Format("Function {0} excepts valid morpher, {1} has materials not in the base mesh {2}", funcName, animName, baseName ) );

			// find match from output mesh primitives
			bool outputFound = false;
			for ( int i = 0 ; i < outputMesh->primitives() ; ++i )
			{
				Model* model = dynamic_cast<Model*>( outputMesh->getPrimitive(i) );
				if ( model && morpher->getTarget(0)->materialName() == model->shader()->name() )
				{
					// model matches morpher
					morpher->setOutput( model );
					outputFound = true;
				}
			}
			if ( !outputFound )
				throw Exception( Format("Function {0} excepts valid morpher, {1} has materials not in the output mesh {2}", funcName, animName, baseName ) );

			// set morpher end behaviour
			for ( int i = 0 ; i < morpher->targets() ; ++i )
			{
				VectorInterpolator* interp = morpher->getTargetWeightController(i);
				if ( interp )
					interp->setEndBehaviour( behaviour );
			}

			morphAnim->addMorpher( morpher );
		}
	}

	m_morphAnims.add( morphAnim );
	return morphAnim;
}

bool GameCharacter::hasMorphAnimation( const lang::String& animName ) const
{
	for ( int i = 0 ; i < m_morphAnims.size() ; ++i )
	{
		MorphAnimation* morphAnim = m_morphAnims[i];
		if ( morphAnim->name() == animName )
			return true;
	}
	return false;
}

void GameCharacter::playMorphAnimation( const String& animName )
{
	const char funcName[] = "playMorphAnimation";

	// find animation to activate
	MorphAnimation* newAnim = 0;
	for ( int i = 0 ; i < m_morphAnims.size() ; ++i )
	{
		MorphAnimation* morphAnim = m_morphAnims[i];
		if ( morphAnim->name() == animName )
		{
			newAnim = morphAnim;
			break;
		}
	}
	if ( !newAnim )
		throw ScriptException( Format("{0} expects valid morph animation name, {1} not found", funcName, animName) );

	// stop previous animations applied to the base mesh,
	// then start new animation
	for ( int i = 0 ; i < m_morphAnims.size() ; ++i )
	{
		MorphAnimation* morphAnim = m_morphAnims[i];
		if ( morphAnim->baseMeshName() == newAnim->baseMeshName() )
			morphAnim->stop();
	}

	newAnim->start( 0.f );
}

void GameCharacter::applyWorldSpaceAnimations( Camera* /*camera*/ )
{
	assert( m_worldAnim );
	assert( m_mesh );

	Node* meshBiped = NodeUtil::findNodeByName( m_mesh, "Bip01" );
	if ( !meshBiped )
		throw Exception( Format("Character {0} mesh {1} has no Bip01 bone", name(), m_mesh->name()) );
	
	String	animBipedName	= m_worldAnim->name();
	String	meshBipedName	= meshBiped->name();
	P(Node)	animRoot		= m_worldAnim->root();

	// collect animation nodes
	s_animNodes->clear();
	s_animNodeNames->clear();
	for ( Node* animNode = animRoot ; animNode ; animNode = animNode->nextInHierarchy() )
	{
		if ( animNode->name().startsWith(animBipedName) )
		{
			s_animNodes->add( animNode );
			String animNodeName = animNode->name().substring( animBipedName.length() );
			s_animNodeNames->add( animNodeName );
		}
	}

	// for each node in mesh
	//   apply state from world animation
	for ( Node* meshNode = m_mesh->nextInHierarchy() ; meshNode ; meshNode = meshNode->nextInHierarchy() )
	{
		if ( meshNode->name().startsWith(meshBipedName) )
		{
			String meshNodeName = meshNode->name().substring( meshBipedName.length() );

			Node* animNode = 0;
			for ( int i = 0 ; i < s_animNodes->size() ; ++i )
			{
				if ( s_animNodeNames->get(i) == meshNodeName )
				{
					animNode = s_animNodes->get(i);
					break;
				}
			}

			if ( !animNode )
			{
				Debug::printlnError( "Node {0} not found in animation {1}", meshNode->name(), m_worldAnim->root()->name() );
				//throw Exception( Format("Node {0} not found in animation {1}", meshNode->name(), m_worldAnim->root()->name()) );
			}
			else
			{
				//animNode->setState( m_worldAnimTime );
				meshNode->setTransform( animNode->transform() );
			}
		}
	}

	// update anim parent states
	//for ( Node* node = m_worldAnim->parent() ; node ; node = node->parent() )
	//	node->setState( m_worldAnimTime );

	// set root transform to anim biped bone world tm
	// and reset mesh biped transform
	Matrix4x4 parenttm = m_worldAnim->parent()->worldTransform();
	m_mesh->setTransform( parenttm );
	Matrix4x4 tm = m_worldAnim->worldTransform();
	const_cast<MovementState&>(movementState()).pos = tm.translation() - up();
}

void GameCharacter::applyTransformAnimations( Camera* /*camera*/ )
{
	assert( m_anims );
	assert( m_mesh );
	assert( !m_mesh->parent() );
	assert( m_primaryBlender );
	assert( m_secondaryBlender );

	// Check buffer size
	int animsrunning = m_primaryBlender->blends();
	// Get normalized blend result
	m_animParamBuffer.setSize( animsrunning );
	animsrunning = m_primaryBlender->getResult( m_animParamBuffer.size(), m_animParamBuffer.begin(), 0.f );

	Node* parent = 0;

	if ( animsrunning > 0 )
	{
		
		m_nodeBuffer.setSize( 0 );
		m_timeBuffer.setSize( 0 );
		m_weightBuffer.setSize( 0 );

		// Check for MASTER_CTRL objects
		for ( int i = 0; i < animsrunning; ++i )
		{
			Node* p = m_anims->getNode( m_animParamBuffer[i].name, "MASTER_CTRL" ); 
			if ( p )
			{
				m_weightBuffer.add( m_animParamBuffer[i].weight );
				m_timeBuffer.add( m_animParamBuffer[i].time );
				m_nodeBuffer.add( p );
			}
		}

		if ( m_nodeBuffer.size() > 0 )
		{
			parent = (Node*)m_nodeBuffer[0];
			parent->blendState( m_nodeBuffer.begin(), m_timeBuffer.begin(), m_weightBuffer.begin(), m_nodeBuffer.size() );
		}
		
		// Blend all bones
		m_nodeBuffer.setSize( animsrunning );
		m_timeBuffer.setSize( animsrunning );
		m_weightBuffer.setSize( animsrunning );

		for ( Node* node = m_mesh->nextInHierarchy() ; node ; node = node->nextInHierarchy() )
		{	
			String output("");

			for ( int i = 0; i < animsrunning ; ++i )
			{
				Node* bone = m_anims->getNode( m_animParamBuffer[i].name, node->name() );
		
				if ( bone )
				{
					m_nodeBuffer[i] = bone;
				}
				else
				{
	//				if ( node->name().indexOf("Bip") >= 0 )
	//					Debug::printlnWarning( "Bone {0} not found in animation set {1}", node->name(), m_animParamBuffer[i].name );
					m_nodeBuffer[i] = node;
				}

				m_weightBuffer[i] = m_animParamBuffer[i].weight;
				m_timeBuffer[i] = m_animParamBuffer[i].time;
			}

			node->blendState( m_nodeBuffer.begin(), m_timeBuffer.begin(), m_weightBuffer.begin(), animsrunning );		
		}
	}

	// Apply translation of MASTER_CTRL (if any) to mesh 
	if ( parent )
	{
		Vector3 rotatedoffset(0,0,0);
		m_mesh->rotation().rotate( parent->position(), &rotatedoffset );
		m_mesh->setPosition( m_mesh->position() + rotatedoffset );
	}

	// Check buffer size
	animsrunning = m_secondaryBlender->blends();
	m_animParamBuffer.setSize( animsrunning );

	// Get blend result, don't normalize if weight is below 1
	animsrunning = m_secondaryBlender->getResult( m_animParamBuffer.size(), m_animParamBuffer.begin(), 1.f );

	// store spine position before applying secondary animations but after applying primary animations
	// (this is needed for biped split to work correctly)
	Matrix4x4 primarySpineTm = m_meshSpineBone->transform();

	// secondary animations 
	if ( animsrunning > 0 )
	{
		for ( Node* node = m_mesh->nextInHierarchy() ; node ; node = node->nextInHierarchy() )
		{
			m_nodeBuffer.setSize(0);
			m_weightBuffer.setSize(0);
			m_timeBuffer.setSize(0);
			float weightsum = 0;

			for ( int i = 0; i < animsrunning; ++i )
			{
				SecondaryAnimationParams& anim = m_preparedSecondaryAnimations[m_animParamBuffer[i].name];

				// if node found in secondary anim hierarchy, add it to buffers
				if ( anim.bones.containsKey( node->name() ) )
				{
					m_weightBuffer.add( m_animParamBuffer[i].weight );
					m_timeBuffer.add( m_animParamBuffer[i].time );
					m_nodeBuffer.add( anim.bones[node->name()] );
					weightsum += m_animParamBuffer[i].weight;
				}
			}
			// If sum of weights is less than 1, also add current node state to buffers
			if ( weightsum < 1.f )
			{
				m_weightBuffer.add( 1.f - weightsum );
				m_timeBuffer.add( m_primaryTime );
				m_nodeBuffer.add( node );
			}
			
			// If node was found in any of the currecntly running secondary animations, apply blendstate
			if ( m_nodeBuffer.size() > 0 )
				node->blendState( m_nodeBuffer.begin(), m_timeBuffer.begin(), m_weightBuffer.begin(), m_nodeBuffer.size() );
		}

		// head bone turning rotation
		Matrix3x3 headBoneTurningRot = m_meshHeadBone->rotation();

		// visual mesh transform in world space
		Matrix4x4 roottm;
		roottm.setRow( 3, Vector4(0,0,0,1) );
		roottm.setTranslation( position() );
		roottm.setRotation( rotation() * CHARACTER_MESH_PRE_ROTATION );

		// align head bone to fix neck rotation inconsistencies in animations
		// convert head transform to character studio bone convention
		Matrix3x3 rot0 = m_meshNeckBone->worldTransform().rotation().inverse() * roottm.rotation();
		Matrix3x3 rot;
		rot.setColumn( 0, rot0.getColumn(1) );
		rot.setColumn( 1, -rot0.getColumn(2) );
		rot.setColumn( 2, -rot0.getColumn(0) );

		// blend between corrected bone and current bone
		Node correctedHeadBone;
		correctedHeadBone.setTransform( m_meshHeadBone->transform() );
		correctedHeadBone.setRotation( rot * m_headLookTransformFix * headBoneTurningRot );

		m_nodeBuffer.setSize(0);
		m_timeBuffer.setSize(0);
		m_weightBuffer.setSize(0);
		
		float weight = 0;
		if ( !isHurting( primaryState() ) )
		{
			if ( isHeadTurnFixEnabled( secondaryState() ) )
			{
				weight = m_headTurnFixBlendTime / m_headTurnFixBlendDelay;
			}
			else
			{
				if ( m_headTurnFixBlendTimeDirection > 0 ) 
					weight = 1;
				if ( m_headTurnFixBlendTimeDirection < 0 ) 
					weight = 0;
			}
		}

		m_nodeBuffer.add( m_meshHeadBone.ptr() );
		m_timeBuffer.add( 0.f );
		m_weightBuffer.add( 1.f - weight );
		
		m_nodeBuffer.add( &correctedHeadBone );
		m_timeBuffer.add( 0.f );
		m_weightBuffer.add( weight );

		m_meshHeadBone->blendState( m_nodeBuffer.begin(), m_timeBuffer.begin(), m_weightBuffer.begin(), m_nodeBuffer.size() );
	}
	
	// restore spine position to primary position
	// (this is needed for biped split to work correctly)
	m_meshSpineBone->setPosition( primarySpineTm.translation() );
}

void GameCharacter::signalAnimationListeners( const lang::String& animationName, float time, float dt )
{
	for ( int i = 0 ; i < m_animListeners.size() ; ++i )
	{
		AnimationListener& listener = m_animListeners[i];
		if ( listener.animationName == animationName )
		{
			Scene* root = dynamic_cast<Scene*>( m_anims->getGroup( animationName ) );
			if ( !root )
				throw Exception( Format("Animation {0} not found, tried to signal animation listeners", animationName) );
			
			float animEnd	= root->animationEnd();
			float t			= fmodf( time, animEnd );
			float t1		= fmodf( time+dt, animEnd );

			// handle time wrap
			if ( t1 < t )
				t = t1 - dt;

			if ( listener.time >= t && listener.time < t1 )
			{
				VM* vm = this->vm();
				if ( !vm->getRef( listener.scriptFuncRef ) )
				{
					// listener is obsolote, unref and remove from the list
					vm->unref( listener.scriptFuncRef );
					m_animListeners.remove(i);
					--i;
				}
				else
				{
					vm->pushTable( this );
					vm->call( 1, 0 );
				}
			}
		}
	}
}

void GameCharacter::updateMorphAnimations( float dt )
{
	for ( int i = 0 ; i < m_morphAnims.size() ; ++i )
	{
		P(MorphAnimation) morphAnim = m_morphAnims[i];
		if ( morphAnim->isRemovedAfterEnd() && (!morphAnim->active() || morphAnim->time() > morphAnim->endTime()) )
			m_morphAnims.remove( i-- );
	}
	for ( int i = 0 ; i < m_morphAnims.size() ; ++i )
	{
		MorphAnimation* morphAnim = m_morphAnims[i];
		morphAnim->update( dt );
	}
}

void GameCharacter::updateAnimationState( float dt )
{
	assert( m_anims );
	assert( m_mesh );
	assert( !m_mesh->parent() );
	assert( m_primaryBlender );
	assert( m_secondaryBlender );

	// update world animation
	if ( m_worldAnim )
		m_worldAnimTime += dt;

	updateMorphAnimations( dt );

	// update movement animation timers
	for ( HashtableIterator<String, P(MovementAnimation)> it = m_movementAnimations.begin(); it != m_movementAnimations.end(); ++it )
		it.value()->update( dt );	

	for ( HashtableIterator<String, P(MovementAnimation)> it = m_secondaryMovementAnimations.begin(); it != m_secondaryMovementAnimations.end(); ++it )
		it.value()->update( dt );	
	
	// apply primary movement animations
	if ( m_activeMovementAnimation != "" )
	{
		if ( !m_movementAnimations.containsKey(m_activeMovementAnimation) )
			throw Exception( Format( "Primary Movement Animation '{0}' not found!", m_activeMovementAnimation ).format() );
	
		// Output to blender
		m_movementAnimations[m_activeMovementAnimation]->outputToBlender();	

		// Signal animation listeners
		int animsrunning = m_primaryBlender->blends();
		m_nameBuffer.setSize( animsrunning );
		m_timeBuffer.setSize( animsrunning );
		m_weightBuffer.setSize( animsrunning );
		animsrunning = m_primaryBlender->getAnims( animsrunning, m_nameBuffer.begin(), m_timeBuffer.begin(), m_weightBuffer.begin() );
		int largest = 0;
		float largestweight = 0.f;
		for ( int i = 0; i < animsrunning; ++i )
		{
			if ( m_weightBuffer[i] > largestweight )
			{
				largest = i;
				largestweight = m_weightBuffer[i];
			}
		}
		if ( animsrunning > 0 )
			signalAnimationListeners( m_nameBuffer[largest], m_timeBuffer[largest], dt );
		else
		{
			Debug::printlnWarning( Format("0 anims in primary blender in character {0}, movement anim {1}", this->name(), m_activeMovementAnimation).format() );
		}
	}
	else if ( m_primaryAnimBlenderID != Blender::INVALIDID )
	{
		m_primaryTime += dt * m_primarySpeed;
		m_primaryBlender->setAnimTime( m_primaryAnimBlenderID, m_primaryTime ); 
		signalAnimationListeners( m_primaryBlender->getAnim( m_primaryAnimBlenderID ).name, m_primaryTime, dt * m_primarySpeed ); 
	}

	// do head turning blend time
	m_headTurnFixBlendTime += m_headTurnFixBlendTimeDirection * dt;
	if ( m_headTurnFixBlendTime > m_headTurnFixBlendDelay )
	{
		m_headTurnFixBlendTime = m_headTurnFixBlendDelay;
	}
	else if ( m_headTurnFixBlendTime < 0.f )
	{
		m_headTurnFixBlendTime = 0;
	}

	// apply secondary movement animations
	if ( m_activeSecondaryMovementAnimation != "" )
	{
		if ( !m_secondaryMovementAnimations.containsKey(m_activeSecondaryMovementAnimation) )
			throw Exception( Format( "Secondary Movement Animation '{0}' not found!", m_activeSecondaryMovementAnimation ).format() );
	
		// Output to blender
		m_secondaryMovementAnimations[m_activeSecondaryMovementAnimation]->outputToBlender();	

		// Signal animation listeners
		int animsrunning = m_secondaryBlender->blends();
		m_nameBuffer.setSize( animsrunning );
		m_timeBuffer.setSize( animsrunning );
		m_weightBuffer.setSize( animsrunning );
		animsrunning = m_secondaryBlender->getAnims( animsrunning, m_nameBuffer.begin(), m_timeBuffer.begin(), m_weightBuffer.begin() );
		int largest = 0;
		float largestweight = 0.f;
		for ( int i = 0; i < animsrunning; ++i )
		{
			if ( m_weightBuffer[i] > largestweight )
			{
				largest = i;
				largestweight = m_weightBuffer[i];
			}
		}
		if ( animsrunning > 0 )
			signalAnimationListeners( m_nameBuffer[largest], m_timeBuffer[largest], dt );
	}
	else if ( m_secondaryAnimBlenderID != Blender::INVALIDID )
	{
		m_secondaryTime += dt * m_secondarySpeed;
		m_secondaryBlender->setAnimTime( m_secondaryAnimBlenderID, m_secondaryTime ); 
		signalAnimationListeners( m_secondaryBlender->getAnim( m_secondaryAnimBlenderID ).name, m_secondaryTime, dt * m_secondarySpeed ); 
	}
	else
	{
		// secondary animations are disabled, disable head turning fixing
		m_headTurnFixBlendTime = 0;
	}

	// update blenders
	m_primaryBlender->update( dt );
	m_secondaryBlender->update( dt );
}

float GameCharacter::aimingTimeAfterShooting() const
{
	return m_aimingTimeAfterShooting;
}

bool GameCharacter::canTurn( PrimaryState state ) 
{
	return !isPeeking( state ) && !isJumping( state );
}

bool GameCharacter::isAttackable( PrimaryState state )
{
	return state != PRIMARY_STATE_DEAD;
}

bool GameCharacter::isHurting( PrimaryState state )
{
	return state == PRIMARY_STATE_FALLING ||
			state == PRIMARY_STATE_PROJECTILE_HURT ||
			state == PRIMARY_STATE_PHYSICAL_HURT;
}

bool GameCharacter::isStanding( PrimaryState state ) 
{
	return state == PRIMARY_STATE_STANDING;
}

bool GameCharacter::isWalking( PrimaryState state ) 
{
	return state == PRIMARY_STATE_WALKING || 
		state == PRIMARY_STATE_WALKING_BACKWARD;
}

bool GameCharacter::isControllable( PrimaryState state )
{
	return state != PRIMARY_STATE_FALLING &&
			state != PRIMARY_STATE_DEAD;
}

bool GameCharacter::isSignalsEnabled( PrimaryState state )
{
	return state != PRIMARY_STATE_CUSTOM;
}

bool GameCharacter::isJumping( PrimaryState state ) 
{
	return state == PRIMARY_STATE_JUMPING;
}

bool GameCharacter::isRolling( PrimaryState state ) 
{
	return state == PRIMARY_STATE_ROLLING_FORWARD ||
			state == PRIMARY_STATE_ROLLING_BACKWARD ||
			state == PRIMARY_STATE_ROLLING_LEFT ||
			state == PRIMARY_STATE_ROLLING_RIGHT;
}

bool GameCharacter::isCrouched( PrimaryState state ) 
{
	return state == PRIMARY_STATE_CROUCHING_DOWN || 
			state == PRIMARY_STATE_CROUCHED || 
			state == PRIMARY_STATE_CROUCHED_WALKING || 
			state == PRIMARY_STATE_UNCROUCHING;
}

bool GameCharacter::isInAir( PrimaryState state ) 
{
	return state == PRIMARY_STATE_INAIR || 
			state == PRIMARY_STATE_JUMPING;
}

bool GameCharacter::isFalling( PrimaryState state ) 
{
	return state == PRIMARY_STATE_FALLING;
}

bool GameCharacter::isStrafing( PrimaryState state ) 
{
	return state == PRIMARY_STATE_STRAFING;
}

bool GameCharacter::isAttacking( SecondaryState state ) 
{
	return state == SECONDARY_STATE_ATTACKING;
}

bool GameCharacter::isPhysicalAttacking( SecondaryState state ) 
{
	return state == SECONDARY_STATE_PHYSICAL_STRIKE ||
			state == SECONDARY_STATE_PHYSICAL_LOCK;
}

bool GameCharacter::isPhysicalAttacking( PrimaryState state ) 
{
	return state == PRIMARY_STATE_PHYSICAL_KICK ||
			state == PRIMARY_STATE_PHYSICAL_LOCK;
}

bool GameCharacter::isAiming( SecondaryState state ) 
{
	return state == SECONDARY_STATE_AIMING ||
			state == SECONDARY_STATE_HOLDING_AIM;
}

bool GameCharacter::isDead( PrimaryState state ) 
{
	return state == PRIMARY_STATE_DEAD;
}

bool GameCharacter::isHeadTurnFixEnabled( SecondaryState state )
{
	return state == SECONDARY_STATE_LOOKING || 
		state == SECONDARY_STATE_CYCLING_WEAPON;
}

bool GameCharacter::isPeeking( PrimaryState state ) 
{
	return state == PRIMARY_STATE_PEEKING_LEFT ||
			state == PRIMARY_STATE_UNPEEKING_LEFT ||
			state == PRIMARY_STATE_PEEKING_RIGHT ||
			state == PRIMARY_STATE_UNPEEKING_RIGHT;
}

void GameCharacter::updateVelocity( float dt )
{
	Vector3 vel = velocity();
	
	// walking
	if ( m_groundPlane != Vector4(0,0,0,0) )
	{
		Vector3 groundNormal( m_groundPlane.x, m_groundPlane.y, m_groundPlane.z );
		Vector3 groundForward = forward() - groundNormal*forward().dot(groundNormal);
		Vector3 groundRight = right() - groundNormal*right().dot(groundNormal);
		
		if ( m_controlSource == CONTROL_USER )
			m_userControl->adjustVelocity( dt, groundForward, groundRight, &vel, &m_movementVector, &m_movementControlVector );
		else if ( m_controlSource == CONTROL_COMPUTER )
			m_computerControl->adjustVelocity( &vel, &m_movementVector, &m_movementControlVector );
	}

	// gravity
	if ( m_groundPlane == Vector4(0,0,0,0) )
 		vel += Vector3(0,-9.8f,0) * m_gravity * dt;

	// falling friction
	if ( m_groundPlane == Vector4(0,0,0,0) )
		vel -= vel * (m_fallingFriction);

	// eliminate velocity against ground plane
	Vector3 gpnormal = Vector3(m_groundPlane.x, m_groundPlane.y, m_groundPlane.z);
	float vdotn = gpnormal.dot( vel );
	if ( vdotn < 0.f )
		vel -= gpnormal * vdotn;

	// eliminate velocity against slide plane
	vdotn = vel.dot( m_slideNormal );
	if ( vdotn < 0.f )
	{
		Vector3 vel0 = vel;
		vel = vel - m_slideNormal*vdotn;
		// make sure that we don't slide upwards
		if ( vel.y > vel0.y )
			vel.y = vel0.y; 
	}

	setVelocity( vel );
}

void GameCharacter::findSlidingPlane( const CollisionInfo& cinfo )
{
	dev::Profile pr( "character.findSlidingPlane" );
	//Vector3 oldSlideNormal = m_slideNormal;
	
	if ( cinfo.isCollision(CollisionInfo::COLLIDE_ALL) )
	{
		m_slideNormal = cinfo.normal();
	}
	else
	{
		// no collision, check if we still have the old sliding plane
		if ( m_slideNormal != Vector3(0,0,0) )
		{
			GameCell* oldcell = cell();
			Vector3 oldpos = position();
			CollisionInfo cinfo;
			move( m_slideNormal*-0.02f, &cinfo );
			if ( !cinfo.isCollision(CollisionInfo::COLLIDE_ALL) || cinfo.normal() != m_slideNormal )
				m_slideNormal = Vector3(0,0,0);
			setPosition( oldcell, oldpos );
		}
	}

	// DEBUG: print slide plane changes
	//if ( (oldSlideNormal-m_slideNormal).length() > 0.001f ) Debug::println( "Slide plane is {0} {1} {2}", m_slideNormal.x, m_slideNormal.y, m_slideNormal.z );
}

void GameCharacter::sampleGroundColor( const Vector3& point )
{
	// 4 point lightmap sampling
	dev::Profile pr( "character.sampleGroundColor" );

	// compute sample points
	const int NUM_SAMPLES = 4;
	Colorf c[NUM_SAMPLES];
	Vector3 cp[NUM_SAMPLES] = 
	{
		point + right() * .5f + up() *.5f,
		point - right() * .5f + up() *.5f,
		point + forward() * .5f + up() *.5f,
		point - forward()  * .5 + up() *.5f
	};

	// sample lightmap colors
	CollisionInfo cinfo;
	for ( int i = 0 ; i < NUM_SAMPLES ; ++i )
	{
		c[i] = m_groundLightmapColor;

		m_visibilityChecker->setPosition( cell(), cp[i] );
		m_visibilityChecker->move( -up(), &cinfo );

		if ( (cinfo.isCollision(CollisionInfo::COLLIDE_GEOMETRY_SEETHROUGH) || 
			cinfo.isCollision(CollisionInfo::COLLIDE_GEOMETRY_SOLID)) && cinfo.collisionCell() )
		{
			cinfo.bspTree()->getLightmapPixel( cinfo.point(), cinfo.polygon(), &c[i] );
		}
	}

	// compute average
	const float INV_NUM_SAMPLES = 1.f / NUM_SAMPLES;
	Colorf avg(0,0,0);
	for ( int i = 0 ; i < NUM_SAMPLES ; ++i )
		avg += c[i] * INV_NUM_SAMPLES;

	m_groundLightmapColor = avg;
}

void GameCharacter::findGroundPlane()
{
	dev::Profile pr( "character.findGroundPlane" );
	//Vector3 oldGroundNormal( m_groundPlane.x, m_groundPlane.y, m_groundPlane.z );
	
	GameCell* oldcell = cell();
	Vector3 oldpos = position();
	CollisionInfo cinfo;
	move( up()*-0.02f, &cinfo );
	if ( cinfo.isCollision(CollisionInfo::COLLIDE_ALL) && cinfo.normal().y > m_steepAngleCos )
	{
		Vector4 plane( cinfo.normal().x, cinfo.normal().y, cinfo.normal().z, -(cinfo.normal().dot(cinfo.point())) );
		m_groundPlane = plane;
		m_groundCollisionInfo = cinfo;
	}
	else
	{
		m_groundPlane = Vector4(0,0,0,0);
	}
	setPosition( oldcell, oldpos );

	// DEBUG: print ground plane changes
	//if ( (Vector3(m_groundPlane.x,m_groundPlane.y,m_groundPlane.z)-oldGroundNormal).length()>0.001f ) Debug::println( "Ground is {0} {1} {2} (cos limit angle={3})", m_groundPlane.x, m_groundPlane.y, m_groundPlane.z, m_steepAngleCos );
}

void GameCharacter::updateTransform( float dt, bool firstUpdateInFrame )
{
	// move position
	CollisionInfo cinfo;
	m_offset = Vector3(0,0,0);
	move( velocity()*dt, &cinfo );
	if ( m_offset.lengthSquared() > 0.f )
		moveWithoutColliding( m_offset );
	if ( firstUpdateInFrame || level()->mainCharacter() == this )
	{
		findSlidingPlane( cinfo );
		findGroundPlane();
	}

	// update rotation
	Matrix3x3 rot = rotation();
	float angleDelta = m_rotationSpeed * dt;
		
	if ( m_controlSource == CONTROL_USER )
		m_userControl->adjustRotation( &rot, angleDelta );
	else if ( m_controlSource == CONTROL_COMPUTER )
		m_computerControl->adjustRotation( &rot, m_rotationSpeed, dt );

	// align Y axis and orthonormalize
	rot(0,1) = rot(2,1) = 0.f;
	rot.setColumn( 0, rot.getColumn(1).cross(rot.getColumn(2)) );
	setRotation( rot.orthonormalize() );

	// update visual object transform
	Matrix4x4 roottm;
	roottm.setRow( 3, Vector4(0,0,0,1) );
	roottm.setTranslation( position() );
	roottm.setRotation( rotation() * CHARACTER_MESH_PRE_ROTATION );
	m_mesh->setTransform( roottm );
}

bool GameCharacter::readyToShoot() const
{
	// compute sum of aim/recoil animations
	const int MAX_ANIMS = 16;
	String names[MAX_ANIMS];
	float times[MAX_ANIMS];
	float weights[MAX_ANIMS];
	int n = m_secondaryBlender->getAnims( MAX_ANIMS, names, times, weights );
	float sumw = 0.f;

	// TODO: check in _DEBUG build that we have _aim_ and _shoot_ animations

	for ( int i = 0 ; i < n ; ++i )
	{
		if ( names[i].indexOf("_aim_") != -1 || names[i].indexOf("_shoot_") != -1 )
			sumw += weights[i];
	}

	return sumw > .99f;
}

void GameCharacter::updateWeapon( float dt ) 
{
	if ( m_controlSource == CONTROL_COMPUTER )
	{
		m_computerControl->updateAimDelay( dt );
	}

	if ( isAttacking( m_secondaryState ) )
	{
		if ( m_weapon )
		{
 			if ( m_weapon->ready() )
			{
				if ( m_controlSource == CONTROL_COMPUTER )
				{
					// Adjust aim for inaccuracy
					m_computerControl->adjustAim( &m_aimVector, dt );
				}

				Vector3 launchVector;
				rotation().rotate( m_aimVector, &launchVector );
				m_weapon->fire( this, launchVector ); 

				m_secondaryStateTime = 0.f;

				if ( !m_weapon->clipEmpty() )
				{
					pushMethod( "signalShoot" );
					call( 0, 0 );
				}
			}
		}
	}
}

void GameCharacter::cycleWeapon()
{
	if ( m_weapon )
	{
		m_weapon->setPosition( 0, Vector3(0,0,0) );
	}
	m_currentWeapon++;
	if ( m_currentWeapon >= m_weaponInventory.size() )
		m_currentWeapon = 0;

	setWeapon( m_weaponInventory[ m_currentWeapon ] );
}

void GameCharacter::updatePhysicalCombat( float dt ) 
{
	m_timeSinceLastTaunt += dt;

	// Check physical attacks for hits
	if ( m_currentPhysicalAttack != "" && !isHurting( primaryState() ) && !isDead( primaryState() ) )
	{
		PhysicalCombatMove* move = m_physicalCombatMoves[m_currentPhysicalAttack];

		// Test hit cylinders of characters in cell against character's strike cylinder
		GameLevel* level = this->level();
		for ( int i = 0 ; i < level->characters() ; ++i ) 
		{
			GameCharacter* otherCharacter = level->getCharacter(i);

			if ( otherCharacter != this && 
				(otherCharacter->position()-position()).lengthSquared() < CHARACTER_WAY_TOO_FAR_FOR_CLOSE_COMBAT*CHARACTER_WAY_TOO_FAR_FOR_CLOSE_COMBAT &&
				move->start() < m_physicalAttackTimer && m_physicalAttackTimer < move->end() && 
				!move->hasHit() && 
				!otherCharacter->isHurting( otherCharacter->primaryState() ) &&
				!otherCharacter->isDead( otherCharacter->primaryState() ) )
			{
				Vector3 otherInThisSpace = transform().inverse().transform( otherCharacter->position() );
				if ( move->isInAttackSector(otherInThisSpace) )
				{
					float thisCylinderHeight = (this->headWorldPosition() + Vector3( 0, 0.2f, 0 ) - this->position()).length();
					float otherCylinderHeight = (otherCharacter->headWorldPosition() + Vector3( 0, 0.2f, 0 ) - otherCharacter->position()).length();

					if ( Intersection::testVerticalCylinderCylinder( this->position() + Vector3(0,move->attackHeight(),0), thisCylinderHeight - move->attackHeight(), move->attackReachDistance(), 
						otherCharacter->position(), otherCylinderHeight, otherCharacter->hitRange() ) )
					{	
						// Call receive hit 
						otherCharacter->receivePhysicalStrike( this->position() + Vector3(0,move->attackHeight(),0), move->forceVector() ); 
						move->setHit( true );

						// Taunt enemy
						pushMethod( "signalTaunt" );
						vm()->pushNumber( m_timeSinceLastTaunt );
						call( 1, 0 );
						m_timeSinceLastTaunt = 0;
					}
				}
			}
		}
	}
	m_physicalAttackTimer += dt;
}

void GameCharacter::setWeapon( GameWeapon* weapon ) 
{
	if ( weapon && weapon->owner() )
		throw Exception( Format( "Can not assign weapon '{0}' to {1} because it is already owned by {2}", weapon->name(), this->name(), weapon->owner()->name() ) );

	if ( m_weapon )
	{
		m_weapon->setOwner( 0 );
	}

	m_weapon = weapon;

	if ( m_weapon )
	{
		m_weapon->setOwner( this );
	}
}

void GameCharacter::setControlSource( ControlSource controllingParty ) 
{
	m_controlSource = controllingParty;
}

void GameCharacter::signalReceiveProjectileFSM( GameProjectile* projectile )
{
	if ( m_controlSource == CONTROL_COMPUTER && m_computerControl->stateMachine()->states() > 0 )
	{
		// Call statemachine current state signal for received projectile
		Table currentState = m_computerControl->stateMachine()->stateTable();
		if ( !currentState.isNil( "signalReceiveProjectile" ) )
		{
			currentState.pushMember( "signalReceiveProjectile" );
			vm()->pushTable( currentState );
			vm()->pushTable( projectile );
			vm()->call( 2, 0 );
		}

		// Call statemachine next state signal for received projectile
		if ( m_computerControl->stateMachine()->nextState() )
		{
			Table nextState = m_computerControl->stateMachine()->nextStateTable();

			if ( !nextState.isNil( "signalReceiveProjectile" ) )
			{
				nextState.pushMember( "signalReceiveProjectile" );
				vm()->pushTable( nextState );
				vm()->pushTable( projectile );
				vm()->call( 2, 0 );
			}
		}
	}
}

void GameCharacter::listenNoise()
{
	assert( m_controlSource == CONTROL_COMPUTER );

	if ( level()->isActiveCutScene() )
		return;

	const float	hearingLimit = computerControl()->getNumber( "hearingLimit" );

	for ( int i = 0 ; i < m_noiseMgr->noises() ; ++i )
	{
		GameNoise* noise = m_noiseMgr->getNoise(i);

		if ( noise->getLevelAt(position()) > hearingLimit )
		{
			//Vector3 noiseInCharSpace = transform().inverse().transform( noise->position() );
			//float angle = Math::toDegrees( Math::atan2( noiseInCharSpace.z, noiseInCharSpace.x ) );
			//Debug::println( "Character {0} heard noise in direction {1} degrees, distance {2}", name(), angle, noiseInCharSpace.length() );
			signalHearNoiseFSM( noise );
		}
	}
}

void GameCharacter::signalHearNoiseFSM( GameNoise* noise )
{
	assert( m_controlSource == CONTROL_COMPUTER );

	if ( m_controlSource == CONTROL_COMPUTER && m_computerControl->stateMachine()->states() > 0 )
	{
		// Call statemachine current state signal for heard noise
		Table currentState = m_computerControl->stateMachine()->stateTable();
		if ( !currentState.isNil( "signalHearNoise" ) )
		{
			currentState.pushMember( "signalHearNoise" );
			vm()->pushTable( currentState );
			vm()->pushTable( noise );
			vm()->call( 2, 0 );
		}

		// Call statemachine next state signal for heard noise
		if ( m_computerControl->stateMachine()->nextState() )
		{
			Table nextState = m_computerControl->stateMachine()->nextStateTable();

			if ( !nextState.isNil( "signalHearNoise" ) )
			{
				nextState.pushMember( "signalHearNoise" );
				vm()->pushTable( nextState );
				vm()->pushTable( noise );
				vm()->call( 2, 0 );
			}
		}
	}
}

void GameCharacter::receiveProjectile( GameProjectile* projectile, const Vector3& collisionPoint, 
	const BoneCollisionBox& /*collisionBone*/ ) 
{
	// Collision point in character space
	Vector3 charCollisionPoint = transform().inverse().transform( collisionPoint );

	// Call character signal
	pushMethod( "signalReceiveProjectile" );
	vm()->pushTable( projectile );
	vm()->pushNumber( collisionPoint.x );
	vm()->pushNumber( collisionPoint.y );
	vm()->pushNumber( collisionPoint.z );
	vm()->pushNumber( m_timeSinceLastProjectile );
	call( 1+4, 0 );

	m_timeSinceLastProjectile = 0;

	// Call AI signal
	if ( !isDead( m_primaryState) )
		signalReceiveProjectileFSM( projectile );

}

GameCharacter::StillAlive GameCharacter::receiveDamage( float damage )
{
	float oldhealth = m_health;

	if ( !isDead( m_primaryState ) )
	{
		if ( !m_invulnerable )
			m_health -= damage;

		if ( m_health <= 0.f )
		{
			Debug::println( "{0} is dead", name() );
			setPrimaryState( PRIMARY_STATE_DEAD );
			setSecondaryState( SECONDARY_STATE_DEAD );
			m_activeMovementAnimation = "";
			m_activeSecondaryMovementAnimation = "";
			return CHARACTER_DIED;
		}

		// signal health changes back to character...
		if ( hasMethod("signalHealthChanged") )
		{
			pushMethod( "signalHealthChanged" );
			vm()->pushNumber( m_health );
			vm()->pushNumber( oldhealth );
			call( 2, 0 );
		}
		return CHARACTER_ALIVE;
	}
	return CHARACTER_ALREADYDEAD;
}

void GameCharacter::receivePhysicalStrike( const Vector3& from, const Vector3& force ) 
{
	// Sector of hit
	int sector = 0;

	float angle = Math::acos( Vector3(0,0,1).dot( force.normalize() ) );
	angle = force.x < 0.f ? -angle : angle;

	Vector3 localReceiveVector = transform().rotation().inverse().rotate((Vector3(from.x, position().y, from.z) - position())).normalize();
	localReceiveVector.rotate( Vector3(0,1,0), angle );

	if ( Math::abs( localReceiveVector.x ) > Math::abs( localReceiveVector.z ) )
	{
		// Left or right
		if ( localReceiveVector.x > 0.f ) 
			sector = 1;
		else
			sector = 3;
	}
	else
	{
		// Front or back
		if ( localReceiveVector.z > 0.f )
			sector = 0;
		else
			sector = 2;
	}

	// other hit parameters
	float height = from.y - position().y;
	float damage = force.length();

	// Reduce height of hit when crouched
	if ( GameCharacter::isCrouched( m_primaryState ) )
	{
		height -= 0.5f;
	}

	// Call character signal
	pushMethod( "signalReceivePhysicalAttack" );
	vm()->pushNumber( (float)sector );
	vm()->pushNumber( height );
	vm()->pushNumber( damage );
	call( 3, 0 );
}

bool GameCharacter::invulnerable() const 
{
	return m_invulnerable;
}

GameWeapon*	GameCharacter::weapon() const 
{
	assert( m_weapon );
	return m_weapon;
}

bool GameCharacter::hasWeapon() const 
{
	return m_weapon != 0;
}

UserControl* GameCharacter::userControl() const 
{
	return m_userControl;
}

ComputerControl* GameCharacter::computerControl() const 
{
	return m_computerControl;
}

void GameCharacter::setShaderParams( Shader* fx, Mesh* mesh )
{
	Matrix4x4 worldTmInv = mesh->cachedWorldTransform().inverse();

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
		else if ( desc.name == "pLight1" )
		{
			Light* lt = keylight();
			if ( lt )
				fx->setVector4( desc.name, lt->worldTransform().getColumn(3) );
		}
	}
}

void GameCharacter::updateShaderParameters()
{
	m_mesh->validateHierarchy();

	// set shader parameters
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
					setShaderParams( fx, mesh );
			}
		}
	}
}

Vector3 GameCharacter::headWorldPosition() const
{
	return getBoneWorldPosition( m_meshHeadBone );
}

Matrix4x4 GameCharacter::headWorldTransform() const
{
	Matrix4x4 tm = getBoneWorldTransform( m_meshHeadBone );

	// convert to Z forward, X right, Y up -format
	Matrix3x3 rot0 = tm.rotation();
	Matrix3x3 rot1;
	rot1.setColumn( 0, rot0.getColumn(2) );
	rot1.setColumn( 1, rot0.getColumn(0) );
	rot1.setColumn( 2, rot0.getColumn(1) );
	tm.setRotation( rot1 );

	return tm;
}

Vector3 GameCharacter::getBoneWorldPosition( Node* bone ) const
{
	if ( visible() || m_worldAnim )
		return bone->worldTransform().translation();

	return bone->worldTransform().translation();
/*
	// get name of the most signifigant animation
	String names[64];
	float times[64];
	float weights[64];
	int blends = m_primaryBlender->getAnims( 64, names, times, weights );
	if ( blends == 0 )
		return bone->worldTransform().translation();
		//throw Exception( Format("Primary blender has no animations in character {0}\nblends = {1}", name(), m_primaryBlender->blends()) );
	assert( blends < 64 && blends >= 1 );

	String animname;
	float maxw = 0.f;
	float animtime = 0.f;
	int maxindex = 0;
	for ( int i = 0 ; i < blends ; ++i )
	{
		if ( weights[i] > maxw )
		{
			maxw = weights[i];
			maxindex = i;
		}
	}
	animname = names[maxindex];
	animtime = times[maxindex];
	assert( animname != "" );

	// get node of the most signifigant animation
	Node* animBone = m_anims->getNode( animname, bone->name() );
	if ( !animBone )
		throw Exception( Format("Bone {0} not found in animation {1} for character {2}", bone->name(), animname, this->name()) );

	// compute world position
	Vector3 pos(0,0,0);
	Node* n = bone;
	for ( Node* a = animBone ; ; )
	{
		anim::Animatable* anims = a;
		const float w = 1.f;
		n->blendState( &anims, &animtime, &w, 1 );
		pos = n->transform().transform( pos );

		n = n->parent();
		a = a->parent();
		if ( !a->parent() || !n->parent() )
			break;
	}
	
	// rotate positions 180 degrees about Y
	// (to correct 'wrong' character direction in exported data)
	pos = Vector3( -pos.x, pos.y, -pos.z );
	return transform().transform( pos );
*/
}

Matrix4x4 GameCharacter::getBoneWorldTransform( Node* bone ) const
{
	if ( visible() || m_worldAnim )
		return bone->worldTransform();

	return bone->worldTransform();
/*
	// get name of the most signifigant animation
	String names[64];
	float times[64];
	float weights[64];
	int blends = m_primaryBlender->getAnims( 64, names, times, weights );
	if ( blends == 0 )
		return bone->worldTransform();
		//throw Exception( Format("Primary blender has no animations in character {0}\nblends = {1}", name(), m_primaryBlender->blends()) );
	assert( blends < 64 && blends >= 1 );

	String animname;
	float maxw = 0.f;
	float animtime = 0.f;
	int maxindex = 0;
	for ( int i = 0 ; i < blends ; ++i )
	{
		if ( weights[i] > maxw )
		{
			maxw = weights[i];
			maxindex = i;
		}
	}
	animname = names[maxindex];
	animtime = times[maxindex];
	assert( animname != "" );

	// get head/feet nodes of the most signifigant animation
	Node* animBone = m_anims->getNode( animname, bone->name() );
	if ( !animBone )
		throw Exception( Format("Bone {0} not found in animation {1}", bone->name(), animname) );

	// compute world position
	Matrix4x4 tm(1.f);
	Node* n = bone;
	for ( Node* a = animBone ; ; )
	{
		anim::Animatable* anims = a;
		const float w = 1.f;
		n->blendState( &anims, &animtime, &w, 1 );
		tm = n->transform() * tm;

		n = n->parent();
		a = a->parent();
		if ( !a->parent() || !n->parent() )
			break;
	}

	Matrix4x4 roottm;
	roottm.setRow( 3, Vector4(0,0,0,1) );
	roottm.setTranslation( position() );
	roottm.setRotation( rotation() * CHARACTER_MESH_PRE_ROTATION );
	return roottm * tm;
*/
}

bool GameCharacter::isVisibleLine( const Vector3& worldPos, const Vector3& start, const Vector3& end, int firstSampleIndex, int samples ) const
{
	// check if character is occluded by level geometry
	m_visibilityChecker->setPosition( cell(), position() );
	m_visibilityChecker->moveWithoutColliding( start-m_visibilityChecker->position() );
	Vector3 startpos = m_visibilityChecker->position();
	GameCell* startcell = m_visibilityChecker->cell();
	Vector3 delta = end - start;
	bool visible = false;
	for ( int i = firstSampleIndex ; i < samples ; ++i )
	{
		float u = (float)i / (float)(samples-1);
		m_visibilityChecker->setPosition( startcell, startpos );
		m_visibilityChecker->moveWithoutColliding( delta*u );

		CollisionInfo cinfo;
		m_visibilityChecker->move( worldPos-m_visibilityChecker->position(), &cinfo );
		if ( !cinfo.isCollision(CollisionInfo::COLLIDE_ALL) )
		{
			visible = true;
			break;
		}
	}
	return visible;
}

bool GameCharacter::isVisiblePoint( const Vector3& worldPos, const Vector3& point ) const
{
	// check if character is occluded by level geometry
	m_visibilityChecker->setPosition( cell(), position() );
	m_visibilityChecker->moveWithoutColliding( point-m_visibilityChecker->position() );
	CollisionInfo cinfo;
	m_visibilityChecker->move( worldPos-m_visibilityChecker->position(), &cinfo );
	return !cinfo.isCollision( CollisionInfo::COLLIDE_ALL );
}

bool GameCharacter::isVisibleFrom( const Vector3& worldPos ) const
{
	dev::Profile pr( "character.isVisibleFrom" );

	Vector3 rightFoot = getBoneWorldPosition( m_meshLeftFootBone );
	Vector3 head = getBoneWorldPosition( m_meshHeadBone ) + up()*(boundSphere()*.5f);
	Vector3 leftUpperArm = getBoneWorldPosition( m_meshLeftUpperArmBone );
	Vector3 rightUpperArm = getBoneWorldPosition( m_meshRightUpperArmBone );
	return isVisiblePoint(worldPos,head) ||
		isVisiblePoint(worldPos,leftUpperArm) ||
		isVisiblePoint(worldPos,rightUpperArm) ||
		isVisiblePoint(worldPos,rightFoot);
/*
	// find character endpoints
	Vector3 leftFoot = getBoneWorldPosition( m_meshLeftFootBone );
	Vector3 rightFoot = getBoneWorldPosition( m_meshLeftFootBone );
	Vector3 head = getBoneWorldPosition( m_meshHeadBone );
	Vector3 leftForeArm = getBoneWorldPosition( m_meshLeftForeArmBone );
	Vector3 rightForeArm = getBoneWorldPosition( m_meshRightForeArmBone );

	return isVisibleLine( worldPos, head, leftFoot, 0, 3 ) || 
		isVisibleLine( worldPos, head, rightFoot, 1, 3 ) ||
		isVisibleLine( worldPos, leftForeArm, rightForeArm, 0, 2 );
*/
}

bool GameCharacter::isTorsoVisibleFrom( const Vector3& worldPos ) const
{
	dev::Profile pr( "character.isTorsoVisibleFrom" );

	Vector3 head = getBoneWorldPosition( m_meshHeadBone );
	Vector3 leftUpperArm = getBoneWorldPosition( m_meshLeftUpperArmBone );
	Vector3 rightUpperArm = getBoneWorldPosition( m_meshRightUpperArmBone );
	
	return isVisiblePoint( worldPos, head ) ||
		isVisiblePoint( worldPos, leftUpperArm ) ||
		isVisiblePoint( worldPos, rightUpperArm );
		//isVisiblePoint( worldPos, spine );
}

Node* GameCharacter::getRenderObject( Camera* camera )
{
	if ( hidden() )
		return 0;

	GameObject::getRenderObject( camera );

	if ( m_mesh && camera )
	{
		if ( m_worldAnim )
			applyWorldSpaceAnimations( camera );

		// select LOD level
		m_lod->setRadius( 1.f );
		if ( !m_lod->selectLOD(center(), camera) )
			return 0;

		// occlusion culling
		if ( level()->mainCharacter() != this && !m_worldAnim )
		{
			const Matrix4x4& camtm = camera->cachedWorldTransform();
			bool visible = isVisibleFrom( camtm.translation() );
			if ( !visible )
				return 0;
		}

		// NOTE !! applyTransformAnimations modifies transformations of m_mesh hierarchy!
		if ( !m_worldAnim )
			applyTransformAnimations( camera );

		// apply morph animations if highest LOD level in use
		if ( m_lod->level() == 0 )
			applyMorphAnimations( camera );

		// if have weapon, set weapon transform
		if ( m_weapon )
		{
			Node* hand = NodeUtil::findNodeByName( m_mesh, "Bip01 R Hand" );
			if ( !hand )
				throw Exception( Format("Bip01 R Hand not found in {0}", m_mesh->name()) );

			m_weapon->mesh()->setPosition( hand->worldTransform().translation() );
			m_weapon->mesh()->setRotation( hand->worldTransform().rotation() 
												* Matrix3x3( Vector3(0,1,0), -Math::PI * .5f ) 
												* Matrix3x3( Vector3(0,0,1), Math::PI * .5f ) );
		}

		sampleGroundColor( position() );
		updateShaderParameters();

		// update root collision box
		Matrix4x4 invtm = m_mesh->cachedWorldTransform().inverse();
		m_rootCollisionBox.setBox( Vector3(0,0,0), Vector3(0,0,0) );
		for ( int i = 0 ; i < m_boneCollisionBoxes.size() ; ++i )
		{
			const BoneCollisionBox& box = m_boneCollisionBoxes[i];
			Matrix4x4 tm = invtm * box.bone()->cachedWorldTransform();
			m_rootCollisionBox.mergePoint( tm.transform(box.boxMin()) );
			m_rootCollisionBox.mergePoint( tm.transform(box.boxMax()) );
		}
	}
	return m_mesh;
}

String GameCharacter::stateString() const
{
	assert( m_primaryState >= 0 && m_primaryState < PRIMARY_STATE_COUNT );
	assert( m_secondaryState >= 0 && m_secondaryState < SECONDARY_STATE_COUNT );
	return Format("primary {0} (t={2,#.00}), secondary {1} (t={3,#.00})", sm_primaryStates[m_primaryState].name, sm_secondaryStates[m_secondaryState].name, m_primaryStateTime, m_secondaryStateTime).format();
}

bool GameCharacter::morphValid() const
{
	int activeMorphs = 0;
	int activeMorphBlendTargets = 0;
	int activeMorphBlendSources = 0;
	for ( int i = 0 ; i < m_morphAnims.size() ; ++i )
	{
		MorphAnimation* anim = m_morphAnims[i];
		if ( anim->active() && !anim->isBlendTarget() )
			++activeMorphs;
		if ( anim->isBlendTarget() )
			++activeMorphBlendTargets;
		if ( anim->isBlendSource() )
			++activeMorphBlendSources;
	}
	assert( activeMorphs <= 1 );
	assert( activeMorphBlendTargets <= 1 );
	assert( activeMorphBlendSources <= 1 );
	return activeMorphs <= 1 && activeMorphBlendTargets <= 1 && activeMorphBlendSources <= 1;
}

String GameCharacter::morphStateString() const
{
	assert( morphValid() );

	for ( int i = 0 ; i < m_morphAnims.size() ; ++i )
	{
		MorphAnimation* anim = m_morphAnims[i];
		if ( anim->active() && !anim->isBlendTarget() )
		{
			if ( anim->isBlendSource() )
				return Format( "{0}-{1} ({2})", anim->name(), anim->blendTarget()->name(), anim->blendPhase() ).format();
			else
				return Format( "{0} (t={1,#.00}, t.end={2,#.00})", anim->name(), anim->time(), anim->endTime() ).format();
		}
	}
	return "none";
}

String GameCharacter::animationPrimaryStateString() const
{
	return m_primaryBlender->stateString(true);
}

String GameCharacter::animationSecondaryStateString() const
{
	return m_secondaryBlender->stateString();
}

String GameCharacter::inputStateString() const
{
	if ( m_controlSource == CONTROL_USER )
		return Format( "acc={0,#.##} / back={1,#.##} / lft={2,#.##} / rgt={3,#.##} / strfrgt = {4,#.##} / strflft = {5,#.##} / fire={6}", 
					   m_userControl->accelerating(), m_userControl->acceleratingBackwards(), m_userControl->turningRight(), 
					   m_userControl->turningLeft(), m_userControl->strafingRight(), m_userControl->strafingLeft(), 
					   m_userControl->attacking() ).format();
	else
		return "";
}

Vector3 GameCharacter::center() const
{
	return position() + up() * boundSphere();
}

Vector3 GameCharacter::aimCenter() const
{
	if ( m_weapon )
		return m_weapon->launchPosition();
	else
		return position() + up() * 1.7f;
}

Vector3 GameCharacter::hitCenter() const 
{
	if ( m_hitBone )
		return m_hitBone->worldTransform().translation();
	else
		return position() + up() * 1.5f;
}

const math::Vector3& GameCharacter::aimVector() const
{
	return m_aimVector;
}

const math::Vector3& GameCharacter::lookVector() const
{
	return m_lookVector;
}

float GameCharacter::crouchWalkingSpeed() const 
{
	return m_crouchWalkingSpeed;
}

float GameCharacter::crouchStrafingSpeed() const 
{
	return m_crouchStrafingSpeed;
}

float GameCharacter::crouchBackwardSpeed() const 
{
	return m_crouchBackwardSpeed;
}

float GameCharacter::rollingSpeedForward() const 
{
	return m_rollingSpeedForward;
}

float GameCharacter::rollingSpeedBackward() const 
{
	return m_rollingSpeedBackward;
}

float GameCharacter::rollingSpeedSideways() const 
{
	return m_rollingSpeedSideways;
}

float	GameCharacter::health() const 
{
	return m_health;
}

float GameCharacter::primaryStateTime() const 
{
	return m_primaryStateTime;
}

float GameCharacter::secondaryStateTime() const 
{
	return m_secondaryStateTime;
}

const BoneCollisionBox& GameCharacter::getBoneCollisionBox( int i ) const
{
	return m_boneCollisionBoxes[i];
}

const BoneCollisionBox& GameCharacter::getBoneCollisionBox( const String& boneName ) const
{
	for ( int i = 0 ; i < m_boneCollisionBoxes.size() ; ++i )
	{
		if ( m_boneCollisionBoxes[i].bone()->name() == boneName )
			return m_boneCollisionBoxes[i];
	}
	throw Exception( Format("Bone {0} has no collision box in character {1}", boneName, name()) );
	return m_boneCollisionBoxes[0];
}

int GameCharacter::boneCollisionBoxes() const
{
	return m_boneCollisionBoxes.size();
}

float GameCharacter::animLength( const lang::String& animationName ) const
{
	Node* root = m_anims->getGroup( animationName );
	float animlength = 0.f;
	float nodelength;

	for ( Node* node = root ; node ; node = node->nextInHierarchy() )
	{
		Interpolator* posInterpl = dynamic_cast<Interpolator*>(node->positionController());
		if ( posInterpl )
		{
			nodelength = posInterpl->getKeyTime( posInterpl->keys() - 1 );
			if ( nodelength > animlength )
				animlength = nodelength;
		}
		Interpolator* rotInterpl = dynamic_cast<Interpolator*>(node->rotationController());
		if ( rotInterpl )
		{
			nodelength = rotInterpl->getKeyTime( rotInterpl->keys() - 1 );
			if ( nodelength > animlength )
				animlength = nodelength;
		}
		Interpolator* sclInterpl = dynamic_cast<Interpolator*>(node->scaleController());
		if ( sclInterpl )
		{
			nodelength = sclInterpl->getKeyTime( sclInterpl->keys() - 1 );
			if ( nodelength > animlength )
				animlength = nodelength;
		}
	}
	
	Scene* scene = dynamic_cast<Scene*>( root );
	if ( scene && animlength < scene->animationEnd() )
		animlength = scene->animationEnd();

	return animlength;
}

GameSurface* GameCharacter::groundMaterial() const
{
	return getCollisionMaterial( m_groundCollisionInfo );
}

const BoneCollisionBox&	GameCharacter::rootCollisionBox() const
{
	if ( !m_rootCollisionBox.hasBone() )
		throw Exception( Format("No bone collision boxes in character {0}", name()) );
	return m_rootCollisionBox;
}

const Colorf& GameCharacter::groundLightmapColor() const
{
	return m_groundLightmapColor;
}

float GameCharacter::peekMoveCheckDistance() const 
{
	return m_peekMoveCheckDistance;
}

GamePointObject* GameCharacter::visibilityCollisionChecker() const
{
	return m_visibilityChecker;
}

void GameCharacter::checkCollisionsAgainstCell( const Vector3& start, const Vector3& delta, BSPNode* bsptree, GameCell* collisionCell, CollisionInfo* cinfo )
{
	float margin = 0.01f;
	float r = boundSphere();
	
	float t = 1.f;
	const BSPPolygon* cpoly = 0;
	Vector3 cnormal( 0, 0, 0 );
	Vector3 cpoint( 0, 0, 0 );
	Vector3 shiftedStart = start + up()*(r+margin);
	BSPCollisionUtil::findMovingSphereIntersection( bsptree, shiftedStart, delta, r, CollisionInfo::COLLIDE_ALL, &t, &cpoly, &cnormal, &cpoint );

	if ( t < 1.f )
	{
		Vector3 shiftedEnd = shiftedStart + delta*t;
		Vector3 end = start + delta*t;

		float inside = r + margin - (shiftedEnd - cpoint).dot( cnormal );
		if ( inside > 0.f )
		{
			end += cnormal * inside;
		}

		*cinfo = CollisionInfo( end, cinfo->positionCell(), collisionCell, cpoly, cnormal, cpoint, 0, collisionCell->bspTree() );
	}
}

void GameCharacter::checkCollisionsAgainstObjects( const Vector3& start, const Vector3& delta, const Vector<GameObjectDistance>& objects, CollisionInfo* cinfo )
{
	// character position shifted up by bound sphere radius
	float margin = 0.01f;
	Vector3 shiftedStart = start + up()*(boundSphere()+margin);

	for ( int i = 0 ; i < objects.size() ; ++i )
	{
		GameObject* obj = objects[i].object;

		GameCharacter* character = dynamic_cast<GameCharacter*>( obj );
		if ( character && character != this )
		{
			Vector3 distv = start - character->position();
			float t;
			if ( !GameCharacter::isDead(character->primaryState()) &&
				delta.dot(distv) < 0.f && // moving towards each other?
				Intersection::findLineSphereIntersection( start, delta, character->position(), m_characterCollisionRadius+character->m_characterCollisionRadius, &t ) )
			{
				Vector3 cpos = start + delta*t;
				Vector3 cnormal = (cpos - character->position()).normalize();
				*cinfo = CollisionInfo( cpos+cnormal*0.01f, cinfo->positionCell(), character->cell(), 0, cnormal, cpos-cnormal*boundSphere(), character, 0 );
			}
			continue;
		}

		GameBoxTrigger* trigger = dynamic_cast<GameBoxTrigger*>( obj );
		if ( trigger )
		{
			if ( Intersection::testLineBox(start, delta, trigger->transform(), trigger->dimensions()+Vector3(boundSphere(),boundSphere(),boundSphere())) )
			{
				if ( trigger->addAffectedObject(this) && trigger->hasMethod("signalCharacterCollision") )
				{
					trigger->pushMethod( "signalCharacterCollision" );
					vm()->pushTable( this );
					trigger->call( 1, 0 );
				}
			}
			continue;
		}

		GameDynamicObject* dynamicObject = dynamic_cast<GameDynamicObject*>( obj );
		if ( dynamicObject && !dynamicObject->hidden() &&
			Intersection::findLineSphereIntersection(shiftedStart, delta, dynamicObject->position(), boundSphere()+dynamicObject->boundSphere(), 0) )
		{
			Matrix4x4 tm = dynamicObject->transform();
			Matrix4x4 invtm = tm.inverse();
			Vector3 bspStart = invtm.transform( shiftedStart );
			Vector3 bspDelta = invtm.rotate( delta );

			float t = 1.f;
			float r = boundSphere();
			const BSPPolygon* cpoly = 0;
			Vector3 cnormal( 0, 0, 0 );
			Vector3 cpoint( 0, 0, 0 );
			GameBSPTree* bsptree = dynamicObject->bspTree();
			BSPCollisionUtil::findMovingSphereIntersection( bsptree->root(), bspStart, bspDelta, r, CollisionInfo::COLLIDE_ALL, &t, &cpoly, &cnormal, &cpoint );

			if ( t < 1.f )
			{
				cnormal = tm.rotate( cnormal );
				cpoint = tm.transform( cpoint );
				Vector3 shiftedEnd = shiftedStart + delta*t;
				Vector3 end = start + delta*t;

				float inside = r + margin - (shiftedEnd - cpoint).dot( cnormal );
				if ( inside > 0.f )
				{
					end += cnormal * inside;
				}

				*cinfo = CollisionInfo( end, dynamicObject->cell(), dynamicObject->cell(), cpoly, cnormal, cpoint, dynamicObject, bsptree );
			}

			continue;
		}
	}
}

GameCharacter::PrimaryState GameCharacter::toPrimaryState( const String& str )
{
	for ( int i = 0 ; i < PRIMARY_STATE_COUNT ; ++i )
	{
		if ( str == sm_primaryStates[i].name )
			return (PrimaryState)i;
	}
	throw Exception( Format("String {0} is not valid character primary state name", str) );
	return PRIMARY_STATE_UNKNOWN;
}

GameCharacter::SecondaryState GameCharacter::toSecondaryState( const String& str )
{
	for ( int i = 0 ; i < SECONDARY_STATE_COUNT ; ++i )
	{
		if ( str == sm_secondaryStates[i].name )
			return (SecondaryState)i;
	}
	throw Exception( Format("String {0} is not valid character secondary state name", str) );
	return SECONDARY_STATE_UNKNOWN;
}

bool GameCharacter::canMove( const math::Vector3& delta ) const
{
	m_collisionChecker->setPosition( cell(), position() );
	float r = boundSphere()*.5f;
	m_collisionChecker->setBoundSphere( r );
	m_collisionChecker->moveWithoutColliding( up()*boundSphere() );
	CollisionInfo cinfo;
	m_collisionChecker->setObjectToIgnore( this );
	m_collisionChecker->move( delta, &cinfo );
	return !cinfo.isCollision(CollisionInfo::COLLIDE_ALL);
}

bool GameCharacter::canMoveLine( const math::Vector3& delta ) const
{
	Vector3 direction = delta;
	if ( delta.length() > Float::MIN_VALUE )
		direction = direction.normalize();

	m_collisionCheckerPoint->setPosition( cell(), position() );
	m_collisionCheckerPoint->setBoundSphere( boundSphere() );
	m_collisionCheckerPoint->moveWithoutColliding( up()*boundSphere() );
	CollisionInfo cinfo;
	m_collisionCheckerPoint->setObjectToIgnore( this );
	m_collisionCheckerPoint->move( delta + direction*boundSphere(), &cinfo );
	return !cinfo.isCollision(CollisionInfo::COLLIDE_ALL);
}

Node* GameCharacter::headBone() const
{
	return m_meshHeadBone;
}

float GameCharacter::maxSneakingSpeed() const
{
	return m_maxSneakingSpeed;
}

float GameCharacter::maxWalkingSpeed() const
{
	return m_maxWalkingSpeed;
}

float GameCharacter::maxRunningSpeed() const
{
	return m_maxRunningSpeed;
}

float GameCharacter::minSneakingSpeed() const
{
	return m_minSneakingSpeed;
}

float GameCharacter::minWalkingSpeed() const
{
	return m_minWalkingSpeed;
}

float GameCharacter::minRunningSpeed() const
{
	return m_minRunningSpeed;
}

float GameCharacter::maxSneakingControl() const
{
	return m_maxSneakingControl;
}

float GameCharacter::maxWalkingControl() const
{
	return m_maxWalkingControl;
}

float GameCharacter::maxRunningControl() const
{
	return m_maxRunningControl;
}

float GameCharacter::minSneakingControl() const
{
	return m_minSneakingControl;
}

float GameCharacter::minWalkingControl() const
{
	return m_minWalkingControl;
}

float GameCharacter::minRunningControl() const
{
	return m_minRunningControl;
}

float GameCharacter::hitRange() const 
{
	return m_hitRange;
}

float GameCharacter::canSeeProbability( GameCharacter* other ) const
{
	if ( level()->isActiveCutScene() )
		return 0.f;

	// vision frustum test
	Matrix4x4 headtm = headWorldTransform();
	Vector3 otherPos = headtm.inverse().transform( other->headWorldPosition() ); // other character position in this character space
	bool visible = otherPos.z > Float::MIN_VALUE;
	float detectProb = 0.f;
	if ( visible )
	{
		// check against bounding sphere
		Vector3 origDir = -otherPos;
		origDir.z = 0.f;
		float dist = origDir.length();
		if ( dist > Float::MIN_VALUE )
		{
			origDir *= 1.f/dist;
			float signx0 = otherPos.x < 0.f ? -1.f : 1.f;
			float signy0 = otherPos.y < 0.f ? -1.f : 1.f;
			otherPos += origDir * boundSphere();
			float signx1 = otherPos.x < 0.f ? -1.f : 1.f;
			float signy1 = otherPos.y < 0.f ? -1.f : 1.f;
			if ( signx0 != signx1 )
				otherPos.x = 0.f;
			if ( signy0 != signy1 )
				otherPos.y = 0.f;
		}

		// vision frustum test
		ComputerControl* cc				= computerControl();
		const float lightEffectMin		= cc->getNumber( "visionLightEffectMin" );
		const float lightEffectMax		= cc->getNumber( "visionLightEffectMax" );
		const float horzInnerCone		= Math::toRadians( cc->getNumber( "visionHorzInnerCone" ) ) * .5f;
		const float horzOuterCone		= Math::toRadians( cc->getNumber( "visionHorzOuterCone" ) ) * .5f;
		const float vertInnerCone		= Math::toRadians( cc->getNumber( "visionVertInnerCone" ) ) * .5f;
		const float vertOuterCone		= Math::toRadians( cc->getNumber( "visionVertOuterCone" ) ) * .5f;
		const float farAttenStart		= cc->getNumber( "visionFarAttenStart" );
		const float farAttenEnd			= cc->getNumber( "visionFarAttenEnd" );

		float horzAngle = Math::atan( Math::abs(otherPos.x) / otherPos.z );
		float vertAngle = Math::atan( Math::abs(otherPos.y) / otherPos.z );
		float distance = otherPos.length();

		visible = distance < farAttenEnd && horzAngle < horzOuterCone && vertAngle < vertOuterCone;
		if ( visible )
		{
			detectProb = 1.f;
			detectProb *= lerp( 1.f, 0.f, (vertAngle-vertInnerCone)/Math::max(vertOuterCone-vertInnerCone,1e-6f) );
			detectProb *= lerp( 1.f, 0.f, (horzAngle-horzInnerCone)/Math::max(horzOuterCone-horzInnerCone,1e-6f) );
			detectProb *= lerp( lightEffectMin, lightEffectMax, other->groundLightmapColor().brightness() );
			detectProb *= lerp( 1.f, 0.f, (distance-farAttenStart)/Math::max(farAttenEnd-farAttenStart,1e-6f) );
			visible = detectProb > 0.f;
		}

		// occlusion test
		if ( visible )
		{
			visible = other->isTorsoVisibleFrom( headtm.translation() );
			if ( !visible )
				detectProb = 0.f;
		}
	}
	return detectProb;
}

int GameCharacter::methodCall( VM* vm, int i )
{
	return ScriptUtil<GameCharacter,GameObject>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

int	GameCharacter::script_setAnimationFolder( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects animation folder name (directory name in characters/ folder)", funcName) );

	m_animationFolderName = vm->toString(1);
	if ( !m_animationFolderName.endsWith("/") )
		m_animationFolderName = m_animationFolderName + "/";
	return 0;
}

int GameCharacter::script_addAnimation( VM* vm, const char* funcName )
{
	int argc = vm->top();
	int tags1[] = {VM::TYPE_STRING};
	int tags2[] = {VM::TYPE_STRING, VM::TYPE_STRING};
	int tags3[] = {VM::TYPE_STRING, VM::TYPE_STRING, VM::TYPE_STRING};
	if ( !hasParams(tags1,sizeof(tags1)/sizeof(tags1[0])) &&
		!hasParams(tags2,sizeof(tags2)/sizeof(tags2[0])) &&
		!hasParams(tags3,sizeof(tags3)/sizeof(tags3[0])) )
		throw ScriptException( Format("{0} expects animation base name, optional end and optional pre behaviour (RESET, CONSTANT, REPEAT, OSCILLATE)", funcName) );

	const String name = vm->toString(1);
	const String endBehaviour = (argc >= 2 ? String(vm->toString(2)).toUpperCase() : "REPEAT");
	const String preBehaviour = (argc >= 3 ? String(vm->toString(3)).toUpperCase() : "REPEAT");

	if ( !m_anims->hasGroup(name) )
	{
		// load animation
		String fname = m_animationFolderName + name + ".sg";
		P(Node) root = m_sceneMgr->getScene( fname, SceneFile::LOAD_ANIMATIONS );
		m_anims->addGroup( name, root );

		// test bones in animation against bones in m_mesh
		if ( m_mesh )
		{
			assert( m_mesh );
			for ( Node* node = root; node ; node = node->nextInHierarchy() )
			{
				if ( node->name().indexOf( "Bip" ) != -1 ) 
					if ( !NodeUtil::findNodeByName( m_mesh, node->name() ) )
						throw Exception( Format("Anim {1} Node {0} not found in mesh!", node->name(), name ) );
			}
		}

		// optional: set end behaviour
		if ( argc >= 2 )
		{
			Interpolator::BehaviourType behaviour = Interpolator::toBehaviour( endBehaviour );
			NodeUtil::setHierarchyEndBehaviour( root, behaviour );
		}
		// optional: set pre behaviour
		if ( argc >= 3 )
		{
			Interpolator::BehaviourType behaviour = Interpolator::toBehaviour( preBehaviour );
			NodeUtil::setHierarchyPreBehaviour( root, behaviour );
		}
	}
	return 0;
}

int	GameCharacter::script_blendAnimation( script::VM* vm, const char* funcName )
{
	int argc = vm->top();
	int tags1[] = {VM::TYPE_STRING, VM::TYPE_NUMBER};
	int tags2[] = {VM::TYPE_STRING, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	int tags3[] = {VM::TYPE_STRING, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags1,sizeof(tags1)/sizeof(tags1[0])) &&
		!hasParams(tags2,sizeof(tags2)/sizeof(tags2[0])) &&
		!hasParams(tags3,sizeof(tags3)/sizeof(tags3[0])))
		throw ScriptException( Format("{0} expects animation base name, blend time, optional frames per second (negative fps causes reverse playback) and optional start time", funcName) );

	assert( m_primaryBlender );

	m_primaryBlender->fadeoutAllBlends();

	const String	dst			= vm->toString(1);
	float			delay		= vm->toNumber(2);
	float			fps			= (argc >= 3 ? vm->toNumber(3) : 30.f );
	float			dstStart	= (argc >= 4 ? vm->toNumber(4) : 0.f);

//	if ( m_anims->hasGroup( m_activeMovementAnimation ) )
//		m_movementAnimations[ m_activeMovementAnimation ]->deactivate();

	m_activeMovementAnimation = "";

	if ( !m_anims->hasGroup(dst) )
		throw ScriptException( Format("{0} expects valid animation name (animation {1} does not exist)", funcName, dst) );

	AnimationParams params;
	params.blendDelay = delay;
	params.blendTime = 0.f;
	params.time = dstStart;
	params.name = dst;
	params.weight = 0.f;
	params.speed = fps / 30.f;

	m_primaryAnimBlenderID = m_primaryBlender->addBlend( params, 1.f ); 
	m_primaryTime = dstStart;
	m_primarySpeed = fps / 30.f;

	return 0;
}

int GameCharacter::script_setAnimation( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects a string, animation base name", funcName) );

	const String dst = vm->toString(1);
	if ( !m_anims->hasGroup(dst) )
		throw ScriptException( Format("{1}({0}) failed - no such animation", dst, funcName ) );

	AnimationParams params;
	params.blendDelay = 0.00001f;
	params.blendTime = 0.f;
	params.time = 0.f;
	params.name = dst;
	params.weight = 0.f;
	params.speed = 1.f;

	assert( m_primaryBlender );

	m_primaryBlender->fadeoutAllBlends();
	m_primaryAnimBlenderID = m_primaryBlender->addBlend( params, 1.f ); 
	m_primaryTime = 0.f;
	
	m_activeMovementAnimation = "";

	return 0;
}

int GameCharacter::script_addSecondaryAnimation( script::VM* vm, const char* funcName )
{	
	int argc = vm->top();
	if ( argc < 1 )
		throw ScriptException( Format("{0} prepares a secondary animation from animation name and a set of bones ( only top-level bones in hierarchy need to be specified )", funcName ) ); 
	for ( int i = 1 ; i <= argc ; ++i )
		if ( !vm->isString(i) )
			throw ScriptException( Format("{0} prepares a secondary animation from animation name and a set of bones ( only top-level bones in hierarchy need to be specified )", funcName ) ); 

	SecondaryAnimationParams params;

	params.name = vm->toString(1);
	P(Node) root = 0;

	if ( !m_anims->hasGroup(params.name) )
	{
		String fname = m_animationFolderName + params.name + ".sg";
		root = m_sceneMgr->getScene( fname, SceneFile::LOAD_ANIMATIONS );
		m_anims->addGroup( params.name, root );
	}
	else
	{
		String fname = m_animationFolderName + params.name + ".sg";
		root = m_sceneMgr->getScene( fname, SceneFile::LOAD_ANIMATIONS );
	}

	for ( int i = 0; i < (argc-1); ++i )
	{
		Node* bone = NodeUtil::findNodeByName( root, vm->toString( i + 2 ) );
		if ( bone )
		{
			Node* parent = bone->parent();
			bone->unlink();

			for ( Node* iterator = bone; iterator; iterator = iterator->nextInHierarchy() )
			{					
				params.bones.put( iterator->name(), iterator );
			}
			bone->linkTo( parent );
		}
	}
	m_preparedSecondaryAnimations.put( vm->toString(1), params );

	return 0;
}

int GameCharacter::script_setRotationSpeed( VM* vm, const char* funcName )
{
	float v;
	int retv = getParam( vm, funcName, "degrees/s", &v );
	m_rotationSpeed = Math::toRadians( v );
	return retv;
}

int GameCharacter::script_setFriction( VM* vm, const char* funcName )
{
	return getParam( vm, funcName, "relative speed decrease in second", &m_friction );
}

int	GameCharacter::script_setFallingFriction( script::VM* vm, const char* funcName )
{
	return getParam( vm, funcName, "relative speed decrease in second when falling", &m_fallingFriction );
}

int GameCharacter::script_setGravity( VM* vm, const char* funcName )
{
	return getParam( vm, funcName, "G's", &m_gravity );
}

int GameCharacter::script_setSteepSurface( VM* vm, const char* funcName )
{
	float v = 0.f;
	int retv = getParam( vm, funcName, "degrees", &v );
	m_steepAngleCos = Math::cos( Math::toRadians(v) );
	return retv;
}

int	GameCharacter::script_addPrimaryStateListener( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_FUNCTION, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects a state name, function, time and state flags", funcName) );

	// find state type
	const String stateName = vm->toString(1);
	PrimaryState state = findPrimaryState( stateName );

	// check set flags
	int flags = (int)vm->toNumber(4);
	if ( flags & ~(STATE_ENTRY|STATE_EXIT|STATE_REMOVE) )
		throw ScriptException( Format("{0} got invalid player state flags ({1,x})", funcName, flags) );
	if ( 0 == (sm_primaryStates[state].flags & STATE_WAIT) && 0 != (flags & STATE_REMOVE) )
		throw ScriptException( Format("STATE_REMOVE flag cannot be used with non-waited state ({0})", stateName) );

	// check time
	float time = vm->toNumber(3);
	if ( 0 != (flags & STATE_EXIT) && time != 0.f )
		throw ScriptException( Format("{0} does not accept non-zero time if player state flag STATE_EXIT is set", funcName) );

	// create state listener
	StateListener listener;
	vm->pushValue( 2 );
	listener.scriptFuncRef = vm->ref( true );
	listener.time = time;
	listener.flags = (flags | sm_primaryStates[state].flags);

	// connect the listener to the state
	assert( m_primaryStateListeners.size() > (int)state );
	if ( !m_primaryStateListeners[state] )
		m_primaryStateListeners[state] = new Vector<StateListener>( Allocator<StateListener>(__FILE__) );
	m_primaryStateListeners[state]->add( listener );
	
	return 0;
}

int	GameCharacter::script_addSecondaryStateListener( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_FUNCTION, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects a state name, function, time and state flags", funcName) );

	// find state type
	const String stateName = vm->toString(1);
	SecondaryState state = findSecondaryState( stateName );

	// check set flags
	int flags = (int)vm->toNumber(4);
	if ( flags & ~(STATE_ENTRY|STATE_EXIT|STATE_REMOVE) )
		throw ScriptException( Format("{0} got invalid player state flags ({1,x})", funcName, flags) );
	if ( 0 == (sm_secondaryStates[state].flags & STATE_WAIT) && 0 != (flags & STATE_REMOVE) )
		throw ScriptException( Format("STATE_REMOVE flag cannot be used with non-waited state ({0})", stateName) );

	// check time
	float time = vm->toNumber(3);
	if ( 0 != (flags & STATE_EXIT) && time != 0.f )
		throw ScriptException( Format("{0} does not accept non-zero time if player state flag STATE_EXIT is set", funcName) );

	// create state listener
	StateListener listener;
	vm->pushValue( 2 );
	listener.scriptFuncRef = vm->ref( true );
	listener.time = time;
	listener.flags = (flags | sm_secondaryStates[state].flags);

	// connect the listener to the state
	assert( m_secondaryStateListeners.size() > (int)state );
	if ( !m_secondaryStateListeners[state] )
		m_secondaryStateListeners[state] = new Vector<StateListener>( Allocator<StateListener>(__FILE__) );
	m_secondaryStateListeners[state]->add( listener );
	
	return 0;
}

int	GameCharacter::script_addAnimationListener( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_FUNCTION, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects a animation name, function and time", funcName) );

	// check time
	float time = vm->toNumber(3);
	if ( time < 0.f )
		throw ScriptException( Format("{0} does not accept negative time", funcName) );

	// check animation name
	String animName = vm->toString(1);
	if ( !m_anims->hasGroup(animName) )
		throw ScriptException( Format("{0} expects valid animation name (animation {1} not exist)", funcName, animName) );

	// create animation listener
	AnimationListener listener;
	vm->pushValue( 2 );
	listener.scriptFuncRef = vm->ref( true );
	listener.animationName = animName;
	listener.time = time;

	m_animListeners.add( listener );
	return 0;
}

int GameCharacter::script_resetInputState( script::VM*, const char* )
{
	resetInputState();
	return 0;
}

int GameCharacter::script_setPrimaryState( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects a state name string", funcName) );
	
	const String stateName = vm->toString(1);
	PrimaryState state = findPrimaryState( stateName );
	setPrimaryState( state );
	return 0;
}

int GameCharacter::script_setSecondaryState( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects a state name string", funcName) );
	
	const String stateName = vm->toString(1);
	SecondaryState state = findSecondaryState( stateName );
	setSecondaryState( state );
	return 0;
}

int GameCharacter::script_setMorphBase( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects morph base object name", funcName) );
	
	String baseName = vm->toString(1);

	// find morph base
	Mesh* baseMesh = dynamic_cast<Mesh*>( NodeUtil::findNodeByName( m_mesh, baseName ) );
	if ( !baseMesh )
		throw ScriptException( Format("{0} excepts valid object name as parameter, object {1} not found", funcName, baseName) );

	// create new container for original morph base models
	P(Mesh) baseClone = new Mesh;
	baseClone->setName( baseMesh->name() );

	// replace base models with output (morph result) models,
	// move base models safe to cloned mesh
	int basePrimitives = baseMesh->primitives();
	for ( int i = 0 ; i < basePrimitives ; )
	{
		P(Model) model = dynamic_cast<Model*>( baseMesh->getPrimitive(i) );
		if ( model )
		{
			P(Model) modelClone = new Model( *model, Model::SHARE_SHADER );
			baseMesh->addPrimitive( modelClone );
			baseClone->addPrimitive( model );
			baseMesh->removePrimitive( i );
			--basePrimitives;
		}
		else
		{
			++i;
		}
	}

	m_morphBases.add( baseClone );
	return 0;
}

int GameCharacter::script_addMorphAnimation( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING,VM::TYPE_STRING,VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),1) )
		throw ScriptException( Format("{0} expects morph base object name, morph animation gm file name and optional end behaviour (REPEAT,CONSTANT,RESET,OSCILLATE)", funcName) );
	
	String	baseName	= vm->toString(1);
	String	animName	= vm->toString(2);

	// animation end behaviour (default is REPEAT)
	Interpolator::BehaviourType behaviour = Interpolator::BEHAVIOUR_REPEAT;
	if ( vm->top() >= 3 )
		behaviour = Interpolator::toBehaviour( vm->toString(3) );

	addMorphAnimation( baseName, animName, behaviour );
	return 0;
}

int GameCharacter::script_playMorphAnimationOnce( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING,VM::TYPE_STRING,VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),1) )
		throw ScriptException( Format("{0} expects morph base object name, morph animation gm file name and optional end behaviour (REPEAT,CONSTANT,RESET,OSCILLATE)", funcName) );
	
	String	baseName	= vm->toString(1);
	String	animName	= vm->toString(2);

	// animation end behaviour (default is REPEAT)
	Interpolator::BehaviourType behaviour = Interpolator::BEHAVIOUR_REPEAT;
	if ( vm->top() >= 3 )
		behaviour = Interpolator::toBehaviour( vm->toString(3) );

	if ( !hasMorphAnimation(animName) )
	{
		MorphAnimation* anim = addMorphAnimation( baseName, animName, behaviour );
		anim->enableRemoveAfterEnd();
	}

	playMorphAnimation( animName );
	return 0;
}

int GameCharacter::script_playMorphAnimation( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),0) )
		throw ScriptException( Format("{0} expects morph animation gm file name", funcName) );
	
	String animName = vm->toString(1);

	playMorphAnimation( animName );
	return 0;
}

int GameCharacter::script_blendMorphAnimation( script::VM* vm, const char* funcName )
{
	int argc = vm->top();
	int tags1[] = {VM::TYPE_STRING, VM::TYPE_NUMBER};
	int tags2[] = {VM::TYPE_STRING, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags1,sizeof(tags1)/sizeof(tags1[0])) &&
		!hasParams(tags2,sizeof(tags2)/sizeof(tags2[0])) )
		throw ScriptException( Format("{0} expects morph animation name, blend time and optional start time", funcName) );

	const String	dst		= vm->toString(1);
	float			delay	= vm->toNumber(2);

	// find destination animation
	MorphAnimation* dstAnim	= 0;
	for ( int i = 0 ; i < m_morphAnims.size() ; ++i )
	{
		MorphAnimation* anim = m_morphAnims[i];
		if ( anim->name() == dst )
		{
			dstAnim = anim;
			break;
		}
	}
	if ( !dstAnim )
		throw ScriptException( Format("{0} expects valid morph animation name (animation {1} not exist)", funcName, dst) );

	// find active source animation if any
	MorphAnimation* srcAnim	= 0;
	for ( int i = 0 ; i < m_morphAnims.size() ; ++i )
	{
		MorphAnimation* anim = m_morphAnims[i];
		if ( anim->baseMeshName() == dstAnim->baseMeshName() && 
			anim->active() &&
			!anim->isBlendTarget() )
		{
			srcAnim = anim;
			break;
		}
	}

	// stop previous animations applied to the base mesh
	for ( int i = 0 ; i < m_morphAnims.size() ; ++i )
	{
		MorphAnimation* morphAnim = m_morphAnims[i];
		if ( morphAnim->baseMeshName() == dstAnim->baseMeshName() && morphAnim != srcAnim )
			morphAnim->stop();
	}

	// destination animation start time offset
	float dstStart = 0.f;
	if ( argc >= 3 )
		dstStart = vm->toNumber(3);
	else if ( srcAnim )
		dstStart = srcAnim->time();

	// start blending
	if ( srcAnim && srcAnim != dstAnim )
		srcAnim->blendTo( dstAnim, delay, dstStart );
	else // ( !srcAnim )
		dstAnim->start( dstStart );

	return 0;
}

int GameCharacter::script_setMesh( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects a mesh file name, LOD min size limit (pixels) and max size limit", funcName) );

	int		param = 1;
	String	sceneName = vm->toString(param++);
	float	minSize = vm->toNumber(param++);
	float	maxSize = vm->toNumber(param++);

	m_mesh = m_sceneMgr->getScene( sceneName, SceneFile::LOAD_GEOMETRY )->clone();
	removeLightsAndCameras( m_mesh );
	setRenderPasses( m_mesh, GameRenderPass::RENDERPASS_CHARACTER_SOLID, GameRenderPass::RENDERPASS_CHARACTER_TRANSPARENT );
	MeshUtil::restoreBones( m_mesh );

	// find shortcut bones
	m_meshHeadBone = NodeUtil::findNodeByName( m_mesh, "Bip01 Head" );
	if ( !m_meshHeadBone )
		throw Exception( Format("Bip01 Head not found in {0}", sceneName ) );
	m_meshNeckBone = NodeUtil::findNodeByName( m_mesh, "Bip01 Neck" );
	if ( !m_meshNeckBone )
		throw Exception( Format("Bip01 Neck not found in {0}", sceneName ) );
	m_meshLeftFootBone = NodeUtil::findNodeByName( m_mesh, "Bip01 L Foot" );
	if ( !m_meshLeftFootBone )
		throw Exception( Format("Bip01 L Foot not found in {0}", sceneName ) );
	m_meshRightFootBone = NodeUtil::findNodeByName( m_mesh, "Bip01 R Foot" );
	if ( !m_meshRightFootBone )
		throw Exception( Format("Bip01 R Foot not found in {0}", sceneName ) );
	m_meshSpineBone = NodeUtil::findNodeByName( m_mesh, "Bip01 Spine" );
	if ( !m_meshSpineBone )
		throw Exception( Format("Bip01 Spine not found in {0}", sceneName ) );
	m_hitBone = NodeUtil::findNodeByName( m_mesh, "Bip01 Spine2" );
	if ( !m_hitBone )
		throw Exception( Format("Bip01 Spine2 not found in {0}", sceneName ) );
	m_meshLeftForeArmBone = NodeUtil::findNodeByName( m_mesh, "Bip01 L Forearm" );
	if ( !m_meshLeftForeArmBone )
		throw Exception( Format("Bip01 L Forearm not found in {0}", sceneName ) );
	m_meshRightForeArmBone = NodeUtil::findNodeByName( m_mesh, "Bip01 R Forearm" );
	if ( !m_meshRightForeArmBone )
		throw Exception( Format("Bip01 R Forearm not found in {0}", sceneName ) );
	m_meshLeftUpperArmBone = NodeUtil::findNodeByName( m_mesh, "Bip01 L UpperArm" );
	if ( !m_meshLeftUpperArmBone )
		throw Exception( Format("Bip01 L UpperArm not found in {0}", sceneName ) );
	m_meshRightUpperArmBone = NodeUtil::findNodeByName( m_mesh, "Bip01 R UpperArm" );
	if ( !m_meshRightUpperArmBone )
		throw Exception( Format("Bip01 R UpperArm not found in {0}", sceneName ) );

	// create LOD container
	m_lod = new LOD;
	m_lod->setRadius( 1.f );

	// add Mesh clones to LOD
	for ( Node* node = m_mesh ; node ; node = node->nextInHierarchy() )
	{
		Mesh* mesh = dynamic_cast<Mesh*>( node );
		if ( mesh && mesh->primitives() > 0 && mesh->renderable() )
			m_lod->add( mesh, minSize, maxSize );
	}

	return 0;
}

int GameCharacter::script_setLOD( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects a mesh file name, LOD min size limit (pixels) and max size limit", funcName) );
	if ( !m_lod || !m_mesh )
		throw ScriptException( Format("{0} requires setMesh called first", funcName) );

	int		param = 1;
	String	sceneName = vm->toString(param++);
	float	minSize = vm->toNumber(param++);
	float	maxSize = vm->toNumber(param++);

	P(Node) root = m_sceneMgr->getScene( sceneName, SceneFile::LOAD_GEOMETRY );

	// we clone only found Meshes from the LOD scene
	// and keep the hierarchy valid by finding corresponding
	// parent nodes from the original (setMesh) hierarchy
	for ( Node* node = root ; node ; node = node->nextInHierarchy() )
	{
		Mesh* mesh = dynamic_cast<Mesh*>( node );
		if ( mesh && mesh->primitives() > 0 && mesh->renderable() )
		{
			assert( mesh->parent() );
			P(Mesh) meshClone = new Mesh( *mesh );
			m_lod->add( meshClone, minSize, maxSize );

			// add mesh to original node hierarchy
			if ( mesh->parent() == mesh->root() )
			{
				// mesh is parented to root
				meshClone->linkTo( m_mesh );
				//Debug::println( "Cloned LOD mesh {0} and added to {1} node", mesh->name(), "root" );
			}
			else
			{
				// find corresponding parent from the original node hierarchy
				for ( Node* node = m_mesh ; node ; node = node->nextInHierarchy() )
				{
					if ( node->name() == mesh->parent()->name() )
					{
						meshClone->linkTo( node );
						//Debug::println( "Cloned LOD mesh {0} and added to {1} node", mesh->name(), node->name() );
						break;
					}
				}
				if ( !meshClone->parent() )
					throw ScriptException( Format("Original mesh {0} does not have object named {1}, but LOD {2} has", m_mesh->name(), mesh->parent()->name(), root->name()) );
			}
		}
	}

	removeLightsAndCameras( m_mesh );
	setRenderPasses( m_mesh, GameRenderPass::RENDERPASS_CHARACTER_SOLID, GameRenderPass::RENDERPASS_CHARACTER_TRANSPARENT );
	MeshUtil::restoreBones( m_mesh );
	return 0;
}

int GameCharacter::script_setCharacterCollisionRadius( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} character-character collision radius", funcName) );

	m_characterCollisionRadius = vm->toNumber(1);
	return 0;
}

int GameCharacter::script_setCrouchWalkingSpeed( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects number", funcName) );

	m_crouchWalkingSpeed = vm->toNumber(1);
	return 0;
}

int GameCharacter::script_setCrouchStrafingSpeed( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects number", funcName) );

	m_crouchStrafingSpeed = vm->toNumber(1);
	return 0;
}

int GameCharacter::script_setCrouchBackwardSpeed( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects number", funcName) );

	m_crouchBackwardSpeed = vm->toNumber(1);
	return 0;
}

int GameCharacter::script_setRollingSpeedForward( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects forward rolling speed (m/s)", funcName) );

	m_rollingSpeedForward = vm->toNumber(1);
	return 0;
}

int GameCharacter::script_setRollingSpeedBackward( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects backward rolling speed (m/s)", funcName) );

	m_rollingSpeedBackward = vm->toNumber(1);
	return 0;
}

int GameCharacter::script_setRollingSpeedSideways( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects sideways rolling speed (m/s)", funcName) );

	m_rollingSpeedSideways = vm->toNumber(1);
	return 0;
}

int GameCharacter::script_setMaxAnimSlewRatePrimary( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects primary animation blender max slew rate (weight/s)", funcName) );
	
	m_primaryBlender->setMaxSlewRate( vm->toNumber(1) );
	return 0;
}

int GameCharacter::script_setMaxAnimSlewRateSecondary( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects secondary animation blender max slew rate (weight/s)", funcName) );

	m_secondaryBlender->setMaxSlewRate( vm->toNumber(1) );
	return 0;
}

int GameCharacter::script_evaluatePrimaryState( script::VM* vm, const char* funcName )
{
	if ( vm->top() > 0 )
		throw ScriptException( Format("{0} forces player state evaluation", funcName) );

	setPrimaryState( evaluatePrimaryState() );
	return 0;
}

int GameCharacter::script_evaluateSecondaryState( script::VM* vm, const char* funcName )
{
	if ( vm->top() > 0 )
		throw ScriptException( Format("{0} forces player state evaluation", funcName) );

	setSecondaryState( evaluateSecondaryState() );
	return 0;
}

int GameCharacter::script_aimAt( script::VM* vm, const char* funcName ) 
{
	GameObject* obj = 0;
	Vector3 pos( 0.f, 0.f, 0.f );
	getGameObjectOrVector3( vm, funcName, &obj, &pos );

	if ( obj )
	{
		GameCharacter* target = dynamic_cast<GameCharacter*>(obj);
		if ( target )		
			pos = target->hitCenter();
	}

	aimAt( pos );

	return 0;
}

int GameCharacter::script_stateFalling( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} return true if player is falling", funcName) );

	vm->pushBoolean( m_primaryState == PRIMARY_STATE_FALLING );
	return 1;
}

int	GameCharacter::script_stateControllable( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} return true if player is in controllable state", funcName) );

	vm->pushBoolean( isControllable(m_primaryState) );
	return 1;
}


int GameCharacter::script_stateInAir( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} return true if player is in air", funcName) );

	vm->pushBoolean( m_primaryState == PRIMARY_STATE_INAIR );
	return 1;
}

int GameCharacter::script_blendSecondaryAnimation( VM* vm, const char* funcName )
{
	int argc = vm->top();
	int tags1[] = {VM::TYPE_STRING, VM::TYPE_NUMBER};
	int tags2[] = {VM::TYPE_STRING, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	int tags3[] = {VM::TYPE_STRING, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags1,sizeof(tags1)/sizeof(tags1[0])) &&
		!hasParams(tags2,sizeof(tags2)/sizeof(tags2[0])) &&
		!hasParams(tags3,sizeof(tags3)/sizeof(tags3[0])))
		throw ScriptException( Format("{0} expects animation base name, blend time, optional frames-per-second and optional start time", funcName) );

	const String dst = vm->toString(1);
	if ( !m_anims->hasGroup(dst) )
		throw ScriptException( Format("setAnimation({0}) failed - no such animation", dst) );

	float blendtime = vm->toNumber(2);
	float fps = argc >= 3 ? vm->toNumber(3) : 30.f;
	float start = argc >= 4 ? vm->toNumber(4) : 0.f;

	assert( m_secondaryBlender );

//	if ( m_anims->hasGroup( m_activeSecondaryMovementAnimation ) )
//		m_secondaryMovementAnimations[ m_activeSecondaryMovementAnimation ]->deactivate();

	m_activeSecondaryMovementAnimation = "";
	m_secondaryBlender->fadeoutAllBlends();
	
	AnimationParams params;
	params.name = dst;
	params.time = start;
	params.speed = fps / 30.f;
	params.weight = 0.f;
	params.blendDelay = blendtime;
	params.blendTime = 0.f;

	m_secondaryAnimBlenderID = m_secondaryBlender->addBlend( params, 1.f );
	m_secondaryTime = start;
	m_secondarySpeed = fps / 30.f;

	return 0;
}

int GameCharacter::script_clearSecondaryAnimations( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} clears secondary blender", funcName ) );

	assert( m_secondaryBlender );
	m_secondaryBlender->fadeoutAllBlends();
	m_activeSecondaryMovementAnimation = "";
	m_secondaryAnimBlenderID = Blender::INVALIDID;

	return 0;
}

int GameCharacter::script_createMovementAnimation( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects name string", funcName) );
	
	String dst = vm->toString(1);

	m_movementAnimations[dst] = new MovementAnimation( vm, m_arch, m_soundMgr, m_particleMgr, this, m_primaryBlender ); // Reset value

	vm->pushTable( m_movementAnimations[dst] );
	return 1;
}

int GameCharacter::script_activateMovementAnimation( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects target name", funcName) );

	String name = vm->toString(1);

	if ( !m_movementAnimations.containsKey( name ) )
		throw ScriptException( Format("Invalid movement animation name : {0}", name ) );

	assert( m_primaryBlender );

	if ( name != m_activeMovementAnimation )
	{
		m_primaryBlender->fadeoutAllBlends();
		m_movementAnimations[name]->activate( false );
	}

	m_activeMovementAnimation = name;
	
	return 0;
}

int GameCharacter::script_createSecondaryMovementAnimation( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects name string", funcName) );
	
	String dst = vm->toString(1);

	m_secondaryMovementAnimations[dst] = new MovementAnimation( vm, m_arch, m_soundMgr, m_particleMgr, this, m_secondaryBlender ); // Reset value

	vm->pushTable( m_secondaryMovementAnimations[dst] );
	return 1;
}

int GameCharacter::script_activateSecondaryMovementAnimation( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects secondary movement animation name", funcName) );

	String name = vm->toString(1);

	if ( !m_secondaryMovementAnimations.containsKey( name ) )
		throw ScriptException( Format("Invalid secondary movement animation name : {0}", name ) );

	if ( name != m_activeSecondaryMovementAnimation )
	{
		m_secondaryBlender->fadeoutAllBlends();
		m_secondaryMovementAnimations[name]->activate( false );
	}

	m_activeSecondaryMovementAnimation = name;
	
	return 0;
}

int GameCharacter::script_setSecondaryMovementAnimation( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects secondary movement animation name", funcName) );

	String animName = vm->toString(1);

	if ( !m_secondaryMovementAnimations.containsKey( animName ) )
		throw ScriptException( Format("Invalid secondary movement animation name : {0}", animName ) );

	m_secondaryBlender->removeAllBlends();
	m_secondaryMovementAnimations[ animName ]->activate( true );
	m_activeSecondaryMovementAnimation = animName;
	
	return 0;
}

int GameCharacter::script_getAnimLength( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects string", funcName) );

	vm->pushNumber( animLength( vm->toString(1) ) );
	return 1;
}

int GameCharacter::script_isCrouched( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} expects no parameters", funcName ) );

	vm->pushBoolean( m_primaryState == PRIMARY_STATE_CROUCHED );
	return 1;
}

int GameCharacter::script_isDead( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} expects no parameters", funcName ) );

	vm->pushBoolean( isDead( m_primaryState ) );
	return 1;
}

int GameCharacter::script_getComputerControl( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} expects no parameters", funcName ) );

	vm->pushTable( m_computerControl );
	return 1;
}

int GameCharacter::script_setHealth( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects number", funcName) );

	m_health = vm->toNumber(1);
	return 0;
}

int GameCharacter::script_canSee( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_TABLE};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} returns true if character passed in as parameter can be seen", funcName) );
	
	GameCharacter* other = dynamic_cast<GameCharacter*>( getThisPtr(vm,1) );
	if ( !other )
		throw ScriptException( Format("{0} expects game character", funcName) );

	float visionLimit	= computerControl()->getNumber( "visionLimit" );
	bool visible		= canSeeProbability(other) > visionLimit;

	vm->pushBoolean( visible );
	return 1;
}

int GameCharacter::script_canMove( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER,VM::TYPE_NUMBER,VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects (x,y,z) delta", funcName) );

	Vector3 delta;
	delta.x = vm->toNumber(1);
	delta.y = vm->toNumber(2);
	delta.z = vm->toNumber(3);

	vm->pushBoolean( canMoveLine(delta) );
	return 1;
}

int GameCharacter::script_setWeapon( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_TABLE};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects GameWeapon table", funcName) );
	
	GameWeapon* o = dynamic_cast<GameWeapon*>( getThisPtr(vm,1) );
	if ( !o )
		throw ScriptException( Format("{0} expects GameWeapon", funcName) );

	setWeapon( o );

	return 0;
}

int	GameCharacter::script_addCollisionBone( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects bone name and damage multiplier", funcName) );

	if ( !m_mesh )
		throw ScriptException( Format("setMesh must be called before {0}", funcName) );

	String boneName = vm->toString(1);
	float damageMultiplier = vm->toNumber(2);

	Node* bone = NodeUtil::findNodeByName( m_mesh, boneName );
	if ( !bone )
		throw Exception( Format("Collision bone {0} not found in {1} (case-sensitive)", boneName, m_mesh->name()) );

	BoneCollisionBox box( m_mesh, bone, damageMultiplier );
	if ( box.boxMax() != box.boxMin() )
		m_boneCollisionBoxes.add( box );

	m_rootCollisionBox = BoneCollisionBox( m_mesh, m_mesh, 0.f );
	return 0;
}

int	GameCharacter::script_setAimingTimeAfterShooting( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects aiming time after shooting, in seconds", funcName) );

	m_aimingTimeAfterShooting = vm->toNumber(1);
	return 0;
}

int	GameCharacter::script_weapon( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns current weapon", funcName) );

	vm->pushTable( m_weapon );
	return 1;
}

int	GameCharacter::script_setSneakingSpeedRange( script::VM* vm, const char* funcName )
{
	float v[2];
	getParams( vm, funcName, "min and max sneaking speed, m/s", v, 2 );
	m_minSneakingSpeed = v[0];
	m_maxSneakingSpeed = v[1];
	return 0;
}

int	GameCharacter::script_setWalkingSpeedRange( script::VM* vm, const char* funcName )
{
	float v[2];
	getParams( vm, funcName, "min and max walking speed, m/s", v, 2 );
	m_minWalkingSpeed = v[0];
	m_maxWalkingSpeed = v[1];
	return 0;
}

int GameCharacter::script_setRunningSpeedRange( script::VM* vm, const char* funcName )
{
	float v[2];
	getParams( vm, funcName, "min and max running speed, m/s", v, 2 );
	m_minRunningSpeed = v[0];
	m_maxRunningSpeed = v[1];
	return 0;
}

int GameCharacter::script_minSneakingSpeed( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns min sneaking speed, m/s", funcName) );
	if ( m_minSneakingSpeed < 0.f )
		throw ScriptException( Format("setSneakingSpeedRange must be called before minSneakingSpeed") );
	vm->pushNumber( m_minSneakingSpeed );
	return 1;
}

int GameCharacter::script_minWalkingSpeed( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns min walking speed, m/s", funcName) );
	if ( m_minWalkingSpeed < 0.f )
		throw ScriptException( Format("setWalkingSpeedRange must be called before minWalkingSpeed") );
	vm->pushNumber( m_minWalkingSpeed );
	return 1;
}

int GameCharacter::script_minRunningSpeed( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns min running speed, m/s", funcName) );
	if ( m_minRunningSpeed < 0.f )
		throw ScriptException( Format("setRunningSpeedRange must be called before minRunningSpeed") );
	vm->pushNumber( m_minRunningSpeed );
	return 1;
}

int GameCharacter::script_maxSneakingSpeed( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns max sneaking speed, m/s", funcName) );
	if ( m_maxSneakingSpeed < 0.f )
		throw ScriptException( Format("setSneakingSpeedRange must be called before maxSneakingSpeed") );
	vm->pushNumber( m_maxSneakingSpeed );
	return 1;
}

int GameCharacter::script_maxWalkingSpeed( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns max walking speed, m/s", funcName) );
	if ( m_maxWalkingSpeed < 0.f )
		throw ScriptException( Format("setWalkingSpeedRange must be called before maxWalkingSpeed") );
	vm->pushNumber( m_maxWalkingSpeed );
	return 1;
}

int GameCharacter::script_maxRunningSpeed( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns max running speed, m/s", funcName) );
	if ( m_maxRunningSpeed < 0.f )
		throw ScriptException( Format("setRunningSpeedRange must be called before maxRunningSpeed") );
	vm->pushNumber( m_maxRunningSpeed );
	return 1;
}

int	GameCharacter::script_setSneakingControlRange( script::VM* vm, const char* funcName )
{
	float v[2];
	getParams( vm, funcName, "min and max sneaking controller range [0,1]", v, 2 );
	m_minSneakingControl = v[0];
	m_maxSneakingControl = v[1];
	return 0;
}

int	GameCharacter::script_setWalkingControlRange( script::VM* vm, const char* funcName )
{
	float v[2];
	getParams( vm, funcName, "min and max walking controller range [0,1]", v, 2 );
	m_minWalkingControl = v[0];
	m_maxWalkingControl = v[1];
	return 0;
}

int GameCharacter::script_setRunningControlRange( script::VM* vm, const char* funcName )
{
	float v[2];
	getParams( vm, funcName, "min and max running controller range [0,1]", v, 2 );
	m_minRunningControl = v[0];
	m_maxRunningControl = v[1];
	return 0;
}

int GameCharacter::script_minSneakingControl( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns min sneaking controller range [0,1]", funcName) );
	if ( m_minSneakingControl < 0.f )
		throw ScriptException( Format("setSneakingControlRange must be called before minSneakingControl") );
	vm->pushNumber( m_minSneakingControl );
	return 1;
}

int GameCharacter::script_minWalkingControl( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns min walking controller range [0,1]", funcName) );
	if ( m_minWalkingControl < 0.f )
		throw ScriptException( Format("setWalkingControlRange must be called before minWalkingControl") );
	vm->pushNumber( m_minWalkingControl );
	return 1;
}

int GameCharacter::script_minRunningControl( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns min running controller range [0,1]", funcName) );
	if ( m_minRunningControl < 0.f )
		throw ScriptException( Format("setRunningControlRange must be called before minRunningControl") );
	vm->pushNumber( m_minRunningControl );
	return 1;
}

int GameCharacter::script_maxSneakingControl( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns max sneaking controller range [0,1]", funcName) );
	if ( m_maxSneakingControl < 0.f )
		throw ScriptException( Format("setSneakingControlRange must be called before maxSneakingControl") );
	vm->pushNumber( m_maxSneakingControl );
	return 1;
}

int GameCharacter::script_maxWalkingControl( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns max walking controller range [0,1]", funcName) );
	if ( m_maxWalkingControl < 0.f )
		throw ScriptException( Format("setWalkingControlRange must be called before maxWalkingControl") );
	vm->pushNumber( m_maxWalkingControl );
	return 1;
}

int GameCharacter::script_maxRunningControl( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns max running controller range [0,1]", funcName) );
	if ( m_maxRunningControl < 0.f )
		throw ScriptException( Format("setRunningControlRange must be called before maxRunningControl") );
	vm->pushNumber( m_maxRunningControl );
	return 1;
}

int GameCharacter::script_setLookingHeadBoneTransformFix( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects anim name", funcName) );

	String name = vm->toString(1);

	m_headLookTransformFix = m_preparedSecondaryAnimations[name].bones["Bip01 Head"]->rotation().inverse();
	return 0;
}

int GameCharacter::script_setHeadTurnFixBlendDelay( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects head turning blend time", funcName) );

	m_headTurnFixBlendDelay = vm->toNumber(1);
	return 0;
}

int GameCharacter::script_groundMaterial( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns ground collision material", funcName) );

	vm->pushTable( groundMaterial() );
	return 1;
}

int GameCharacter::script_addPhysicalCombatMove( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_STRING, VM::TYPE_STRING, VM::TYPE_STRING, 
		VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER, 
		VM::TYPE_NUMBER, VM::TYPE_NUMBER, 
		VM::TYPE_NUMBER, VM::TYPE_NUMBER, 
		VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} adds a new physical combat move. Expects Name, Anim name (preloaded primary or secondary), Type; STRIKE/LOCK, Type; CONTACT/NONCONTACT, Height of hit, Amount of damage, Direction of force (angle in degrees, forward=0, angles increase counter-clockwise), Effect start time, Effect end time, Start angle (degrees) of attack sector (0=right, 90=forward, 180=left, 270=behind), End angle (degrees) of attack sector, Reach distance for this attack", funcName) );

	String name = vm->toString(1);
	String anim = vm->toString(2);
	String hitType = vm->toString(3);
	String contactType = vm->toString(4);
	float forceHeight = vm->toNumber(5);
	float forceAmount = vm->toNumber(6);
	float forceAngle = Math::toRadians(vm->toNumber(7));
	float startTime = vm->toNumber(8);
	float endTime = vm->toNumber(9);
	float startAngle = Math::toRadians( vm->toNumber(10) );
	float endAngle = Math::toRadians( vm->toNumber(11) );
	float reachDistance = vm->toNumber(11);
	int flags = 0;
	
// Test for erronous input
	if ( !m_anims->hasGroup( anim ) )
		throw ScriptException( Format("{0} was not able to find anim {1}", funcName, anim) );

	if ( hitType != "STRIKE" && hitType != "LOCK" )
		throw ScriptException( Format("{0} does not accept hit type {1}", funcName, hitType) );

	if ( contactType != "CONTACT" && contactType != "NONCONTACT" )
		throw ScriptException( Format("{0} does not accept contact type {1}", funcName, contactType) );

	if ( startTime > endTime )
		throw ScriptException( Format("{0} should be passed start time < end time", funcName) );

// Process flags 
	if ( m_preparedSecondaryAnimations.containsKey( anim ) )
		flags |= PhysicalCombatMove::PHYSICAL_MOVE_SECONDARY;
	else
		flags |= PhysicalCombatMove::PHYSICAL_MOVE_PRIMARY;

	if ( hitType == "STRIKE" )
		flags |= PhysicalCombatMove::PHYSICAL_MOVE_STRIKE;
	else if ( hitType == "LOCK" )
		flags |= PhysicalCombatMove::PHYSICAL_MOVE_LOCK;

	if ( contactType == "CONTACT" )
		flags |= PhysicalCombatMove::PHYSICAL_MOVE_CONTACT;
		
// Make force vector
	Vector3 force(0,0,forceAmount);
	force = force.rotate( Vector3(0,1,0), forceAngle );

// Add move
	m_physicalCombatMoves[name] = new PhysicalCombatMove( anim, flags, startTime, endTime, forceHeight, force, startAngle, endAngle, reachDistance );

	return 0; 
}

int GameCharacter::script_setBalanceLostThreshold( script::VM* vm, const char* funcName ) 
{ 
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} sets the threshold below which the character will lose balance.", funcName) );

	float threshold = vm->toNumber(1);

	if ( threshold < 0.f || threshold > 1.f )
		throw ScriptException( Format("{0} accepts values in range 0..1", funcName ) );

	m_balanceLostThreshold = threshold;

	return 0; 
}

int GameCharacter::script_setPhysicalHitRange( script::VM* vm, const char* funcName ) 
{ 
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} sets physical hit contact cylinder radius.", funcName) );

	m_hitRange = vm->toNumber(1);
	
	return 0; 
}

int GameCharacter::script_physicalAttack( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} attacks physically with specified move.", funcName) );

	String move = vm->toString(1);

	if ( !m_physicalCombatMoves.containsKey( move ) )
		throw ScriptException( Format("{0} must be passed a valid move. Can't find {1}", funcName, move ) );

	PhysicalCombatMove* technique = m_physicalCombatMoves[move];
	
	if ( technique->isSecondary() )
	{
		vm->pushString( "SECONDARY" );
	}
	else
	{
		vm->pushString( "PRIMARY" );
		setPrimaryState( PRIMARY_STATE_PHYSICAL_KICK );
	}

	m_currentPhysicalAttack = move;
	m_physicalAttackTimer = 0;
	technique->setHit( false );
	return 1;
}

int GameCharacter::script_getPhysicalAnim( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} returns anim associated with move.", funcName) );
	
	String name = vm->toString(1);

	if ( !m_physicalCombatMoves.containsKey( name ) )
		throw ScriptException( Format("{0} can not find physical combat move {1}.", funcName, name ) );
	
	vm->pushString( m_physicalCombatMoves[name]->animName() );
	return 1;
}

int	GameCharacter::script_endPhysicalAttack( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} ends current physical move", funcName) );
	
	m_currentPhysicalAttack = "";
	return 0;
}

int GameCharacter::script_getPrimaryStateEndTime( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} returns primary state end time, expects 1 string parameter", funcName) );
	
	// find state type
	const String stateName = vm->toString(1);
	PrimaryState state = findPrimaryState( stateName );

	// return end time
	vm->pushNumber( getStateEndTime( state ) );
	return 1;
}

int GameCharacter::script_getSecondaryStateEndTime( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} returns secondary state end time, expects 1 string parameter", funcName) );

	// find state type
	const String stateName = vm->toString(1);
	SecondaryState state = findSecondaryState( stateName );

	// return end time
	vm->pushNumber( getStateEndTime( state ) );
	return 1;
}

int GameCharacter::script_receiveDamage( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} reduces character health. Returns \"ALIVE\" if character died in the process, \"DEAD\" otherwise.", funcName) );

	float damage = vm->toNumber(1);
	StillAlive alive = receiveDamage( damage );
	if ( alive == CHARACTER_ALIVE )
	{
		vm->pushString( "ALIVE" );
	}
	else if ( alive == CHARACTER_DIED )
	{
		vm->pushString( "DEAD" );
	}
	else if ( alive == CHARACTER_ALREADYDEAD )
	{
		vm->pushString( "ALREADYDEAD" );
	}
	else
	{
		throw ScriptException( Format("{0} encountered an impossible character alive status.", funcName ) );
	}

	return 1; 
}

int GameCharacter::script_addControlSector( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects min angle (degrees, forward=0), max angle (degrees) and control vector length limit [0,1]. Angles increase clockwise and zero angle is forward.", funcName) );
	
	float minAngle = Math::toRadians( vm->toNumber(1) );
	float maxAngle = Math::toRadians( vm->toNumber(2) );
	float controlLimit = vm->toNumber(3);

	m_movementControlSectors.add( ControlSector(minAngle,maxAngle,controlLimit) );
	return 0;
}

int GameCharacter::script_getHealth( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns health", funcName ) );

	vm->pushNumber( m_health );
	return 1;
}

int GameCharacter::script_stopWorldSpaceAnimation( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} stops any active world animation", funcName) );
	
	m_worldAnim = 0;
	m_worldAnimTime = 0.f;
	return 0;
}

int GameCharacter::script_playWorldSpaceAnimation( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_STRING, VM::TYPE_STRING, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),2) )
		throw ScriptException( Format("{0} expects animation file name (.sg included), biped name and optional end behaviour (REPEAT,CONSTANT,OSCILLATE,RESET) and start time", funcName) );

	int		argc			= vm->top();
	String	sceneName		= vm->toString(1);
	String	bipedName		= vm->toString(2);
	String	endBehaviour	= argc > 1 ? vm->toString(3) : "REPEAT";
	float	startTime		= argc > 2 ? vm->toNumber(4) : 0.f;
	P(Node)	root			= m_sceneMgr->getScene( sceneName, SceneFile::LOAD_ANIMATIONS );

	Debug::println( "\"{0}\":playWorldSpaceAnimation( {1}, {2}, {3}, {4} )", name(), sceneName, bipedName, endBehaviour, startTime );

	m_worldAnim = NodeUtil::findNodeByName( root, bipedName );
	if ( !m_worldAnim )
		throw ScriptException( Format("{0}: Scene {1} does not contain biped animation {2}", funcName, sceneName, bipedName) );
	NodeUtil::setHierarchyEndBehaviour( m_worldAnim, Interpolator::toBehaviour(endBehaviour) );

	m_worldAnimTime = startTime;

	applyWorldSpaceAnimations( 0 );
	return 0;
}

int GameCharacter::script_isAiming( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns true (1) if character is aiming weapon, false (nil) otherwise", funcName ) );

	vm->pushBoolean( GameCharacter::isAiming( m_secondaryState ) );
	return 1;
}

int GameCharacter::script_isHurting( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns true (1) if character is hurting, false (nil) otherwise", funcName ) );

	vm->pushBoolean( GameCharacter::isHurting( m_primaryState ) );
	return 1;
}

int GameCharacter::script_isRolling( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns true (1) if character is rolling, false (nil) otherwise", funcName ) );

	vm->pushBoolean( GameCharacter::isRolling( m_primaryState ) );
	return 1;
}

int GameCharacter::script_primaryState( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns character primary state", funcName ) );

	vm->pushString( sm_primaryStates[m_primaryState].name );
	return 1;
}

int GameCharacter::script_secondaryState( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns character secondary state", funcName ) );

	vm->pushString( sm_secondaryStates[m_secondaryState].name );
	return 1;
}

int GameCharacter::script_addWeaponToInventory( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_TABLE};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects GameWeapon table", funcName) );
	
	GameWeapon* o = dynamic_cast<GameWeapon*>( getThisPtr(vm,1) );
	if ( !o )
		throw ScriptException( Format("{0} expects GameWeapon", funcName) );

	o->setPosition(0, Vector3(0,0,0));
	m_weaponInventory.add( o );
	return 0;
}

int GameCharacter::script_isReloading( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns true (1) if character is reloading its' weapon, false (nil) otherwise", funcName ) );

	vm->pushBoolean( m_secondaryState == SECONDARY_STATE_CHANGING_CLIP );
	return 1;
}

int GameCharacter::script_setPeekMoveCheckDistance( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects distance of movement in peek", funcName) );

	m_peekMoveCheckDistance = vm->toNumber(1);
	return 0;
}

int GameCharacter::script_cycleWeapon( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} cycles weapon to next weapon", funcName ) );

	cycleWeapon();
	return 0;
}

int GameCharacter::script_throwProjectile( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects projectile script name ", funcName) );

	if ( !m_meshThrowBone )
		throw ScriptException( Format("Call \"setThrowBone()\" before calling {0}", funcName ) );

	String script = vm->toString(1);

	Vector3 launchVector(0,0,0);
	rotation().rotate( m_lookVector, &launchVector );
	launchVector = launchVector.rotate( right(), -m_throwAngle );

	GameProjectile* emptyshell = m_projectileMgr->createProjectile( script, cell(), 0, m_meshThrowBone->cachedWorldTransform().translation(), launchVector);
	emptyshell->setGroundLightmapColor( m_groundLightmapColor );
	
	return 0;
}

int GameCharacter::script_setThrowBone( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects bone name", funcName) );
	
	String name = vm->toString(1);

	m_meshThrowBone = NodeUtil::findNodeByName( m_mesh, name );

	if ( !m_meshThrowBone ) 
		throw ScriptException( Format("{0} can not find Bone {1}", funcName, name ) );

	return 0;
}

int GameCharacter::script_setThrowAngle( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects amount of degrees to throw upwards", funcName) );

	m_throwAngle = Math::toRadians( vm->toNumber(1) );
	return 0;
}

int	GameCharacter::script_userControl( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns character user controller interface", funcName) );

	vm->pushTable( m_userControl );
	return 1;
}

int	GameCharacter::script_computerControl( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns character computer controller interface", funcName) );

	vm->pushTable( m_computerControl );
	return 1;
}

int GameCharacter::script_isSneaking( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns true if character is sneaking", funcName) );

	float ctrl = m_movementControlVector.length();
	if ( GameCharacter::isWalking(primaryState()) || GameCharacter::isStrafing(primaryState()) )
		vm->pushBoolean( ctrl > ControlBase::ZERO_CONTROL_VALUE && ctrl < maxSneakingControl() );
	else
		vm->pushBoolean( false );
	return 1;
}

int GameCharacter::script_isWalking( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns true if character is walking", funcName) );

	float ctrl = m_movementControlVector.length();
	if ( GameCharacter::isWalking(primaryState()) || GameCharacter::isStrafing(primaryState()) )
		vm->pushBoolean( ctrl >= minWalkingControl() && ctrl < maxWalkingControl() );
	else
		vm->pushBoolean( false );
	return 1;
}

int GameCharacter::script_isRunning( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns true if character is running", funcName) );

	float ctrl = m_movementControlVector.length();
	if ( GameCharacter::isWalking(primaryState()) || GameCharacter::isStrafing(primaryState()) )
		vm->pushBoolean( ctrl >= minRunningControl() );
	else
		vm->pushBoolean( false );
	return 1;
}

//-----------------------------------------------------------------------------

GameCharacter::SecondaryAnimationParams::SecondaryAnimationParams() :
	name(""),
	weight(0.f),
	bones( Allocator< HashtablePair<String, Node*> >(__FILE__) )
{
}
