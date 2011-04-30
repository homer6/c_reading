#ifndef _SG_NODE_H
#define _SG_NODE_H


#include <lang/String.h>
#include <util/Vector.h>
#include <math/Matrix4x4.h>
#include <anim/Animatable.h>


namespace anim {
	class Control;}


namespace sg
{


class Camera;


/**
 * Base class for all objects in scene graph. Node contains model
 * transformation and object hierarchy information.
 *
 * Note that even though nodes are reference counted, 
 * references to parent are weak, i.e. parent can be destroyed
 * even though child is still referenced by the user. In that case
 * the child is automatically unlinked from the parent.
 *
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Node :
	public anim::Animatable
{
public:
	/** Node state flags. */
	enum NodeFlags
	{
		/** Node is enabled. */
		NODE_ENABLED				= 1,
		/** Node is renderable. */
		NODE_RENDERABLE				= 2,
		/** Node world transform is dirty. */
		NODE_WORLDTMDIRTY			= 4,
		/** Node was rendered in last frame. */
		NODE_RENDEREDINLASTFRAME	= 8,
		/** Default flags for node. */
		NODE_DEFAULTS				= -1
	};

	///
	Node();

	///
	~Node();

	/** Copy by value. Clones children. */
	Node( const Node& other );

	/** Returns clone of the object. Clones also children. */
	virtual Node*			clone() const;

	/** Sets name of this node. */
	void					setName( const lang::String& name );

	/** 
	 * Set node enabled/disabled. 
	 * Non-renderable nodes and their children don't get rendered.
	 */
	void					setEnabled( bool enabled );

	/** 
	 * Set node renderable/non-renderable. 
	 * Disabled nodes don't get rendered, but their children are rendered.
	 */
	void					setRenderable( bool enabled );

	/** Sets model-to-parent node transform. Rotation can have scaling. Animatable. */
	void					setTransform( const math::Matrix4x4& transform );

	/** Sets model-to-parent node position. Animatable. */
	void					setPosition( const math::Vector3& position );

	/** Sets model-to-parent node rotation. Rotation can have scaling. Animatable. */
	void					setRotation( const math::Matrix3x3& rotation );

	/** 
	 * Makes this node rotation to point at other node in world space.
	 * @param target Target node.
	 * @param up Up direction in world space.
	 */
	void					lookAt( const Node* other, const math::Vector3& up = math::Vector3(0,1,0) );

	/** 
	 * Makes this node rotation to point at target position in world space.
	 * @param target Target position in world space.
	 * @param up Up direction in world space.
	 */
	void					lookAt( const math::Vector3& target, const math::Vector3& up = math::Vector3(0,1,0) );

	/** 
	 * Links this as a child to parent node. 
	 * Increments reference counts of both nodes.
	 */
	void					linkTo( Node* parent );

	/** 
	 * Unlinks this from parent() node. 
	 * Decrements reference counts of both nodes.
	 */
	void					unlink();

	/** Sets node animation state. */
	void					blendState( anim::Animatable** anims,
								const float* times, const float* weights, int n );

	/** Computes object visibility in the view frustum. Default is false. */
	virtual bool			updateVisibility( sg::Camera* camera );

	/** 
	 * Renders node to the active device. Can be called multiple times if
	 * multiple passes are needed to render the scene.
	 * @param pass The current rendering pass.
	 */
	virtual void			render( Camera* camera, int pass );

	/** 
	 * Sets position controller. Pass 0 to remove previous controller. 
	 * Position controllers must have 3 channels.
	 */
	void					setPositionController( anim::Control* control );

	/** 
	 * Sets rotation controller. Pass 0 to remove previous controller. 
	 * Rotation controllers must have 4 channels (quaternion).
	 */
	void					setRotationController( anim::Control* control );

	/** 
	 * Sets scale controller. Pass 0 to remove previous controller. 
	 * Scale controllers must have 3 channels.
	 */
	void					setScaleController( anim::Control* control );

	/** 
	 * Validates cached world transforms for the node hierarchy recursively. 
	 * Used in rendering.
	 */
	void					validateHierarchy( bool forceUpdate=false ) const;

	/** Returns name of this node. */
	const lang::String&		name() const;

	/** 
	 * Returns true if the node is enabled. 
	 * Disabled nodes and their children don't get rendered.
	 */
	bool					enabled() const;

	/** 
	 * Returns true if node is renderable. 
	 * Non-renderable nodes don't get rendered, but their children are rendered.
	 */
	bool					renderable() const;

	/** Returns current transform in parent space. Rotation can have scaling. */
	const math::Matrix4x4&	transform() const;

	/** Returns current rotation in parent space. Rotation can have scaling. */
	math::Matrix3x3			rotation() const;

	/** Returns current position in parent space. Rotation can have scaling. */
	math::Vector3			position() const;
	
	/** Returns current model-to-world transform. */
	const math::Matrix4x4&	worldTransform() const;

	/** Returns the parent of this node. Returns 0 if no parent set. */
	Node*					parent() const;
	
	/** Returns the first child node. Returns 0 if no children. */
	Node*					firstChild() const;
	
	/** Returns the next child node. Returns 0 if the child is the last one. */
	Node*					getNextChild( const Node* child ) const;
	
	/** Returns the root of the node hierarchy. */
	Node*					root() const;

	/** 
	 * Returns 'next' node in the scene graph hierarchy. 
	 * This function can be used to enumerate all objects in the scene graph.
	 * Iteration is performed in child-first-then-sibling order.
	 * Note that to iterate throught the scene graph the return
	 * value of this function must be used to ask for next node.
	 * @param childFlags Flags required for node child to be traversed. See NodeFlags.
	 */
	Node*					nextInHierarchy( int childFlags=0 ) const;

	/** 
	 * Returns cached distance to camera along Z axis when the object was last rendered.
	 * Returns 0 if the object hasn't been rendered.
	 */
	float					cachedDistanceToCamera() const;

	/** Returns true if the object was visible last time it was rendered. */
	bool					renderedInLastFrame() const;

	/** 
	 * Returns cached world transform. 
	 * Use only if you know cached world transform to be valid.
	 * (i.e. setTransform of this node has not been called since last call to worldTransform)
	 */
	const math::Matrix4x4&	cachedWorldTransform() const;

	/** Returns position controller if any. */
	anim::Control*			positionController() const;

	/** Returns rotation controller if any. */
	anim::Control*			rotationController() const;

	/** Returns scale controller if any. */
	anim::Control*			scaleController() const;

	/** Returns bounding radius of the node. Default is 0. */
	virtual float			boundSphere() const;

	/**
	 * Returns true if cached world transform is valid. 
	 * Cached world transform is validated by calling validateHierarchy() for the root node.
	 */
	bool		cachedWorldTransformValid() const;

private:
	friend class Camera;

	/** 
	 * Hints for speeding up transform animation state update. 
	 * (hints are indices of previously referenced animation keys)
	 */
	struct TransformHint
	{
		int	posHint;
		int rotHint;
		int scaleHint;
	};

	lang::String					m_name;
	math::Matrix4x4					m_localTransform;
	mutable math::Matrix4x4			m_worldTransform;
	mutable int						m_flags;
	float							m_distanceToCamera;

	Node*							m_parent;
	P(Node)							m_child;
	P(Node)							m_next;
	Node*							m_previous;

	P(anim::Control)				m_posCtrl;
	P(anim::Control)				m_rotCtrl;
	P(anim::Control)				m_scaleCtrl;
	util::Vector<TransformHint>		m_tmHints;

	void		defaults();
	void		destroy();
	void		assign( const Node& other );
	bool		hasParent( const Node* other ) const;

	/** Set true by camera if the node is visible in this frame. */
	void		setRenderedInLastFrame( bool enabled );

	Node& operator=( const Node& );
};


#include "Node.inl"


} // sg


#endif // _SG_NODE_H
