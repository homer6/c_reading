#ifndef _TEXTURESEQUENCE_H
#define _TEXTURESEQUENCE_H


#include <sg/Texture.h>
#include <lang/Object.h>
#include <lang/String.h>
#include <util/Vector.h>


namespace io {
	class InputStreamArchive;}


/** 
 * Sequence of textures. 
 * Texture animation sequence is separated from the image sequence because
 * this makes it possible to create complex animation sequences suchs as (1,2,3,4,3,2,3,4,5,6,7)
 * without restrictions of start/end behaviour types.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class TextureSequence :
	public lang::Object
{
public:
	/** 
	 * Creates texture sequence of specified name from specified files. File name ranges are inclusive. 
	 * Example: TextureSequence( archive, "idle", "audience_idle_{0,000}.tga", 1, 15 )
	 * creates sequence from image files audience_idle_001.tga, ... audience_idle_015.tga.
	 */
	TextureSequence( io::InputStreamArchive* arch, const lang::String& name,	
		const lang::String& imageFilenameFmt, int firstImage, int lastImage );

	~TextureSequence();

	/** Returns ith frame of the texture sequence. */
	sg::Texture*	getFrame( int i ) const;

	/** Returns number of textures in the sequence. */
	int				frames() const;

	/** Returns name of this sequence. */
	const lang::String&	name() const;

private:
	util::Vector<P(sg::Texture)>	m_frames;
	lang::String					m_name;

	TextureSequence( const TextureSequence& );
	TextureSequence& operator=( const TextureSequence& );
};


#endif // _TEXTURESEQUENCE_H
