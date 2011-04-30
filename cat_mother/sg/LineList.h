#ifndef _SG_LINE_H
#define _SG_LINE_H


#include <sg/Primitive.h>


namespace gd {
	class Primitive;}

namespace pix {
	class Color;}

namespace lang {
	class String;}

namespace math {
	class Vector3;}


namespace sg
{


/** 
 * 3D line list primitive.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class LineList :
	public Primitive
{
public:
	/** 
	 * Data lock mode.
	 * @see LineLock
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

	/** Type of lines to draw. */
	enum LinesType
	{
		/** Screen space lines. */
		LINES_2D,
		/** Object/world space lines. */
		LINES_3D
	};

	/** 
	 * Creates a primitive with space allocated for specified amount of lines.
	 */
	explicit LineList( int maxLines, LinesType type );

	///
	~LineList();

	/** Creates  clone of other primitive. */
	LineList( const LineList& other, int shareFlags );

	/** Returns clone of this primitive. */
	Primitive* clone( int shareFlags ) const;

	/** Destroys the object. */
	void	destroy();

	/** Uploads object to the rendering device. */
	void	load();

	/** Unloads object from the rendering device. */
	void	unload();

	/** Draws the lines to the active rendering device. */
	void	draw();

	/**
	 * Adds a line. Vertices must be locked.
	 * @param start Start point of the line.
	 * @param end End point of the line.
	 * @param startColor Color of the start point of the line.
	 */
	void	addLine( const math::Vector3& start, const math::Vector3& end, const pix::Color& color );

	/**
	 * Adds a line. Vertices must be locked.
	 * @param start Start point of the line.
	 * @param end End point of the line.
	 * @param startColor Color of the start point of the line.
	 * @param endColor Color of the end point of the line.
	 */
	void	addLine( const math::Vector3& start, const math::Vector3& end, const pix::Color& startColor, const pix::Color& endColor );

	/**
	 * Adds n lines. Vertices must be locked.
	 * @param positions Start- and end positions of the lines (in local space).
	 * @param colors Start- and end colors of the lines.
	 * @param count Number of lines to add.
	 */
	void	addLines( const math::Vector3* positions, const pix::Color* colors, int count );

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

	/** Removes all lines. */
	void	removeLines();

	/**
	 * Gets a line from the list. Vertices must be locked for reading.
	 * @param i Line index [0,lines()).
	 * @param start [out] Start point of the line.
	 * @param end [out] End point of the line.
	 * @param startColor [out] Color of the start point of the line.
	 * @param endColor [out] Color of the end point of the line.
	 */
	void	getLine( int i, math::Vector3* start, math::Vector3* end ) const;

	/** Computes primitive bounding sphere. */
	float	boundSphere() const;

	/** Returns number of lines. */
	int		lines() const;

	/** Returns maximum number of lines. */
	int		maxLines() const;

	/** Returns true if the vertices are locked. */
	bool	verticesLocked() const;

	/** Returns vertex format used by this geometry. */
	VertexFormat vertexFormat() const;

private:
	friend class LineListLock;

	P(gd::Primitive)	m_lines;
	int					m_count;
	LockType			m_lock;
	int					m_maxLines;
	LinesType			m_type;

	LineList();
	LineList( const LineList& other );
	LineList& operator=( const LineList& other );
};


} // sg


#endif // _SG_LINE_H
