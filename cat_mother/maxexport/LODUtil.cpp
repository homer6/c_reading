#include "StdAfx.h"
#include "LODUtil.h"
#ifdef SGEXPORT_LODCTRL
#include "LODCtrl.h"
#endif
#include <lang/Debug.h>
#include <lang/Character.h>
#include <lang/Exception.h>
#include <lang/NumberReader.h>

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

/** LOD group name base substring. */
const String	LOD_GROUP_BASE_ID	= "_LOD";

/** LOD group name size (pixels) substring. */
const String	LOD_GROUP_SIZE_ID	= "_SIZE";

//-----------------------------------------------------------------------------

/**
 * Parses integer from a string.
 * @exception Exception
 */
static int parseInt( const String& str, int i, const String& name )
{
	int pos = i;

	NumberReader<int> nr;
	while ( i < str.length() &&
		str.charAt(i) < 0x80 &&
		Character::isDigit( str.charAt(i) ) &&
		1 == nr.put( (char)str.charAt(i) ) )
		++i;

	if ( !nr.valid() )
		throw Exception( Format("Failed to parse {0} from {1} (index {2,#})", name, str, pos) );
	return nr.value();
}

//-----------------------------------------------------------------------------

bool LODUtil::isLODHead( INode* node )
{
	return getLODHeadID(node) != "";
}

String LODUtil::getLODHeadID( INode* node )
{
#ifdef SGEXPORT_LODCTRL
	// check for plugin LOD
	for ( int i = 0 ; i < node->NumberOfChildren() ; ++i )
	{
		INode* child = node->GetChildNode( i );

		if ( child->IsGroupMember() )
		{
			Control* vis = child->GetVisController();
			if ( vis && LOD_CONTROL_CLASS_ID == vis->ClassID() )
			{
				LODCtrl* lod = (LODCtrl*)vis;
				return String::valueOf( lod->grpID );
			}
		}
	}
#endif

	// check for name based LOD
	String str = node->GetName();
	int baseIndex = str.indexOf( LOD_GROUP_BASE_ID );
	int numIndex = str.lastIndexOf( "_" );
	if ( -1 != baseIndex )
	{
		int baseLen = LOD_GROUP_BASE_ID.length();
		int serNum = parseInt( str, numIndex+1, "LOD serial number" );
		String lodBaseID = str.substring( 0, baseIndex+baseLen );
		String lodID = lodBaseID + "_" + String::valueOf( serNum );
		return lodID;
	}

	return "";
}

void LODUtil::getLODMemberInfo( INode* node, 
	String* lodID, float* lodMin, float* lodMax )
{
	// defaults
	*lodID = "";
	*lodMin = 0.f;
	*lodMax = 0.f;

#ifdef SGEXPORT_LODCTRL
	// get level of detail from lod plugin
	INode* visnode = node;
	Control* vis = visnode->GetVisController();
	if ( vis && LOD_CONTROL_CLASS_ID == vis->ClassID() )
	{
		LODCtrl* lod = (LODCtrl*)vis;
		*lodID = String::valueOf( lod->grpID );
		*lodMin = lod->min;
		*lodMax = lod->max;
		return;
	}
#endif

	// get level of detail from name
	String str = node->GetName();
	int baseIndex = str.indexOf( LOD_GROUP_BASE_ID );
	int sizeIndex = str.indexOf( LOD_GROUP_SIZE_ID );
	int numIndex = str.lastIndexOf( "_" );
	if ( -1 != baseIndex )
	{
		int baseLen = LOD_GROUP_BASE_ID.length();
		int sizeLen = LOD_GROUP_SIZE_ID.length();
		int serNum = parseInt( str, numIndex+1, "LOD serial number" );

		String lodBaseID = str.substring( 0, baseIndex+baseLen );
		*lodID = lodBaseID + "_" + String::valueOf( serNum );
		*lodMin = 0.f;
		*lodMax = parseInt( str, sizeIndex+sizeLen, "LOD size" );
		return;
	}
}
