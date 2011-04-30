#ifndef _BIPEDUTIL_H
#define _BIPEDUTIL_H


#include <lang/String.h>


/** 
 * Character Studio biped utilities. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class BipedUtil
{
public:
	/** 
	 * Returns true if objectName is format BipXX <boneName>,
	 * for example isBipedName(x,"Spine") will return true if x
	 * is Bip01 Spine, Bip02 Spine, ...
	 */
	static bool		isBipedName( const lang::String& objectName, const lang::String& boneName );

	/** Is node biped or not? */
	static bool		isBiped( INode* node );
};


#endif // _BIPEDUTIL_H
