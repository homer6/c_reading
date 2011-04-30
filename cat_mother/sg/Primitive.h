#ifndef _SG_PRIMITIVE_H
#define _SG_PRIMITIVE_H


#include <sg/VertexFormat.h>
#include <sg/ContextObject.h>


namespace math {
	class Vector3;
	class Vector4;
	class Matrix4x4;}


namespace sg
{


class Shader;
class ViewFrustum;


/** 
 * Abstract base for visible primitives.
 * Primitives are used by adding them to Mesh nodes.
 * Mesh node is a container of primitives in the scene graph.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Primitive :
	public ContextObject
{
public:
	/** Flags for sharing part of the primitive. */
	enum ShareFlags
	{
		/** Share nothing. */
		SHARE_NOTHING	= 0,
		/** Share shader. */
		SHARE_SHADER	= 1,
		/** Share geometry. */
		SHARE_GEOMETRY	= 2,
	};

	///
	Primitive();

	///
	virtual ~Primitive();

	/** 
	 * Returns clone of the primitive.
	 * @see ShareFlags
	 */
	virtual Primitive*	clone( int shareFlags=SHARE_NOTHING ) const = 0;

	/** Draws the primitive to the active rendering device. */
	virtual void	draw() = 0;

	/** Uploads object to the rendering device. */
	//virtual void	load() = 0;

	/** Unloads object from the rendering device. */
	//virtual void	unload() = 0;

	/** Releases resources of the object. Object cannot be used after this. */
	//virtual void	destroy() = 0;

	/** Computes object visibility in the view frustum. Default is true. */
	virtual bool	updateVisibility( const math::Matrix4x4& modelToCamera, 
						const ViewFrustum& viewFrustum );

	/** 
	 * Sets the shader used to render this primitive. 
	 * Primitive needs to have a shader to be rendered.
	 */
	void			setShader( Shader* shader );

	/** 
	 * Returns primitive bounding sphere. Default is 0. 
	 */
	virtual float	boundSphere() const;

	/** 
	 * Returns number of used bones by this primitive.
	 * Default is 0.
	 */
	virtual int		usedBones() const;

	/** 
	 * Returns array of used bones by this primitive.
	 * If the primitive has no bones the return value is 0.
	 */
	virtual const int*	usedBoneArray() const;

	/** Returns vertex format used by this geometry. */
	virtual VertexFormat	vertexFormat() const = 0;

	/** 
	 * Returns the shader used to render this primitive. 
	 * If the primitive does not have shader set return value is 0.
	 */
	Shader*			shader() const;

protected:
	Primitive( const Primitive& other, int shareFlags );

private:
	P(Shader)	m_shader;

	Primitive( const Primitive& );
	Primitive& operator=( const Primitive& );
};


} // sg


#endif // _SG_PRIMITIVE_H
