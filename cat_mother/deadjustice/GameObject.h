#ifndef _GAMEOBJECT_H
#define _GAMEOBJECT_H


#include "GameScriptable.h"
#include "GameObjectListItem.h"
#include <math/Vector3.h>
#include <math/Matrix3x3.h>
#include <math/Matrix4x4.h>
#include <util/Vector.h>


class CollisionInfo;
class GameCell;
class GameLevel;
class GameSurface;
class GameNoiseManager;

namespace io {
	class InputStreamArchive;
	class InputStream;}

namespace sg {
	class Camera;
	class Node;}

namespace ps {
	class ParticleSystemManager; }

namespace bsp {
	class BSPNode;
	class BSPPolygon; }

namespace snd {
	class SoundManager; }

namespace sgu {
	class SceneManager;}


/** 
 * Base functionality for all game objects.
 * Any object can be scriptable.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GameObject :
	public GameScriptable
{
public:
	/** Complete movement state of game object. */
	class MovementState
	{
	public:
		math::Matrix3x3		rot;
		math::Vector3		pos;
		math::Vector3		vel;

		MovementState();
	};

	/** Helper class for dynamic collisions. */
	class GameObjectDistance
	{
	public:
		float		distanceSquared;
		GameObject*	object;

		bool operator<( const GameObjectDistance& other ) const		{return distanceSquared < other.distanceSquared;}
		bool operator==( const GameObjectDistance& other ) const	{return object == other.object;}
	};

	/** Rotation fix for character studio exported objects. */
	static const math::Matrix3x3	CHARACTER_MESH_PRE_ROTATION;

	/** Creates game object with scripting support. */
	explicit GameObject( script::VM* vm, io::InputStreamArchive* arch, 
		snd::SoundManager* soundMgr, ps::ParticleSystemManager* particleMgr,
		GameNoiseManager* noiseMgr );

	///
	~GameObject();

	/** 
	 * Updates object state. Default implementation does nothing.
	 * @param dt Update interval in seconds.
	 * @exception ScriptException
	 */
	virtual void	update( float dt );

	/** 
	 * Returns object to be used in rendering. Call base class implementation
	 * when overriding this as getRenderObject may be used for other purposes also (as updating animations).
	 * @param camera Pointer to camera if any. If 0 then scene is not rendered but the object is requested for other purposes.
	 */
	virtual sg::Node* getRenderObject( sg::Camera* camera );

	/** Called when explosion hits the object. Calls script signalReceiveExplosion(damage,dirx,diry,dirz,dist). */
	void			receiveExplosion( GameObject* source, float damage );

	/** Sets object transform in a cell. */
	void			setTransform( GameCell* cell, const math::Matrix4x4& tm );

	/** Sets game object position in a cell. */
	void			setPosition( GameCell* cell, const math::Vector3& pos );

	/** Sets object rotation. */
	void			setRotation( const math::Matrix3x3& rot );

	/** Sets object velocity (m/s). */
	void			setVelocity( const math::Vector3& vel );

	/** Increments render count by one. */
	void			incrementRenderCount();

	/** Set rendered-in-frame mark. */
	void			setRenderedInFrame( int frame );

	/** Set to true if object can be collided against. */
	void			setCollidable( bool collidable )								{m_collidable=collidable;}

	/** Returns true if object can be collided against. */
	bool			collidable() const												{return m_collidable;}

	/** Returns current cell. */
	GameCell*		cell() const;

	/** Returns true if object is in cell. */
	bool			inCell() const;

	/** Returns current level. */
	GameLevel*		level() const;

	/** Returns true if object is hidden. Hidden objects don't get rendered. */
	bool			hidden() const													{return m_hidden;}

	/** 
	 * Move in level by delta. 
	 * @param delta Vector from current position to end position.
	 * @param cinfo [out] Receives information about collision point (if not 0).
	 */
	void			move( const math::Vector3& delta, CollisionInfo* cinfo );

	/** Move in level by delta without colliding to anything. */
	void			moveWithoutColliding( const math::Vector3& delta );

	/** 
	 * Checks collision of this object against BSP tree. 
	 * Updates CollisionInfo only if collision happens.
	 */
	virtual void	checkCollisionsAgainstCell( const math::Vector3& start, const math::Vector3& delta, bsp::BSPNode* bsptree, GameCell* collisionCell, CollisionInfo* cinfo );

	/** 
	 * Checks collisions against dynamic objects. 
	 * Updates CollisionInfo only if collision happens.
	 */
	virtual void	checkCollisionsAgainstObjects( const math::Vector3& start, const math::Vector3& delta, const util::Vector<GameObjectDistance>& objects, CollisionInfo* cinfo );

	/** Aligns object rotation relative to specified up direction. */
	void			alignRotation( const math::Vector3& up );

	/**
	 * When a (C++) method is called from a script, this function
	 * is executed and unique method identifier is passed as parameter.
	 * Derived classes must override this if they add new scriptable methods.
	 * @param vm Script virtual machine executing the method.
	 * @param i Unique identifier (index) of the called method.
	 * @return Number of arguments returned in the script stack.
	 */
	virtual int		methodCall( script::VM* vm, int i );

	/** Sets object bounding sphere. Default is 0. */
	void			setBoundSphere( float radius );

	/** Rotates object so that its forward() vector points at specified position. */
	void			lookAt( const math::Vector3& target );

	/** Sets object movement state. */
	void			setMovementState( const MovementState& ms );

	/** Sets true if object is visible. Used by GameCamera. */
	void			setVisible( bool visible );

	/** Forces object to be visible. Used in non-continuous camera transform changes in cut scenes. */
	void			forceVisible();

	/** Returns angle to point in radians. */
	float			getAngleTo( const math::Vector3& pos ) const;

	/** Returns signed angle to point in radians. */
	float			getSignedAngleTo( const math::Vector3& pos ) const;

	/** Returns signed absolute angle in world coordinates to point in radians. */
	float			getSignedWorldAngleTo( const math::Vector3& pos ) const;

	/** Returns object position. */
	const math::Vector3&	position() const;

	/** 
	 * Returns object center point to be used in collision checks. 
	 * Center point might be different from object position,
	 * for example players have center point between head and feet, but
	 * position is at feet.
	 */
	virtual math::Vector3	center() const;

	/** Returns object rotation. */
	const math::Matrix3x3&	rotation() const;

	/** Returns object transform. */
	math::Matrix4x4			transform() const;

	/** Returns object rotation X-axis. */
	math::Vector3	right() const;

	/** Returns object rotation Y-axis. */
	math::Vector3	up() const;

	/** Returns object rotation Z-axis. */
	math::Vector3	forward() const;

	/** 
	 * Returns floor normal at object position 
	 * or (0,0,0) if the object is not on the arena surface. 
	 */
	math::Vector3	normal() const;

	/** Returns object velocity (m/s). */
	const math::Vector3&	velocity() const;

	/** Returns object movement state. */
	const MovementState&	movementState() const;

	/** Returns object bounding sphere radius. */
	float			boundSphere() const;

	/** Returns object speed (m/s). */
	float			speed() const;

	/** Returns true if point is in front of this object. */
	bool			isInFront( const math::Vector3& point ) const;

	/** Returns true if point is on the right side of this object. */
	bool			isOnRightSide( const math::Vector3& point ) const;

	/** Returns number of times this object has been rendered. */
	int				renderCount() const;

	/** Returns true if object was visible in last rendering. */
	bool			visible() const;

	/** Returns rendered-in-frame mark or -1 if object has not been rendered. */
	int				renderedInFrame() const;

	/** Returns collision material type from collision info. */
	GameSurface*	getCollisionMaterial( const CollisionInfo& cinfo ) const;

	/** Returns key light (if any) information for this object. */
	virtual sg::Light*	keylight() const;

	/** Returns noise manager or 0 if the object can't cause noises. */
	GameNoiseManager*	noiseManager() const;

	/** 
	 * Gets GameObject table or 3-vector passed as a parameter to a scriptable function. 
	 * @return Index of next parameter on VM stack after extracted parameters.
	 */
	static int		getGameObjectOrVector3( script::VM* vm, const char* funcName,
						GameObject** obj, math::Vector3* pos, int param=1 );

private:
	int									m_methodBase;
	static ScriptMethod<GameObject>		sm_methods[];

	MovementState		m_ms;
	float				m_boundSphere;
	GameCell*			m_cell;
	GameObjectListItem	m_primaryCellItem;
	P(GameNoiseManager) m_noiseMgr;
	int					m_renderCount;
	int					m_renderedInFrame;
	bool				m_visible;
	bool				m_hidden;
	bool				m_collidable;

	// dynamic shadow
	P(sg::Mesh)			m_shadowMesh;
	lang::String		m_shadowName;
	float				m_shadowLength;
	bool				m_shadowDirty;
	P(sg::Light)		m_keylight;

	/** Sets object cell. */
	void	setCell( GameCell* cell );

	/** Move and recurse trough portals, does not check collisions. */
	void	moveWithoutCollidingRecurse( GameCell* cell, const math::Vector3& pos, const math::Vector3& delta, GameCell** targetCell );

	/** Move and recurse trough portals, check collisions against room BSPs. */
	void	moveRecurse( GameCell* cell, const math::Vector3& pos, const math::Vector3& delta, CollisionInfo* cinfo );

	/** Updates m_keylight. */
	void	findKeylight();

	int		script_addLocalVelocity( script::VM* vm, const char* funcName );
	int		script_addWorldVelocity( script::VM* vm, const char* funcName );
	int		script_alignRotation( script::VM* vm, const char* funcName );		
	int		script_boundSphere( script::VM* vm, const char* funcName );
	int		script_cell( script::VM* vm, const char* funcName );
	int		script_enableDynamicShadow( script::VM* vm, const char* funcName );
	int		script_disableDynamicShadow( script::VM* vm, const char* funcName );
	int		script_getAngleTo( script::VM* vm, const char* funcName );
	int		script_getDistanceTo( script::VM* vm, const char* funcName );
	int		script_getGameObject( script::VM* vm, const char* funcName );
	int		script_getLocalVelocity( script::VM* vm, const char* funcName );
	int		script_getPosition( script::VM* vm, const char* funcName );
	int		script_getSignedAngleTo( script::VM* vm, const char* funcName );
	int		script_getSignedWorldAngleTo( script::VM* vm, const char* funcName );
	int		script_getVelocity( script::VM* vm, const char* funcName );
	int		script_getVelocityLength( script::VM* vm, const char* funcName );
	int		script_getUp( script::VM* vm, const char* funcName );
	int		script_getRight( script::VM* vm, const char* funcName );
	int		script_getForward( script::VM* vm, const char* funcName );	
	int		script_hidden( script::VM* vm, const char* funcName );
	int		script_hide( script::VM* vm, const char* funcName );
	int		script_isOnRightSide( script::VM* vm, const char* funcName );
	int		script_isInFront( script::VM* vm, const char* funcName );
	int		script_lookAt( script::VM* vm, const char* funcName );
	int		script_projectPositionOnForward( script::VM* vm, const char* funcName );	
	int		script_projectPositionOnRight( script::VM* vm, const char* funcName );	
	int		script_rotateY( script::VM* vm, const char* funcName );
	int		script_setDynamicShadow( script::VM* vm, const char* funcName );
	int		script_setBoundSphere( script::VM* vm, const char* funcName );
	int		script_setCollisionMask( script::VM* vm, const char* funcName );
	int		script_setPosition( script::VM* vm, const char* funcName );
	int		script_setRotationToIdentity( script::VM* vm, const char* funcName );
	int		script_setVelocity( script::VM* vm, const char* funcName );
	int		script_speed( script::VM* vm, const char* funcName );
	int		script_unhide( script::VM* vm, const char* funcName );

	GameObject( const GameObject& );
	GameObject& operator=( const GameObject& );
};


#include "GameObject.inl"


#endif // _GAMEOBJECT_H
