#ifndef _SG_PATCHLIST_H
#define _SG_PATCHLIST_H


#include <sg/Primitive.h>
#include <sg/VertexFormat.h>
#include <sg/PolygonAdjacency.h>
#include <math/Vector3.h>
#include <util/Vector.h>


namespace sg
{


/** 
 * List of cubic 4x4 Bezier patches. Not rendered. 
 * Each patch consists of 16 vertices.
 *
 * Patch vertices are organized as follows:
 * (u grows right, v grows down, patch normal left-handed)
 * <pre>
   p(0,0)       edge(1)         p(0,3)
   ...
   edge(0)      ...             edge(2)
   ...
   p(3,0)       edge(3)         p(3,3)
   </pre>
 * 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class PatchList :
	public Primitive
{
public:
	/** 
	 * Data lock mode.
	 * @see ModelLock
	 */
	enum LockType
	{
		/** Lock data for reading only. */
		LOCK_READ,
		/** Lock data for writing only. */
		LOCK_WRITE,
		/** Lock data for both reading and writing. */
		LOCK_READWRITE
	};

	/** 
	 * Constructs a patch list with specified number of vertices. 
	 * @param vertices Must be multiple of 16.
	 */
	explicit PatchList( int vertices );

	PatchList( const PatchList& other, int shareFlags );

	Primitive* clone( int shareFlags ) const;

	/** Releases resources of the object. */
	void	destroy();

	/** Uploads object to the rendering device. */
	void	load();

	/** Unloads object from the rendering device. */
	void	unload();

	/** Does nothing. */
	void	draw();

	/** Sets vertex positions. Requires that the vertices are locked. */
	void	setVertexPositions( int firstVertex, const math::Vector3* positions, int count=1 );

	/** 
	 * Locks vertex data. Don't use this method directly,
	 * but use VertexLock instead. (which automatically releases 
	 * the lock at the end of the scope).
	 * @see VertexLock
	 * @exception LockException
	 */
	void	lockVertices( LockType lock );

	/** 
	 * Unlocks vertex data. 
	 * @see lockVertices 
	 */
	void	unlockVertices();

	/** Returns vertex positions. Requires that the vertices are locked. */
	void	getVertexPositions( int firstVertex, math::Vector3* positions, int count=1 ) const;

	/** Returns number of vertices. */
	int		vertices() const;

	/** Returns true if the vertices are locked. */
	bool	verticesLocked() const;

	/** Returns vertex format of the patch list. */
	VertexFormat	vertexFormat() const;

	/** 
	 * Returns control polygon adjacency information.
	 * Requires that vertices are locked for reading.
	 * @param zeroDistance Maximum distance between vertices to be considered equal.
	 */
	const PolygonAdjacency&	getPolygonAdjacency( float zeroDistance ) const;

private:
	util::Vector<math::Vector3>	m_vertices;
	LockType					m_lock;
	mutable PolygonAdjacency	m_adj;
	mutable float				m_adjZeroDistance;

	PatchList( const PatchList& );
	PatchList& operator=( const PatchList& );
};


} // sg


#endif // _SG_PATCHLIST_H
