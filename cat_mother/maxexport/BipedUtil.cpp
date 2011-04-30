#include "StdAfx.h"
#include <lang/Debug.h>
#include <lang/Character.h>
#include "BipedUtil.h"

//-----------------------------------------------------------------------------

// this is the class for all biped controllers except the root and the footsteps
#define BIPSLAVE_CONTROL_CLASS_ID Class_ID(0x9154,0)
// this is the class for the center of mass, biped root controller ("BipXX")
#define BIPBODY_CONTROL_CLASS_ID  Class_ID(0x9156,0) 

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

bool BipedUtil::isBipedName( const String& objectName, const String& boneName )
{
	String name = objectName.toLowerCase();
	String bone = boneName.toLowerCase();

	if ( name.startsWith("bip") )
	{
		// find bone suffix from the name, skip digits and whitespace
		int boneSuffixIndex = 3;
		for ( ; boneSuffixIndex < name.length() ; ++boneSuffixIndex )
		{
			Char ch = name.charAt( boneSuffixIndex );
			if ( !Character::isDigit(ch) && !Character::isWhitespace(ch) )
				break;
		}

		String boneSuffix = name.substring( boneSuffixIndex );
		if ( boneSuffix == bone )
		{
			Debug::println( "{0} is biped bone {1}", objectName, boneName );
			return true;
		}
	}
	//Debug::println( "{0} is not biped bone {1}", objectName, boneName );
	return false;
}

bool BipedUtil::isBiped( INode* node )
{
	require( node );
	Control* c = node->GetTMController();
	return c &&
		c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID ||
		c->ClassID() == BIPBODY_CONTROL_CLASS_ID;
}
