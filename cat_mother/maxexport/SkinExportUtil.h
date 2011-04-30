#include <util/Vector.h>
#include <modstack.h>
#include <iparamb2.h>
#include <iskin.h>


namespace mb {
	class VertexMap;}


/** 
 * Utilities for 3DS Max Skin modifier exporting. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SkinExportUtil
{
public:
	/** 
	 * Find if a given node contains a Skin Modifier.
	 * Use GetInterface(I_SKIN) to get ISkin interface.
	 */
	static Modifier*	findSkinModifier( INode* node );

	/** 
	 * Lists all bones used by the Skin modifier. 
	 */
	static void			listBones( ISkin* skin, util::Vector<INode*>& bones );

	/** Inserts all skin vertices affected by the node to the vertex map. */
	static void			addWeights( mb::VertexMap* vmap, INode* node, ISkin* skin, ISkinContextData* skincx );

	/** Returns initial bone node transform. */
	static Matrix3		getInitBoneTM( INode* node, ISkin* skin );

	/** Returns initial skin node transform. */
	static Matrix3		getInitSkinTM( INode* node, ISkin* skin );
};
