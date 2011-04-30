#include <lang/String.h>


/** 
 * Utilities for level of detail exporting. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class LODUtil
{
public:
	/** Returns true if the node is LOD group head. */
	static bool				isLODHead( INode* node );

	/** Returns LOD ID if the node is LOD group head or "" otherwise. */
	static lang::String		getLODHeadID( INode* node );

	/** Returns LOD member info. */
	static void				getLODMemberInfo( INode* node, 
								lang::String* id, float* mn, float* mx );
};
