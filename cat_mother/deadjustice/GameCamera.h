#ifndef _GAMECAMERA_H
#define _GAMECAMERA_H


#include "GameObject.h"
#include "GameCharacter.h"
#include "ScriptMethod.h"
#include <sg/Camera.h>
#include <util/Vector.h>
#include <math/Vector2.h>


namespace sg {
	class Scene;
	class Context;}

namespace ps {
	class ParticleSystemManager;}

namespace sgu {
	class SceneManager;
	class NodeGroupSet;}

namespace snd {
	class SoundManager;}

class GameLevel;
class GameObject;
class GameCharacter;
class GamePointObject;


/**
 * In-game (player) camera with scriptable properties.
 *
 * Camera is connected to two target positions: 
 * Move target and Look target. Both positions are
 * defined in target object space. Move and Look targets
 * are connected to actual camera world-space translation and 
 * look-at position by damped springs. There are three springs for each
 * position, each spring controlling movement of one axis.
 *
 * @author Jani Kajala (jani.kajala@helsinki.fi), Toni Aittoniemi
 */
class GameCamera :
	public GameObject
{
public:
	/** 
	 * Creates a game camera using specified script virtual machine. 
	 * @param vm Script virtual machine.
	 */
	explicit GameCamera( script::VM* vm, io::InputStreamArchive* arch, 
		sgu::SceneManager* sceneMgr, snd::SoundManager* soundMgr, ps::ParticleSystemManager* particleMgr );

	///
	~GameCamera();

	/** Called when camera changes. */
	void		cameraChanged();

	/** 
	 * Update camera movement. 
	 * @exception ScriptException
	 */
	void		update( float dt );

	/** Renders cells and objects. */
	void		render( sg::Context* context, sg::Scene* root );

	/** Returns camera node used in rendering. */
	sg::Camera*	getRenderCamera();

	/** Sets camera target object. */
	void		setTarget( GameCharacter* target );

	/** Sets look pitch. */ 
	void		setPitch( float pitch );

	/** Returns true if the camera uses world-space target arenaing. */
	bool		worldSpaceControl() const;

	/** Returns look pitch. */
	float		pitch() const;

	/** 
	 * Returns turn threshold. 
	 * @param side 0=left, 1=right.
	 */
	float		turnThreshold( int side ) const;

	/** 
	 * Returns relative turn strength [0,1]. 
	 * @param side 0=left, 1=right.
	 */
	float		turnStrength( int side ) const;

	/** 
	 * Returns tilt(pitch) threshold. 
	 * @param side 0=up, 1=down.
	 */
	float		tiltThreshold( int side ) const;

	/** 
	 * Returns horizontal crosshair limit. 
	 * @param side 0=left, 1=right.
	 */
	float		horizontalLimit( int side ) const;

	/** 
	 * Return vertical crosshair limit. 
	 * @param side 0=up, 1=down.
	 */
	float		verticalLimit( int side ) const;

	/** Return crosshair center (Screenspace (not pixelspace!) relative from center of screen). */
	math::Vector2	crosshairCenter() const;

	/** Prints primitives. */
	void		printPrimitives() const;

	/** Returns world point in screen space. */
	math::Vector3	getScreenPoint( const math::Vector3& worldPoint ) const;

	/** Returns number of rendered frames. */
	static int		frameCount();

private:
	struct Viewport
	{
		float x0;
		float y0;
		float x1;
		float y1;
	};

	/** Target character primary/secondary state based offset in character space. */
	class CharacterStateOffset
	{
	public:
		GameCharacter::PrimaryState		primaryState;
		GameCharacter::SecondaryState	secondaryState;
		math::Vector3					moveOffset;
		math::Vector3					lookOffset;
		float							blendTime;

		CharacterStateOffset();

		/** Returns true if primary and secondary states are equal. */
		bool	operator==( const CharacterStateOffset& other ) const;

		/** Returns true if primary or secondary states are inequal. */
		bool	operator!=( const CharacterStateOffset& other ) const;
	};

	int								m_methodBase;
	static ScriptMethod<GameCamera>	sm_methods[];

	P(sgu::SceneManager)		m_sceneMgr;
	P(sg::Node)					m_background;

	P(sg::Camera)				m_camera;
	float						m_hfov;
	float						m_front;
	float						m_back;
	P(GameCharacter)			m_target;
	bool						m_worldSpaceControl;
	float						m_timeScale;
	math::Vector3				m_moveTargetLocal;
	math::Vector3				m_lookTargetLocal;
	bool						m_worldTargetsDirty;
	bool						m_firstUpdate;			// true for one frame after reset()

	// Pitching (tilt)
	float						m_lookPitch;			
	float						m_pitchStrengthPos;
	float						m_pitchStrengthNeg;

	// Targeting zones
	float						m_turnThreshold[2];		
	float						m_turnStrength[2];
	float						m_tiltThreshold[2];
	float						m_verticalLimit[2];
	float						m_horizontalLimit[2];	
	math::Vector2				m_crosshairOffset;		// from green center

	util::Vector<math::Vector3> m_posAvg;
	util::Vector<math::Vector3> m_lookAvg;
	int							m_posAvgCount;

	math::Vector3				m_moveTarget;
	math::Vector3				m_moveTargetVelocity;
	math::Vector3				m_moveSpringConst;
	math::Vector3				m_moveDampingConst;
	math::Vector3				m_postPitchMove;
								
	math::Vector3				m_lookTarget;
	math::Vector3				m_lookTargetVelocity;
	math::Vector3				m_lookSpringConst;
	math::Vector3				m_lookDampingConst;
								
	float						m_blendTime;
	float						m_blendTimeEnd;
	math::Matrix4x4				m_blendTm;

	util::Vector<P(sg::Node)>	m_renderNodes;
	util::Vector<GameObject*>	m_renderObjects;
	util::Vector<GameCell*>		m_renderCells;
	float						m_minVisiblePortalSize;
	math::Vector2				m_normalizedPixelSize;

	P(GamePointObject)			m_visibilityChecker;

	// cut scene animations
	P(sg::Camera)				m_anim;
	float						m_animTime;
	float						m_cutSceneAspectRatio;

	// target state based offset
	util::Vector<CharacterStateOffset>	m_targetStateOffsets;
	CharacterStateOffset				m_newTargetStateOffset;
	CharacterStateOffset				m_previousTargetStateOffset;
	float								m_targetStateOffsetBlendTime;
	float								m_targetStateOffsetBlendTimeEnd;

	static int					sm_frameCount;

	/** 
	 * Returns character-state based camera target move/look offsets in target local space. 
	 * Returned offsets are blended so that quick changes in character state do not cause jerks in camera.
	 */
	void			getBlendedTargetCharacterStateOffset( float dt, math::Vector3* moveOffset, math::Vector3* lookOffset );

	/** Returns character-state based camera target move/look offsets in target local space. */
	void			getTargetCharacterStateOffset( CharacterStateOffset* targetOffset ) const;

	/**
	 * Finds camera animation with specified name.
	 * @exception ScriptException If camera animation not found.
	 */
	sg::Camera*		findCameraAnimation( const lang::String& animName ) const;

	/** Applies camera animation to the camera. */
	void			applyCameraAnimation();

	/** 
	 * Returns camera animation length (seconds) by animation name. 
	 * @exception ScriptException If camera animation not found.
	 */
	float			getCameraAnimationLength( const lang::String& animName ) const;

	static bool		testPointsVolume( 
						const math::Vector3* points, int pointCount,
						const math::Vector4* planes, int planeCount );

	void			checkCollisionsAgainstCell( const math::Vector3& start, const math::Vector3& delta, 
						bsp::BSPNode* bsptree, GameCell* collisionCell, CollisionInfo* cinfo );

	void			renderRecurse( GameCell* cell, sg::Scene* root, const Viewport& viewport,
						const math::Matrix4x4& viewTm, const math::Matrix4x4& projTm );

	void			prepareRender( sg::Context* context, sg::Scene* root, sg::Camera* camera );

	int				methodCall( script::VM* vm, int i );
	int				script_playAnimation( script::VM* vm, const char* funcName );
	int				script_stopAnimation( script::VM* vm, const char* funcName );
	int				script_getAnimationStartTime( script::VM* vm, const char* funcName );
	int				script_getAnimationEndTime( script::VM* vm, const char* funcName );
	int				script_addTargetStateOffset( script::VM* vm, const char* funcName );
	int				script_setAverageCount( script::VM* vm, const char* funcName );
	int				script_setBlendTime( script::VM* vm, const char* funcName );
	int				script_setHorizontalFov( script::VM* vm, const char* funcName );
	int				script_setLookTarget( script::VM* vm, const char* funcName );
	int				script_setLookSpring( script::VM* vm, const char* funcName );
	int				script_setLookDamping( script::VM* vm, const char* funcName );
	int				script_setMoveTarget( script::VM* vm, const char* funcName );
	int				script_setMoveSpring( script::VM* vm, const char* funcName );
	int				script_setMoveDamping( script::VM* vm, const char* funcName );
	int				script_setWorldSpaceControl( script::VM* vm, const char* funcName );
	int				script_setTimeScale( script::VM* vm, const char* funcName );
	int				script_setFront( script::VM* vm, const char* funcName );
	int				script_setBack( script::VM* vm, const char* funcName );
	int				script_setPitchAmountUp( script::VM* vm, const char* funcName );
	int				script_setPitchAmountDown( script::VM* vm, const char* funcName );
	int				script_setPitchThresholdUp( script::VM* vm, const char* funcName );
	int				script_setPitchThresholdDown( script::VM* vm, const char* funcName );
	int				script_setTurnThresholdLeft( script::VM* vm, const char* funcName );
	int				script_setTurnThresholdRight( script::VM* vm, const char* funcName );
	int				script_setTurnStrengthLeft( script::VM* vm, const char* funcName );
	int				script_setTurnStrengthRight( script::VM* vm, const char* funcName );
	int				script_setCrosshairOffset( script::VM* vm, const char* funcName );
	int				script_setCrosshairLimitLeft( script::VM* vm, const char* funcName );
	int				script_setCrosshairLimitRight( script::VM* vm, const char* funcName );
	int				script_setCrosshairLimitUp( script::VM* vm, const char* funcName );
	int				script_setCrosshairLimitDown( script::VM* vm, const char* funcName );
	int				script_setMinPortalVisibleSize( script::VM* vm, const char* funcName );
	int				script_setPostPitchMove( script::VM* vm, const char* funcName );
	int				script_setCutSceneWideScreenRatio( script::VM* vm, const char* funcName );
	int				script_stopCameraAnimation( script::VM* vm, const char* funcName );

	GameCamera( const GameCamera& );
	GameCamera& operator=( const GameCamera& );
};


#endif // _GAMECAMERA_H
