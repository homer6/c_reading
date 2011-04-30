#ifndef _GMMATERIAL_H
#define _GMMATERIAL_H


#include <pix/Colorf.h>
#include <lang/String.h>
#include <lang/Object.h>
#include <util/Vector.h>


namespace io {
	class ChunkOutputStream;}



/**
 * Geometry material.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GmMaterial :
	public lang::Object
{
public:
	/** Describes which texture layers are enabled, see layerFlags(). */
	enum LayerFlags
	{
		/** Material has diffuse layer. */
		L_DIFF			= 1,
		/** Material has reflection layer. */
		L_REFL			= 2,
		/** Material has glossiness layer. */
		L_GLOS			= 4,
		/** Material has bump layer. */
		L_BUMP			= 8,
		/** Material has lightmap layer. */
		L_LGHT			= 16,
		/** Material has specular color layer. */
		L_SPEC_COLOR	= 32,
		/** Material has specular level layer. */
		L_SPEC_LEVEL	= 64,
	};

	enum BlendType
	{
		BLEND_COPY,
		BLEND_ADD,
		BLEND_MULTIPLY
	};

	enum EnvType
	{
		ENV_NONE,
		ENV_SPHERICAL,
		ENV_CYLINDRICAL,
		ENV_SHRINKWRAP,
		ENV_SCREEN
	};

	class TextureLayer
	{
	public:
		/** File name of the bitmap. */
		lang::String	filename;
		/** Index of the used geometry texture coordinate set. */
		int				coordset;
		/** Texture filtering on/off. */
		bool			filter;
		/** Environment mapping type if any. */
		EnvType			env;
		/** Environment mapping parameter. */
		float			uoffs;
		/** Environment mapping parameter. */
		float			voffs;
		/** Environment mapping parameter. */
		float			uscale;
		/** Environment mapping parameter. */
		float			vscale;

		TextureLayer();
		
		bool	operator==( const TextureLayer& other ) const;
		bool	operator!=( const TextureLayer& other ) const;
	};

	lang::String		name;
	float				opacity;
	float				selfIllum;
	bool				twosided;
	BlendType			blend;
	pix::Colorf			diffuseColor;
	bool				specular;
	pix::Colorf			specularColor;
	float				specularExponent;
	TextureLayer		diffuseLayer;
	TextureLayer		opacityLayer;
	TextureLayer		reflectionLayer;
	TextureLayer		glossinessLayer;
	TextureLayer		bumpLayer;
	TextureLayer		lightMapLayer;
	TextureLayer		specularColorLayer;
	TextureLayer		specularLevelLayer;

	GmMaterial();

	void	write( io::ChunkOutputStream* out ) const;

	/** Returns which texture layers are enabled. */
	int		layerFlags() const;

	bool	operator==( const GmMaterial& other ) const;
	bool	operator!=( const GmMaterial& other ) const;

private:
	void	defaults();

	void	writeMaterial( io::ChunkOutputStream* out ) const;
	void	writeEffect( io::ChunkOutputStream* out, const lang::String& fxName ) const;

	GmMaterial( const GmMaterial& );
	GmMaterial& operator=( const GmMaterial& );
};


#endif // _GMMATERIAL_H
