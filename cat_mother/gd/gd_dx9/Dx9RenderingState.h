#include "Dx9Material.h"
#include <pix/Color.h>


/** 
 * Helper function for checking if some rendering state has been changed.
 * If the state has been changed, new value is assigned and true is returned.
 */
template <class T> inline static bool changed( T& target, const T& source )
{
	bool changed = (target != source);
	if ( changed )
		target = source;
	return changed;
}


/** Device rendering state. */
class Dx9RenderingState
{
public:
	Dx9Material::ReflectanceFactors		refl;
	Dx9Material::BlendMode				srcBlend;
	Dx9Material::BlendMode				dstBlend;
	char								depthEnabled;
	char								depthWrite;
	Dx9Material::CmpFunc				depthFunc;
	Dx9Material::CullMode				cull;
	char								specularEnabled;
	char								lighting;
	char								vertexColor;
	char								fogDisabled;
	Dx9Material::MaterialColorSource	diffuseSource;
	Dx9Material::MaterialColorSource	specularSource;
	Dx9Material::MaterialColorSource	ambientSource;
	Dx9Material::MaterialColorSource	emissiveSource;
	Dx9Material::TextureLayer			textureLayers[Dx9Material::TEXTURE_LAYERS];
	char								stencil;
	int									vertexWeights;
	int8_t								mixedVP;
	int8_t								polygonSorting;
	int									d3dfvf;
	Dx9Material::StencilOperation		stencilFail;
	Dx9Material::StencilOperation		stencilZFail;
	Dx9Material::StencilOperation		stencilPass;
	Dx9Material::CmpFunc				stencilFunc;
	int									stencilRef;
	int									stencilMask;
	int8_t								alphaTestEnabled;
	Dx9Material::CmpFunc				alphaCompareFunc;
	int									alphaReferenceValue;

	Dx9RenderingState();

	void invalidate();
};
