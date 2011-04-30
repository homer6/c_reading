#ifndef _GAMECHARACTER_H
#define _GAMECHARACTER_H


#include "Blender.h"
#include "GameObject.h"
#include "GameWeapon.h"
#include "ScriptMethod.h"
#include "ControlSector.h"
#include "CollisionInfo.h"
#include "MorphAnimation.h"
#include "AnimationParams.h"
#include "BoneCollisionBox.h"
#include "MovementAnimation.h"
#include "PhysicalCombatMove.h"
#include <sg/Mesh.h>
#include <sg/Light.h>
#include <pix/Colorf.h>
#include <lang/String.h>
#include <util/Vector.h>
#include <math/Vector2.h>
#include <util/Hashtable.h>


namespace sg {
	class LOD;
	class LineList;
	class Shader;
	class Camera;}

namespace ps {
	class ParticleSystemManager;}

namespace sgu {
	class SceneManager;
	class NodeGroupSet;}

namespace bsp {
	class BSPNode;}


class GameCell;
class GameLevel;
class CollisionInfo;
class ProjectileManager;
class GameNoise;
class GameSurface;
class GameNoiseManager;
class GameProjectile;
class GameSphereObject;
class GamePointObject;
class ComputerControl;
class UserControl;


/** 
 * Class for player characters. Each character is
 * associated with a script which implements character type specific
 * functionality.
 *
 * Character state is evaluated by code. When the new state
 * is different from the previous one, a script event is called.
 * Script functions control for example simulation parameters,
 * and which animations and effects are played back at different states etc.
 * See GamePlayer::State for list of states and sm_stateEvents
 * for associated script functions.
 *
 * Note about the mesh: Mesh should be cloned from the template.
 * (the same normal mesh can be multiple times in the scene)
 *
 * @author Jani Kajala (jani.kajala@helsinki.fi), Toni Aittoniemi
 */
class GameCharacter :
	public GameObject
{
public:
	/** Primary states. */
	enum PrimaryState
	{
		PRIMARY_STATE_UNKNOWN,
		PRIMARY_STATE_STANDING,
		PRIMARY_STATE_WALKING,
		PRIMARY_STATE_WALKING_BACKWARD,
		PRIMARY_STATE_STRAFING,
		PRIMARY_STATE_JUMPING,
		PRIMARY_STATE_ROLLING_FORWARD,
		PRIMARY_STATE_ROLLING_BACKWARD,
		PRIMARY_STATE_ROLLING_LEFT,
		PRIMARY_STATE_ROLLING_RIGHT,
		PRIMARY_STATE_CROUCHING_DOWN,
		PRIMARY_STATE_CROUCHED,
		PRIMARY_STATE_CROUCHED_WALKING,
		PRIMARY_STATE_UNCROUCHING,
		PRIMARY_STATE_PEEKING_LEFT,
		PRIMARY_STATE_UNPEEKING_LEFT,
		PRIMARY_STATE_PEEKING_RIGHT,
		PRIMARY_STATE_UNPEEKING_RIGHT,
		PRIMARY_STATE_FALLING,
		PRIMARY_STATE_INAIR,
		/** Hurting of a received projectile */
		PRIMARY_STATE_PROJECTILE_HURT,
		/** Striking a physically contacting enemy */
		PRIMARY_STATE_PHYSICAL_CONTACT_STRIKE,
		/** Kicking physically */
		PRIMARY_STATE_PHYSICAL_KICK,
		/** Grabbing & holding physically */
		PRIMARY_STATE_PHYSICAL_LOCK,
		/** Hurting of a physical attack */
		PRIMARY_STATE_PHYSICAL_HURT,
		/** In physical lock */
		PRIMARY_STATE_PHYSICAL_INLOCK,
		PRIMARY_STATE_DEAD,
		PRIMARY_STATE_CUSTOM,
		PRIMARY_STATE_COUNT
	};

	/** Secondary states. */
	enum SecondaryState
	{
		SECONDARY_STATE_UNKNOWN,
		/** Doing nothing */
		SECONDARY_STATE_IDLING,
		/** Looking */
		SECONDARY_STATE_LOOKING,
		/** Firing weapon */
		SECONDARY_STATE_ATTACKING,
		/** Starts aiming before shooting */
		SECONDARY_STATE_AIMING,
		/** Holding aiming after shooting */
		SECONDARY_STATE_HOLDING_AIM,
		/** Re-loading weapon */
		SECONDARY_STATE_CHANGING_CLIP,
		/** Striking a physically contacting enemy */
		SECONDARY_STATE_CYCLING_WEAPON,
		/** Striking a physically contacting enemy */
		SECONDARY_STATE_PHYSICAL_CONTACT_STRIKE,
		/** Striking physically */
		SECONDARY_STATE_PHYSICAL_STRIKE,
		/** Grabbing & holding physically */
		SECONDARY_STATE_PHYSICAL_LOCK,
		/** Hurting of a received projectile while aiming */
		SECONDARY_STATE_PROJECTILE_HURT_WHILE_AIMING,
		/** Throwing empty shell to divert enemy */
		SECONDARY_STATE_THROWING_EMPTY_SHELL,				
		SECONDARY_STATE_DEAD,
		SECONDARY_STATE_COUNT
	};

	/** Character state (listener) flags. */
	enum StateFlags
	{
		/** Signal state listener when the state becomes active. */
		STATE_ENTRY			= 1,
		/** Signal state listener when the state becomes inactive. */
		STATE_EXIT			= 2,
		/** Wait for state listener before re-evaluating the player state. */
		STATE_WAIT			= 4,
		/** Remove state listener after state changes. */
		STATE_REMOVE		= 8,
	};

	/** Character control source. */
	enum ControlSource
	{
		CONTROL_USER,
		CONTROL_COMPUTER
	};

	/** Character life status. */
	enum StillAlive
	{
		CHARACTER_ALIVE,
		CHARACTER_DIED,
		CHARACTER_ALREADYDEAD
	};

	/** 
	 * Creates a game player using specified script virtual machine. 
	 * @param sceneMgr Used to load required meshes.
	 * @param anims Character animation container.
	 */
	GameCharacter( script::VM* vm, io::InputStreamArchive* arch, sgu::NodeGroupSet* anims,
		sgu::SceneManager* sceneMgr, snd::SoundManager* soundMgr, 
		ps::ParticleSystemManager* particleMgr, ProjectileManager* projectileMgr,
		GameNoiseManager* noiseMgr );

	///
	~GameCharacter();

	/** Enables/disables turning state transition. Default is disabled. */
	void		setTurningState( bool enabled );

	/** Resets player control input state. */
	void		resetInputState();

	/** Aim at worldspace position. */
	void		aimAt( const math::Vector3& target );

	/** Set crosshair position. */
	void		setCrosshair( const math::Vector2& pos );

	/** Look at worldspace position. */
	void		lookTo( const math::Vector3& target );

	/** 
	 * Update player movement. 
	 * @exception Exception
	 * @exception ScriptException
	 */
	void		update( float dt, bool firstUpdateInFrame );

	/** Returns object to be used in rendering. */
	sg::Node*	getRenderObject( sg::Camera* camera );

	/** Sets player primary state. */
	void		setPrimaryState( PrimaryState state );

	/** Sets player secondary state. */
	void		setSecondaryState( SecondaryState state );

	/** Sets current weapon */
	void		setWeapon( GameWeapon* weapon );

	/** Cycles next weapon from inventory */
	void		cycleWeapon();

	/** Sets control source, user or computer. */
	void		setControlSource( ControlSource controllingParty );

	/** Sets invulnerability */
	void		setInvulnerable( bool state );

	/** Called when a projectile hits the character. */
	void		receiveProjectile( GameProjectile* projectile, const math::Vector3& collisionPoint, const BoneCollisionBox& collisionBone );

	/** Called when a physical strike attack hits the character. */
	void		receivePhysicalStrike( const math::Vector3& from, const math::Vector3& force );

	/** Called when character receives damage. */
	StillAlive	receiveDamage( float damage );

	/** Returns current weapon. */
	GameWeapon*	weapon() const;

	/** Returns true if character has weapon. */
	bool		hasWeapon() const;

	/** Returns true if the player is on steep surface. */
	bool		onSteepSurface() const;

	/** Returns true if character is visible from specified world point. */
	bool		isVisibleFrom( const math::Vector3& worldPos ) const;

	/** Returns true if character torso is visible from specified world point. */
	bool		isTorsoVisibleFrom( const math::Vector3& worldPos ) const;

	/** Returns ptr to user control. */
	UserControl*	userControl() const;

	/** Returns ptr to computer control. */
	ComputerControl*	computerControl() const;

	/** Returns player primary state. */
	PrimaryState	primaryState() const;

	/** Returns time elapse in primary state. */
	float			primaryStateTime() const;

	/** Returns player secondary state. */
	SecondaryState	secondaryState() const;

	/** Returns time elapse in secondary state. */
	float			secondaryStateTime() const;

	/** Returns string description about character state. */
	lang::String	stateString() const;

	/** Returns string description about character primary animation state. */
	lang::String	animationPrimaryStateString() const;

	/** Returns string description about character secondary animation state. */
	lang::String	animationSecondaryStateString() const;

	/** Returns string description about character morph animation state. */
	lang::String	morphStateString() const;

	/** Returns string description about character input state. */
	lang::String	inputStateString() const;

	/** 
	 * Returns object center point to be used in collision checks. 
	 * Center point is different from object position,
	 * players have center point between head and feet, but
	 * position is at feet.
	 */
	virtual math::Vector3	center() const;

	/** Returns aim center. */
	math::Vector3			aimCenter() const;

	/** Returns aim vector. */
	const math::Vector3&	aimVector() const;

	/** Returns aim vector. */
	const math::Vector3&	lookVector() const;

	/** Returns hit center. */
	math::Vector3			hitCenter() const;

	/** Returns bone position in world space. */
	math::Vector3			getBoneWorldPosition( sg::Node* bone ) const;

	/** Returns bone transform in world space. */
	math::Matrix4x4			getBoneWorldTransform( sg::Node* bone ) const;

	/** Returns character (biped) head position in world space. */
	math::Vector3			headWorldPosition() const;

	/** 
	 * Returns character (biped) head transform in world space. 
	 * Transform is converted so that Z is forward, X right and Y up.
	 */
	math::Matrix4x4			headWorldTransform() const;

	/** Returns max speed when walking crouched. */
	float					crouchWalkingSpeed() const;

	/** Returns max speed when strafing crouched. */
	float					crouchStrafingSpeed() const;

	/** Returns max speed when walking backwards crouched. */
	float					crouchBackwardSpeed() const;

	/** Returns max forward rolling speed. */
	float					rollingSpeedForward() const;

	/** Returns max backward rolling speed. */
	float					rollingSpeedBackward() const;

	/** Returns max sideways rolling speed. */
	float					rollingSpeedSideways() const;

	/** Returns health. */
	float					health() const;

	/** Returns ith bone collision box. */
	const BoneCollisionBox&	getBoneCollisionBox( int i ) const;

	/** 
	 * Returns bone collision box by name. 
	 * @exception Exception if bone collision box not found.
	 */
	const BoneCollisionBox&	getBoneCollisionBox( const lang::String& boneName ) const;

	/** Returns number of bone collision boxes. */
	int						boneCollisionBoxes() const;

	/** Returns top-level bone collision box (contains whole character). */
	const BoneCollisionBox&	rootCollisionBox() const;

	/** Returns current ground lightmap color. */
	const pix::Colorf&		groundLightmapColor() const;

	/** Returns ground collision material. */
	GameSurface*			groundMaterial() const;

	/** Returns character collision checker helper object. */
	GamePointObject*		visibilityCollisionChecker() const;

	/** Returns number of seconds to aim after shooting. */
	float					aimingTimeAfterShooting() const;

	/** Returns true if aim blending is completed. */
	bool					readyToShoot() const;

	/** Returns true if character can move to specified (world space) direction (collision volume check). */
	bool					canMove( const math::Vector3& delta ) const;

	/** Returns true if character can move to specified (world space) direction (collision volume check). */
	bool					canMoveLine( const math::Vector3& delta ) const;

	/** Returns head bone. */
	sg::Node*				headBone() const;

	/** Returns max sneaking speed m/s. */
	float					maxSneakingSpeed() const;

	/** Returns max walking speed m/s. */
	float					maxWalkingSpeed() const;

	/** Returns max running speed m/s. */
	float					maxRunningSpeed() const;

	/** Returns min sneaking speed m/s. */
	float					minSneakingSpeed() const;

	/** Returns min walking speed m/s. */
	float					minWalkingSpeed() const;

	/** Returns min running speed m/s. */
	float					minRunningSpeed() const;

	/** Returns max sneaking control [0,1]. */
	float					maxSneakingControl() const;

	/** Returns max walking control [0,1]. */
	float					maxWalkingControl() const;

	/** Returns max running control [0,1]. */
	float					maxRunningControl() const;

	/** Returns min sneaking control [0,1]. */
	float					minSneakingControl() const;

	/** Returns min walking control [0,1]. */
	float					minWalkingControl() const;

	/** Returns min running control [0,1]. */
	float					minRunningControl() const;

	/** Returns ith movement control sector which limit control ranges in specified directions. */
	const ControlSector&	getMovementControlSector( int i ) const;

	/** Returns number of movement control sectors which limit control ranges in specified directions. */
	int						movementControlSectors() const;

	/** Returns probability that this character can see the other one. */
	float					canSeeProbability( GameCharacter* other ) const;

	/** Returns range from which the character can be hit with a physical attack. */
	float					hitRange() const;

	/** Returns true if character is invulnerable. */
	bool					invulnerable() const;

	/** Returns amount of meters moved in peek. */
	float					peekMoveCheckDistance() const;

	/** Returns distance when character capsule collides another capsule. */
	float					characterCollisionRadius() const;

	/**
	 * Returns true if state is such that turning can be applied
	 */
	static bool				canTurn( PrimaryState state );

	/** 
	 * Returns true if player can be attacked against. 
	 * not CUSTOM, not FALLING.
	 */
	static bool				isAttackable( PrimaryState state );

	/**
	 * Returns true if player is hurting
	 */

	static bool				isHurting( PrimaryState state );
	
	/**
	 * Returns true when primary state is STANDING.
	 */
	static bool				isStanding( PrimaryState state );

	/**
	 * Returns true when primary state is WALKING.
	 */
	static bool				isWalking( PrimaryState state );

	/**
	 * Returns true if signals are enabled for the state.
	 * not CUSTOM.
	 */
	static bool				isSignalsEnabled( PrimaryState state );

	/** 
	 * Returns true if player can be controlled in the state. 
	 * not FALLING
	 */
	static bool				isControllable( PrimaryState state );

	/**
	 * Returns true when state is JUMPING.
	 */
	static bool				isJumping( PrimaryState state );

	/**
	 * Returns true when character is rolling.
	 */
	static bool				isRolling( PrimaryState state );

	/** 
	 * Returns true when state is CROUCHING_DOWN, CROUCHED or UNCROUCHING
     */
	static bool				isCrouched( PrimaryState state );

	/**
	 * Returns true when state is INAIR or JUMPING.
	 */
	static bool				isInAir( PrimaryState state );

	/**
	 * Returns true when strafing left or right.
	 */
	static bool				isStrafing( PrimaryState state );

	/**
	 * Returns true when falling
	 */
	static bool				isFalling( PrimaryState state );

	/**
	 * Returns true when dead
	 */
	static bool				isDead( PrimaryState state );

	/**
	 * Returns true when attacking (= firing weapon)
	 */
	static bool				isAttacking( SecondaryState state );

	/**
	 * Returns true when physical attacking
	 */
	static bool				isPhysicalAttacking( SecondaryState state );

	/*
	 * Returns true when physical attacking
	 */
	static bool				isPhysicalAttacking( PrimaryState state );

	/**
	 * Returns true when aiming
	 */
	static bool				isAiming( SecondaryState state );

	/**
	 * Returns true when peeking.
	 */
	static bool				isPeeking( PrimaryState state );

	/**
	 * Returns true if head turning fix is enabled for this secondary state.
	 */
	static bool				isHeadTurnFixEnabled( SecondaryState state );

	/** 
	 * Converts string to primary state.
	 * @exception Exception If state name not valid.
	 */
	static PrimaryState		toPrimaryState( const lang::String& str );

	/** 
	 * Converts string to secondary state.
	 * @exception Exception If state name not valid.
	 */
	static SecondaryState	toSecondaryState( const lang::String& str );

	friend class MovementAnimation;

private:
	struct StateInfo
	{
		const char*		name;
		int				flags;
	};
	
	struct StateListener
	{
		int				scriptFuncRef;
		float			time;
		int				flags;			// see StateFlags
	};

	struct AnimationListener
	{
		int					scriptFuncRef;
		lang::String		animationName;
		float				time;
	};

	struct SecondaryAnimationParams
	{
		lang::String								name;
		float										weight;
		util::Hashtable< lang::String, sg::Node* >	bones;
	
		SecondaryAnimationParams();
	};

	// scripting
	int										m_methodBase;
	static ScriptMethod<GameCharacter>		sm_methods[];

	// script independent variables
	P(io::InputStreamArchive)		m_arch;
	P(snd::SoundManager)			m_soundMgr;
	P(ps::ParticleSystemManager)	m_particleMgr;
	P(sgu::SceneManager)			m_sceneMgr;
	P(ProjectileManager)			m_projectileMgr;
	P(GameNoiseManager)				m_noiseMgr;
	P(GameNoise)					m_movementNoise;

	P(sgu::NodeGroupSet)			m_anims;
	lang::String					m_animationFolderName;
	P(Blender)						m_primaryBlender;
	P(Blender)						m_secondaryBlender;

	PrimaryState					m_primaryState;
	float							m_primaryStateTime;
	SecondaryState					m_secondaryState;
	float							m_secondaryStateTime;

	// collisions
	float							m_steepAngleCos;	// max angle cos for ground plane
	math::Vector4					m_groundPlane;
	math::Vector3					m_slideNormal;
	float							m_maxStepHeight;	// max step height to ignore
	int								m_inAirCounter;
	CollisionInfo					m_groundCollisionInfo;
	P(GamePointObject)				m_visibilityChecker;
	P(GameSphereObject)				m_collisionChecker;
	P(GamePointObject)				m_collisionCheckerPoint;
	util::Vector<BoneCollisionBox>	m_boneCollisionBoxes;
	BoneCollisionBox				m_rootCollisionBox;
	float							m_characterCollisionRadius;
	math::Vector3					m_offset;

	// visuals
	P(sg::Node)				m_mesh;
	P(sg::Node)				m_meshHeadBone;			// shortcut to NodeUtil::findNodeByName
	P(sg::Node)				m_meshNeckBone;			// shortcut to NodeUtil::findNodeByName
	P(sg::Node)				m_meshLeftFootBone;		// shortcut to NodeUtil::findNodeByName
	P(sg::Node)				m_meshRightFootBone;	// shortcut to NodeUtil::findNodeByName
	P(sg::Node)				m_meshLeftForeArmBone;	// shortcut to NodeUtil::findNodeByName
	P(sg::Node)				m_meshRightForeArmBone;	// shortcut to NodeUtil::findNodeByName
	P(sg::Node)				m_meshLeftUpperArmBone;	// shortcut to NodeUtil::findNodeByName
	P(sg::Node)				m_meshRightUpperArmBone;// shortcut to NodeUtil::findNodeByName
	P(sg::Node)				m_meshSpineBone;
	P(sg::Node)				m_meshThrowBone;		// shortcut to NodeUtil::findNodeByName
	P(sg::LOD)				m_lod;
	pix::Colorf				m_groundLightmapColor;

	// player state
	util::Vector< P(util::Vector<StateListener>) > m_primaryStateListeners;
	util::Vector< P(util::Vector<StateListener>) > m_secondaryStateListeners;
	static const StateInfo	sm_primaryStates[];
	static const StateInfo	sm_secondaryStates[];

	// animation control
	util::Vector< AnimationParams >		m_animParamBuffer;
	util::Vector< anim::Animatable* >	m_nodeBuffer;
	util::Vector< float >				m_timeBuffer;
	util::Vector< float >				m_weightBuffer;
	util::Vector< lang::String >		m_nameBuffer;

	// movement animation control pointers
	util::Hashtable<lang::String, math::Vector3*>			m_controlPtrTable;

	// primary animation 
	float													m_primaryTime;
	float													m_primarySpeed;
	int														m_primaryAnimBlenderID;
	util::Vector<AnimationListener>							m_animListeners;
	util::Hashtable<lang::String, P(MovementAnimation)>		m_movementAnimations;
	lang::String											m_activeMovementAnimation;

	// secondary animation
	float													m_secondaryTime;
	float													m_secondarySpeed;
	int														m_secondaryAnimBlenderID;
	util::Hashtable<lang::String, P(MovementAnimation)>		m_secondaryMovementAnimations;
	lang::String											m_activeSecondaryMovementAnimation;
	util::Hashtable<lang::String, SecondaryAnimationParams>	m_preparedSecondaryAnimations;

	// head looking transform fix
	math::Matrix3x3											m_headLookTransformFix;
	float													m_headTurnFixBlendDelay;
	float													m_headTurnFixBlendTime;
	int														m_headTurnFixBlendTimeDirection;

	// morph animation
	util::Vector<P(MorphAnimation)>	m_morphAnims;
	util::Vector<P(sg::Mesh)>		m_morphBases;

	// simulation params
	float					m_rotationSpeed;			// rotation speed, radians/s
	float					m_friction;					// relative speed decrease in second
	float					m_fallingFriction;			// friction when falling
	float					m_maxBackwardSpeed;			// m/s
	float					m_gravity;					// gravity strenght, G's
	float					m_crouchWalkingSpeed;		// m/s
	float					m_crouchStrafingSpeed;		// m/s
	float					m_crouchBackwardSpeed;		// m/s
	math::Vector3			m_movementVector;			// total movement in second
	math::Vector3			m_movementControlVector;	// controller movement input vector
	math::Vector3			m_aimVector;				// vector from player's gun to target
	math::Vector3			m_lookVector;				// vector from head to target
	math::Vector3			m_aimCrosshairNormalized;	// normalized crosshair pos (clamped to left&right turn limits and up&down crosshair limits)
	float					m_rollingSpeedForward;		// m/s
	float					m_rollingSpeedBackward;		// m/s
	float					m_rollingSpeedSideways;		// m/s
	float					m_rollingSpeedMinimum;		// m/s
	float					m_aimingTimeAfterShooting;	// seconds
	float					m_peekMoveCheckDistance;	// m (used for checking if character can peek)
	float					m_throwAngle;				// throw angle upwards

	// Physical combat
	float													m_balance;					// range 0..1
	float													m_balanceLostThreshold;		// range 0..1
	float													m_hitRange;					// m
	float													m_strikeRange;				// m
	util::Hashtable<lang::String, P(PhysicalCombatMove)>	m_physicalCombatMoves;

	lang::String									m_currentPhysicalAttack;
	float											m_physicalAttackTimer;

	// character movement speeds
	float					m_minSneakingSpeed;
	float					m_maxSneakingSpeed;
	float					m_minWalkingSpeed;
	float					m_maxWalkingSpeed;
	float					m_minRunningSpeed;
	float					m_maxRunningSpeed;

	// character movement controller ranges
	float					m_minSneakingControl;
	float					m_maxSneakingControl;
	float					m_minWalkingControl;
	float					m_maxWalkingControl;
	float					m_minRunningControl;
	float					m_maxRunningControl;
	util::Vector<ControlSector>	m_movementControlSectors;

	// control
	ControlSource				m_controlSource;	
	// computer input state		
	P(ComputerControl)			m_computerControl;
	// user input state			
	P(UserControl)				m_userControl;
								
	// Zero vector				
	math::Vector3				m_none;
	
	// Weapon
	P(GameWeapon)				m_weapon;
	util::Vector<P(GameWeapon)>	m_weaponInventory;
	int							m_currentWeapon;

	// Health & hit bone
	bool						m_invulnerable;
	float						m_health;
	sg::Node*					m_hitBone;

	// time since last received projectile
	float						m_timeSinceLastProjectile;					

	// time since last taunt
	float						m_timeSinceLastTaunt;					

	// world space (cut scene) animations
	P(sg::Node)					m_worldAnim;
	float						m_worldAnimTime;

	// anim
	float	animLength( const lang::String& animationName ) const;

	// listeners
	void	signalAnimationListeners( const lang::String& animationName, float time, float dt );
	void	signalStateListeners( PrimaryState state, int flagsMask, float timeStart, float timeEnd );
	void	signalStateListeners( SecondaryState state, int flagsMask, float timeStart, float timeEnd );

	// update
	void	updateNoise( float dt );
	void	updateState( float dt );
	void	updateVelocity( float dt );
	void	updateTransform( float dt, bool firstUpdateInFrame );
	void	updateAnimationState( float dt );
	void	updateWeapon( float dt );
	void	updatePhysicalCombat( float dt );
	void	updateShaderParameters();
	void	setShaderParams( sg::Shader* fx, sg::Mesh* mesh );
	void	applyTransformAnimations( sg::Camera* camera );
	void	updateMorphAnimations( float dt );
	void	applyMorphAnimations( sg::Camera* camera );
	void	playMorphAnimation( const lang::String& animName );
	bool	hasMorphAnimation( const lang::String& animName ) const;
	void	applyWorldSpaceAnimations( sg::Camera* camera );
	MorphAnimation* addMorphAnimation( const lang::String& baseName, const lang::String& animName, anim::Interpolator::BehaviourType behaviour );

	// collisions
	void	checkCollisionsAgainstCell( const math::Vector3& start, const math::Vector3& delta, bsp::BSPNode* bsptree, GameCell* collisionCell, CollisionInfo* cinfo );
	void	checkCollisionsAgainstObjects( const math::Vector3& start, const math::Vector3& delta, const util::Vector<GameObjectDistance>& objects, CollisionInfo* cinfo );
	void	findSlidingPlane( const CollisionInfo& cinfo );
	void	findGroundPlane();
	bool	isVisibleLine( const math::Vector3& worldPos, const math::Vector3& start, const math::Vector3& end, int firstSampleIndex, int samples ) const;
	bool	isVisiblePoint( const math::Vector3& worldPos, const math::Vector3& point ) const;
	void	sampleGroundColor( const math::Vector3& point );

	// AI
	void	signalReceiveProjectileFSM( GameProjectile* projectile );
	void	signalHearNoiseFSM( GameNoise* noise );
	void	listenNoise();

	// animations

	/** 
	 * Returns 'end time' for given state.
	 * End time means when it is allowed to re-evaluate player state.
	 * All states have end time of 0, for example
	 * ACCELERATING can be interrupted at any time.
	 */
	float	getStateEndTime( PrimaryState state ) const;
	float	getStateEndTime( SecondaryState state ) const;

	/** Removes and unrefs all state listeners. */
	void	removeStateListeners();

	/** Removes and unrefs all animation listeners. */
	void	removeAnimationListeners();

	/** Checks morph state validity. */
	bool	morphValid() const;
	
	/** Finds state by name. Returns STATE_UNKNOWN if not found. */
	static PrimaryState		findPrimaryState( const lang::String& stateName );
	static SecondaryState	findSecondaryState( const lang::String& stateName );

	PrimaryState	evaluatePrimaryState();
	SecondaryState	evaluateSecondaryState();

	// scripting
	int		methodCall( script::VM* vm, int i );
	int		script_activateMovementAnimation( script::VM* vm, const char* funcName );	
	int		script_activateSecondaryMovementAnimation( script::VM* vm, const char* funcName );	
	int		script_addControlSector( script::VM* vm, const char* funcName );
	int		script_addAnimation( script::VM* vm, const char* funcName );
	int		script_addAnimationListener( script::VM* vm, const char* funcName );
	int		script_addCollisionBone( script::VM* vm, const char* funcName );
	int		script_addMorphAnimation( script::VM* vm, const char* funcName );
	int		script_addPhysicalCombatMove( script::VM* vm, const char* funcName );
	int		script_addPrimaryStateListener( script::VM* vm, const char* funcName );
	int		script_addSecondaryAnimation( script::VM* vm, const char* funcName );	
	int		script_addSecondaryStateListener( script::VM* vm, const char* funcName );
	int		script_addWeaponToInventory( script::VM* vm, const char* funcName );
	int		script_aimAt( script::VM* vm, const char* funcName );
	int		script_blendAnimation( script::VM* vm, const char* funcName );
	int		script_blendMorphAnimation( script::VM* vm, const char* funcName );
	int		script_blendSecondaryAnimation( script::VM* vm, const char* funcName );
	int		script_canSee( script::VM* vm, const char* funcName ); 
	int		script_canMove( script::VM* vm, const char* funcName ); 
	int		script_clearSecondaryAnimations( script::VM* vm, const char* funcName ); 
	int		script_createMovementAnimation( script::VM* vm, const char* funcName ); 
	int		script_createSecondaryMovementAnimation( script::VM* vm, const char* funcName ); 
	int		script_cycleWeapon( script::VM* vm, const char* funcName ); 
	int		script_computerControl( script::VM* vm, const char* funcName );
	int		script_setHeadTurnFixBlendDelay( script::VM* vm, const char* funcName );
	int		script_evaluatePrimaryState( script::VM* vm, const char* funcName );
	int		script_evaluateSecondaryState( script::VM* vm, const char* funcName );
	int		script_getAnimLength( script::VM* vm, const char* funcName );
	int		script_getComputerControl( script::VM* vm, const char* funcName );
	int		script_getFriction( script::VM* vm, const char* funcName );
	int		script_getPrimaryStateEndTime( script::VM* vm, const char* funcName );
	int		script_getHealth( script::VM* vm, const char* funcName );
	int		script_getSecondaryStateEndTime( script::VM* vm, const char* funcName );
	int		script_isAiming( script::VM* vm, const char* funcName );
	int		script_isCrouched( script::VM* vm, const char* funcName );
	int		script_isDead( script::VM* vm, const char* funcName );
	int		script_isHurting( script::VM* vm, const char* funcName );
	int		script_isReloading( script::VM* vm, const char* funcName );
	int		script_isRolling( script::VM* vm, const char* funcName );
	int		script_playMorphAnimation( script::VM* vm, const char* funcName );
	int		script_playMorphAnimationOnce( script::VM* vm, const char* funcName );
	int		script_playWorldSpaceAnimation( script::VM* vm, const char* funcName );
	int		script_resetInputState( script::VM* vm, const char* funcName );
	int		script_setSecondaryMovementAnimation( script::VM* vm, const char* funcName );	
	int		script_setAcceleration( script::VM* vm, const char* funcName );
	int		script_setAnimationFolder( script::VM* vm, const char* funcName );
	int		script_setAnimation( script::VM* vm, const char* funcName );
	int		script_setBalanceLostThreshold( script::VM* vm, const char* funcName ); // NEW
	int		script_setCharacterCollisionRadius( script::VM* vm, const char* funcName );
	int		script_setCrouchWalkingSpeed( script::VM* vm, const char* funcName );
	int		script_setCrouchStrafingSpeed( script::VM* vm, const char* funcName );
	int		script_setCrouchBackwardSpeed( script::VM* vm, const char* funcName );
	int		script_setFallingFriction( script::VM* vm, const char* funcName );
	int		script_setFriction( script::VM* vm, const char* funcName );
	int		script_setGravity( script::VM* vm, const char* funcName );
	int		script_setHealth( script::VM* vm, const char* funcName );
	int		script_setLOD( script::VM* vm, const char* funcName );
	int		script_setLookingHeadBoneTransformFix( script::VM* vm, const char* funcName );
	int		script_setMaxAnimSlewRatePrimary( script::VM* vm, const char* funcName );
	int		script_setMaxAnimSlewRateSecondary( script::VM* vm, const char* funcName );
	int		script_setMesh( script::VM* vm, const char* funcName );
	int		script_setMorphBase( script::VM* vm, const char* funcName );
	int		script_setPhysicalHitRange( script::VM* vm, const char* funcName );
	int		script_setPhysicalStrikeRange( script::VM* vm, const char* funcName );
	int		script_setRotationSpeed( script::VM* vm, const char* funcName );
	int		script_setRollingSpeedForward( script::VM* vm, const char* funcName );
	int		script_setRollingSpeedBackward( script::VM* vm, const char* funcName );
	int		script_setRollingSpeedSideways( script::VM* vm, const char* funcName );
	int		script_setRollingSpeedMinimum( script::VM* vm, const char* funcName );
	int		script_setPrimaryState( script::VM* vm, const char* funcName );
	int		script_setSecondaryState( script::VM* vm, const char* funcName );
	int		script_setSteepSurface( script::VM* vm, const char* funcName );
	int		script_setWeapon( script::VM* vm, const char* funcName );
	int		script_setAimingTimeAfterShooting( script::VM* vm, const char* funcName );
	int		script_stateControllable( script::VM* vm, const char* funcName );
	int		script_stateFalling( script::VM* vm, const char* funcName );
	int		script_stateInAir( script::VM* vm, const char* funcName );
	int		script_stopWorldSpaceAnimation( script::VM* vm, const char* funcName );
	int		script_weapon( script::VM* vm, const char* funcName );
	int		script_setMaxStrafeControl( script::VM* vm, const char* funcName );
	int		script_setMaxBackwardControl( script::VM* vm, const char* funcName );
	int		script_setMaxRelativeStrafeMovement( script::VM* vm, const char* funcName );
	int		script_setMaxRelativeBackwardMovement( script::VM* vm, const char* funcName );
	int		script_setSneakingSpeedRange( script::VM* vm, const char* funcName );
	int		script_setWalkingSpeedRange( script::VM* vm, const char* funcName );
	int		script_setRunningSpeedRange( script::VM* vm, const char* funcName );
	int		script_setPeekMoveCheckDistance( script::VM* vm, const char* funcName );
	int		script_groundMaterial( script::VM* vm, const char* funcName );
	int		script_minSneakingSpeed( script::VM* vm, const char* funcName );
	int		script_minWalkingSpeed( script::VM* vm, const char* funcName );
	int		script_minRunningSpeed( script::VM* vm, const char* funcName );
	int		script_maxSneakingSpeed( script::VM* vm, const char* funcName );
	int		script_maxWalkingSpeed( script::VM* vm, const char* funcName );
	int		script_maxRunningSpeed( script::VM* vm, const char* funcName );
	int		script_setSneakingControlRange( script::VM* vm, const char* funcName );
	int		script_setWalkingControlRange( script::VM* vm, const char* funcName );
	int		script_setRunningControlRange( script::VM* vm, const char* funcName );
	int		script_minSneakingControl( script::VM* vm, const char* funcName );
	int		script_minWalkingControl( script::VM* vm, const char* funcName );
	int		script_minRunningControl( script::VM* vm, const char* funcName );
	int		script_maxSneakingControl( script::VM* vm, const char* funcName );
	int		script_maxWalkingControl( script::VM* vm, const char* funcName );
	int		script_maxRunningControl( script::VM* vm, const char* funcName );
	int		script_physicalAttack( script::VM* vm, const char* funcName );		 
	int		script_getPhysicalAnim( script::VM* vm, const char* funcName );		 
	int		script_endPhysicalAttack( script::VM* vm, const char* funcName );
	int		script_receiveDamage( script::VM* vm, const char* funcName );
	int		script_throwProjectile( script::VM* vm, const char* funcName );
	int		script_setThrowBone( script::VM* vm, const char* funcName );
	int		script_setThrowAngle( script::VM* vm, const char* funcName );
	int		script_userControl( script::VM* vm, const char* funcName );
	int		script_primaryState( script::VM* vm, const char* funcName );		 
	int		script_secondaryState( script::VM* vm, const char* funcName );		 
	int		script_isSneaking( script::VM* vm, const char* funcName );		 
	int		script_isWalking( script::VM* vm, const char* funcName );		 
	int		script_isRunning( script::VM* vm, const char* funcName );		 

	GameCharacter( const GameCharacter& );
	GameCharacter& operator=( const GameCharacter& );
};


#endif // _GAMECHARACTER_H
