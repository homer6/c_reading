#ifndef _SG_SPRITE_H
#define _SG_SPRITE_H


#include <sg/Primitive.h>
#include <math/Vector2.h>


namespace sg
{


class Material;
class TriangleList;
class Texture;


/**
 * Single sprite primitive.
 * Sprites are square texturemapped polgons always facing towards camera in 3D scene.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Sprite :
	public sg::Primitive
{
public:
	explicit Sprite( sg::Texture* tex, sg::Material* mat, sg::TriangleList* tri );

	Sprite( const Sprite& other, int shareFlags );
	
	~Sprite();

	Primitive* clone( int shareFlags ) const;

	/** Draws the primitive to the active rendering device. */
	void	draw();

	/** Uploads object to the rendering device. */
	void	load();

	/** Unloads object from the rendering device. */
	void	unload();

	/** Releases resources of the object. Object cannot be used after this. */
	void	destroy();

	/** Sets sprite position in screen space. */
	void	setPosition( const math::Vector2& v );

	/** Sets sprite rotation angle in radians. */
	void	setRotation( float angle );

	/** Sets sprite scaling in screen space. */
	void	setScale( const math::Vector2& v );

	/** Returns unscaled width of the sprite in pixels. */
	float	width() const;

	/** Returns unscaled height of the sprite in pixels. */
	float	height() const;

	/** Returns vertex format used by this geometry. */
	VertexFormat				vertexFormat() const;

	/** Returns position of the sprite. */
	const math::Vector2&		position() const;

	/** Returns sprite rotation angle in radians. */
	float						rotation() const;

	/** Returns scaling of the sprite. */
	const math::Vector2&		scale() const;

	/** Create material compatible with Sprite. */
	static sg::Material*		createMaterial();

	/** Create triangle list compatible with Sprite. */
	static sg::TriangleList*	createTriangleList( int verts=6 );

private:
	math::Vector2		m_pos;
	math::Vector2		m_scale;
	math::Vector2		m_size;
	float				m_rot;
	P(sg::TriangleList)	m_tri;

	static VertexFormat	defaultVertexFormat();

	Sprite( const Sprite& );
	Sprite& operator=( const Sprite& );
};


} // sg


#endif // _SG_SPRITE_H
