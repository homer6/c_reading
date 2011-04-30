#include <util/Vector.h>
#ifdef SGEXPORT_PHYSIQUE
#include <PhyExp.h>


namespace mb {
	class VertexMap;}


/** 
 * Utilities for 3DS Max Character Studio skin (Physique) exporting. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class PhyExportUtil
{
public:
	/** Biped scaling. */
	enum BipedScaleType
	{
		/** Non-uniform scaling. */
		BIPED_NONUNIFORM,
		/** Uniform scaling. */
		BIPED_UNIFORM,
	};

	/** 
	 * Find if a given node contains a Physique Modifier.
	 */
	static Modifier*	findPhysiqueModifier( INode* nodePtr );

	/** 
	 * Lists all bones used by the Physique modifier context. 
	 */
	static void			listBones( IPhyContextExport* mcExport, util::Vector<INode*>& bones );

	/** Inserts all skin vertices affected by the node to the vertex map. */
	static void			addWeights( mb::VertexMap* vmap, INode* node, IPhyContextExport* mcExport );

	/** Returns initial bone node transform. */
	static Matrix3		getInitBoneTM( INode* node, IPhysiqueExport* phyExport );

	/** Returns initial skin node transform. */
	static Matrix3		getInitSkinTM( INode* node, IPhysiqueExport* phyExport );
};
#endif