#include <bsp/BSPFile.h>
#include <bsp/BSPNode.h>
#include <bsp/BSPPolygon.h>
#include <bsp/BSPTree.h>
#include <io/IOException.h>
#include <io/ChunkInputStream.h>
#include <io/ChunkOutputStream.h>
#include <lang/Debug.h>
#include <lang/String.h>
#include <lang/Exception.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <util/Vector.h>
#include <string.h>
#include "config.h"

//-----------------------------------------------------------------------------

// increase this always when you modify file format
#define BSP_FILE_VERSION 6

//-----------------------------------------------------------------------------

using namespace bsp;
using namespace io;
using namespace lang;
using namespace math;
using namespace util;

//-----------------------------------------------------------------------------

namespace bsp
{


BSPFile::BSPFile( BSPTree* root, OutputStream* out ) :
	m_tree( root ),
	m_polygonBuffer( Allocator<BSPPolygon*>(__FILE__,__LINE__) ),
	m_vertexBuffer( Allocator<Vector3>(__FILE__,__LINE__) ),
	m_indexBuffer( Allocator<int>(__FILE__,__LINE__) )
{
	write( out );
}

BSPFile::BSPFile( io::InputStream* in ) :
	m_tree( new BSPTree ),
	m_polygonBuffer( Allocator<BSPPolygon*>(__FILE__,__LINE__) ),
	m_vertexBuffer( Allocator<Vector3>(__FILE__,__LINE__) ),
	m_indexBuffer( Allocator<int>(__FILE__,__LINE__) )
{
	read( in );
}

BSPFile::~BSPFile()
{
}

void BSPFile::read( io::InputStream* in ) 
{
	ChunkInputStream input(in);
	String rootname;

	// read main chunk
	String name;
	long end;
	input.beginChunk( &name, &end );
	if ( name != "bsptree" )
		throw IOException( Format("BSP data {0} must begin with bsptree chunk", in->toString()) );

	// read version
	int ver = input.readInt();
	if ( ver != BSP_FILE_VERSION )
		throw IOException( Format("BSP data {0} invalid version (was {1}, should have been {2})", in->toString(), ver, BSP_FILE_VERSION) );

	// read node count
	int nodes = input.readInt();
	m_tree->setNodeAllocationUnit( nodes );

	// read total number of polys in all nodes, reserve space for them to avoid allocations
	int nodepolys = input.readInt();
	m_tree->nodePolygonData.setSize( nodepolys );
	m_tree->nodePolygonData.clear();

	// read vertices
	{
		String name;
		long end;
		input.beginChunk( &name, &end );
		if ( name != "vertices" )
			throw IOException( Format("BSP data {0} vertices chunk must follow BSP file version", in->toString()) );
		
		int nvertices = input.readInt();
		m_tree->vertexData.setSize( nvertices );
		for ( int i = 0 ; i < nvertices ; ++i )
		{
			float v[3];
			input.readFloatArray( v, 3 );
			m_tree->vertexData[i] = Vector3(v[0],v[1],v[2]);
		}

		input.endChunk( end );
	}

	// read polygons
	{
		String name;
		long end;
		input.beginChunk( &name, &end );
		if ( name != "polygons" )
			throw IOException( Format("BSP data {0} polygon chunk must follow vertices chunk", in->toString()) );
		
		int npolys = input.readInt();
		m_tree->setPolygonAllocationUnit( npolys );
		m_tree->edgePlaneData.setSize( 3*npolys ); // reserve space for edges
		m_tree->edgePlaneData.clear();
		m_tree->indexData.setSize( 3*npolys ); // reserve space for indices
		m_tree->indexData.clear();
		for ( int i = 0 ; i < npolys ; ++i )
			readPolygon( &input );

		input.endChunk( end );
	}

	// read node tree
	readNode( &input );

	// end main chunk
	input.endChunk( end );
}

void BSPFile::write( io::OutputStream* out ) 
{
	Debug::println( "bsp: Writing BSP tree to {0}...", out->toString() );

	// write version
	ChunkOutputStream output( out );
	output.beginChunk( "bsptree" );
	output.writeInt( BSP_FILE_VERSION );

	// write node count
	assert( m_tree->nodes() > 0 );
	output.writeInt( m_tree->nodes() );

	// write total number of polys in all nodes
	output.writeInt( getTreePolygonCount(m_tree->root()) );

	// write vertices
	output.beginChunk( "vertices" );
	int nvertices = m_tree->vertexData.size();
	output.writeInt( nvertices );
	for ( int i = 0 ; i < nvertices ; ++i )
		for ( int k = 0 ; k < 3 ; ++k )
			output.writeFloat( m_tree->vertexData[i][k] );
	output.endChunk();

	// write polygons
	output.beginChunk( "polygons" );
	output.writeInt( m_tree->polygons() );
	for ( int i = 0 ; i < m_tree->polygons() ; ++i )
		writePolygon( &output, m_tree->getPolygon(i) );
	output.endChunk();

	// write node tree
	writeNode( &output, m_tree->root() );

	output.endChunk();
}

BSPTree* BSPFile::tree() const 
{
	return m_tree;
}

BSPNode* BSPFile::readNode( ChunkInputStream* in ) 
{
	long chunkend = 0;

	String name;
	in->beginChunk( &name, &chunkend );

	Vector4 plane = Vector4(0,0,0,1);

	plane.x = in->readFloat();
	plane.y = in->readFloat();
	plane.z = in->readFloat();
	plane.w = in->readFloat();

	BSPNode* pos = 0;
	BSPNode* neg = 0;
	int childFlags = in->readInt();
	if ( childFlags & 1 )
		pos = readNode( in );
	if ( childFlags & 2 )
		neg = readNode( in );

	int npolygons = in->readInt();
	m_polygonBuffer.setSize( npolygons );
	for ( int i = 0; i < npolygons; ++i )
	{
		int ix = in->readInt();
		if ( ix < 0 || ix >= m_tree->polygons() )
			throw IOException( Format("Invalid polygon index in BSP {0}", in->toString()) );
		m_polygonBuffer[i] = m_tree->getPolygon(ix);
	}

	in->endChunk( chunkend );
	return m_tree->createNode( plane, m_polygonBuffer, pos, neg );
}

void BSPFile::readPolygon( ChunkInputStream* in ) 
{
	int id = in->readInt();
	int nvertices = in->readInt();
	int collisionMask = in->readInt();

	if ( nvertices < 3 )
		throw IOException( Format("Invalid polygon in BSP data {0}", in->toString()) );

	m_indexBuffer.setSize( nvertices );
	for ( int i = 0 ; i < nvertices ; ++i )
	{
		m_indexBuffer[i] = in->readInt();
		if ( m_indexBuffer[i] < 0 || m_indexBuffer[i] >= m_tree->vertexData.size() )
			throw IOException( Format("Invalid vertex index in BSP data {0}", in->toString()) );
	}

	BSPPolygon* poly = m_tree->createPolygon();
	poly->create( m_indexBuffer.begin(), nvertices, id, m_tree, collisionMask );
}
	
void BSPFile::writeNode( ChunkOutputStream* out, const BSPNode* nodeIn ) 
{
	Debug::println( "bsp: Writing node of {0} polygons", nodeIn->polygons() );

	out->beginChunk( "node" );

	out->writeFloat( nodeIn->plane().x );
	out->writeFloat( nodeIn->plane().y );
	out->writeFloat( nodeIn->plane().z );
	out->writeFloat( nodeIn->plane().w );

	int childFlags = 0;
	if ( nodeIn->positive() )
		childFlags |= 1;
	if ( nodeIn->negative() )
		childFlags |= 2;
	out->writeInt( childFlags );

	if ( nodeIn->positive() )
		writeNode( out, nodeIn->positive() );
	if ( nodeIn->negative() )
		writeNode( out, nodeIn->negative() );

	out->writeInt( nodeIn->polygons() );
	for ( int i = 0; i < nodeIn->polygons(); ++i )
		out->writeInt( m_tree->getPolygonIndex(&nodeIn->getPolygon(i)) );

	out->endChunk();
}

void BSPFile::writePolygon( ChunkOutputStream* out, const BSPPolygon* polygonIn ) 
{
	out->writeInt( polygonIn->id() );
	out->writeInt( polygonIn->vertices() );
	out->writeInt( polygonIn->collisionMask() );
	
	for ( int i = 0; i < polygonIn->vertices(); ++i )
		out->writeInt( polygonIn->getVertexIndex(i) );
}

int BSPFile::getTreePolygonCount( bsp::BSPNode* node )
{
	int c = 0;
	if ( node )
	{
		c += node->polygons();
		if ( node->positive() )
			c += getTreePolygonCount( node->positive() );
		if ( node->negative() )
			c += getTreePolygonCount( node->negative() );
	}
	return c;
}


} // bsp
