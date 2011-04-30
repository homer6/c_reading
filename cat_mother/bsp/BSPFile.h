#ifndef _BSP_BSPFILE_H
#define _BSP_BSPFILE_H


#include "BSPPolygon.h"
#include <lang/Object.h>
#include <util/Vector.h>


namespace math {
	class Vector3;
	class Vector4; }

namespace io {
	class ChunkInputStream; 
	class ChunkOutputStream; 
	class InputStream; 
	class OutputStream; }

namespace lang {
	class String; }


namespace bsp 
{


class BSPNode;
class BSPTree;


/** 
 * BSP tree input/output. 
 * @author Jani Kajala (jani.kajala@helsinki.fi), Toni Aittoniemi
 */
class BSPFile : 
	public lang::Object
{
public:
	/** Construct BSPFile from node & write to output stream. */
	BSPFile( bsp::BSPTree* tree, io::OutputStream* out );

	/** Contruct BSPFile from input stream. */
	explicit BSPFile( io::InputStream* in );

	///
	~BSPFile();

	/** Returns root node. */
	BSPTree*		tree() const;

private:
	BSPNode*				readNode( io::ChunkInputStream* in );
	void					readPolygon( io::ChunkInputStream* in );
	
	void					writeNode( io::ChunkOutputStream* out, const bsp::BSPNode* nodeIn );
	void					writePolygon( io::ChunkOutputStream* out, const bsp::BSPPolygon* polygonIn );

	P(util::Vector<BSPPolygon*>)	m_polygons;
	P(BSPTree)						m_tree;

	util::Vector<BSPPolygon*>		m_polygonBuffer;
	util::Vector<math::Vector3>		m_vertexBuffer;	
	util::Vector<int>				m_indexBuffer;	

	/** Read BSP from stream. */
	void			read( io::InputStream* in );

	/** Write BSP to stream. */
	void			write( io::OutputStream* out );

	/** Returns total polygon count of BSP tree. */
	static int		getTreePolygonCount( bsp::BSPNode* node );

	BSPFile( const BSPFile& );
	BSPFile& operator=( const BSPFile& );
};


} // bsp


#endif // _BSP_BSPFILE_H
