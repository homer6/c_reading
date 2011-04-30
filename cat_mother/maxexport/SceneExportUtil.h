#include <lang/String.h>
#include <util/Vector.h>
#include <stdmat.h>


namespace anim {
	class KeyFrameContainer;}


/** 
 * Utilities for 3DS Max scene exporting. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SceneExportUtil
{
public:
	/** 
	 * Collects nodes from the scene. 
	 * Does not clear node container before adding the materials.
	 *
	 * @param nodes [out] Receives nodes to export.
	 */
	static void			getNodesToExport( Interface* i, DWORD options, util::Vector<INode*>& nodes );

	/**
	 * Collects used materials from the node.
	 * Lists Multi/Sub-Object sub-materials separately.
	 * Does not clear material container before adding the materials.
	 *
	 * @parma materials [out] Receives used materials.
	 */
	static void			getUsedMaterials( INode* node, util::Vector<Mtl*>& materials );

	/**
	 * Collects used materials from the nodes.
	 * Lists Multi/Sub-Object sub-materials separately.
	 * Does not clear material container before adding the materials.
	 *
	 * @parma materials [out] Receives used materials.
	 */
	static void			getUsedMaterials( const util::Vector<INode*>& nodes, util::Vector<Mtl*>& materials );

	/**
	 * Return a pointer to a TriObject given an INode or return 0
	 * if the node cannot be converted to a TriObject.
	 *
	 * @param deleteIt [out] Receives true if the triobj should be deleted after use.
	 * @param triobj [out] Receives the triangle object.
	 */
	static void			getTriObjectFromNode( INode *node, TimeValue t, bool* deleteIt, TriObject** triobj );

	/** 
	 * Returns Standard material Blinn shader bitmap texture (if any) from specified channel. 
	 * @param id Channel id: ID_DI, ID_RL, ...
	 * @return Bitmap texture of the channel or 0 if not any.
	 */
	static BitmapTex*	getStdMatBitmapTex( StdMat* stdmat, int id );

	/** 
	 * Copies source file to target. 
	 * @exception IOException
	 */
	static void			copyFile( const lang::String& target, const lang::String& source );

	/** Removes path from the filename. */
	static lang::String	stripPath( const lang::String& str );

	/** Gets only the path from the filename. */
	static lang::String	getPath( const lang::String& str );
};
