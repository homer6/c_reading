#ifndef _TEXTUREANIMATION_H
#define _TEXTUREANIMATION_H


#include "TextureSequence.h"
#include <sg/Material.h>
#include <lang/Object.h>
#include <util/Vector.h>
#include <anim/VectorInterpolator.h>


/** 
 * Texture animation sequence data.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class TextureAnimation :
	public lang::Object
{
public:
	lang::String					name;
	P(TextureSequence)				frames;
	P(anim::VectorInterpolator)		frameCtrl;
	util::Vector<P(sg::Material)>	materials;
	float							time;			// <0 if not active
	int								hint;
	int								lastFrame;

	TextureAnimation();
	TextureAnimation( const TextureAnimation& other );
	~TextureAnimation();

private:
	TextureAnimation& operator=( const TextureAnimation& );
};


#endif // _TEXTUREANIMATION_H
