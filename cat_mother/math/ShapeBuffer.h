#ifndef _MATH_SHAPEBUFFER_H
#define _MATH_SHAPEBUFFER_H


#include <lang/Array.h>
#include <lang/Object.h>


namespace math
{


/** 
 * Class for solving 2D packing problems.
 *
 * Top-left coordinates are (0,0) and bottom-down coordinates are (width,height).
 * All shapes are defined borders included, ie. if you add a rectangle with 
 * width dimension [0,32] you'll get 33 units wide rectangle.
 * 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ShapeBuffer :
	public lang::Object
{
public:
	/** Creates a 0x0 size empty buffer. */
	ShapeBuffer();

	/** Creates an empty buffer of specified size. */
	ShapeBuffer( int width, int height );

	/** 
	 * Draws triangle to the buffer. Clockwise vertex order. 
	 * @param id Shape identifier.
	 */
	void	drawTriangle( int X0, int Y0, int X1, int Y1, int X2, int Y2, int id );

	/** 
	 * Draws rectangle to the buffer. 
	 * @param x0 Left X coordinate, inclusive.
	 * @param y0 Top Y coordinate, inclusive.
	 * @param x1 Right X coordinate, inclusive.
	 * @param y1 Bottom Y coordinate, inclusive.
	 * @param id Shape identifier.
	 */
	void	drawRectangle( int X0, int Y0, int X1, int Y1, int id );

	/** 
	 * Sets size of the buffer.
	 * Clears old contents.
	 */
	void	setSize( int width, int height );

	/**
	 * Clears old contents.
	 */
	void	clear();

	/** 
	 * Checks if triangle would fit to the buffer. Clockwise vertex order.
	 * @return true if shape fits to the buffer.
	 * @param offsetX [out] Receives X offset to position where the shape could be placed.
	 * @param offsetY [out] Receives Y offset to position where the shape could be placed.
	 */
	bool	fitTriangle( int X0, int Y0, int X1, int Y1, int X2, int Y2, int* offsetX, int* offsetY ) const;

	/** 
	 * Checks if rectangle would fit to the buffer.
	 * @return true if shape fits to the buffer.
	 * @param x0 Left X coordinate, inclusive.
	 * @param y0 Top Y coordinate, inclusive.
	 * @param x1 Right X coordinate, inclusive.
	 * @param y1 Bottom Y coordinate, inclusive.
	 * @param offsetX [out] Receives X offset to position where the shape could be placed.
	 * @param offsetY [out] Receives Y offset to position where the shape could be placed.
	 */
	bool	fitRectangle( int X0, int Y0, int X1, int Y1, int* offsetX, int* offsetY ) const;

	/** Returns shape identifier at specified position or -1 if no shape. */
	int		getShape( int X, int Y ) const;

	/** Returns height of the buffer. */
	int		width() const;

	/** Returns width of the buffer. */
	int		height() const;

private:
	class Segment
	{
	public:
		Segment();
		Segment( const Segment& other );
		Segment( int left, int right, int id );

		Segment& operator=( const Segment& other );

		void	setLeft( int x );
		void	setRight( int x );

		int		left() const;
		int		right() const;
		int		id() const;

		/** Returns number of overlapping units. */
		int		overlaps( const Segment& other ) const;

	private:
		int		m_left;
		int		m_right;
		int		m_id;
	};

	typedef lang::Array<Segment,1>			LineBufferValue;
	typedef lang::Array<LineBufferValue,1>	LineBuffer;
	typedef LineBufferValue*				LineBufferIterator;

	int					m_width;
	int					m_height;
	LineBuffer			m_lines;
	mutable LineBuffer	m_tempLines;	// Temporary buffer.

	/** Checks if triangle would fit to the buffer. Clockwise vertex order. */
	bool	fitShape( const int* X, const int* Y, int vertices, const LineBuffer& buffer, int* offsetX, int* offsetY ) const;

	/** Returns true if specified shape is inside buffer. */
	bool	isShapeInside( const int* X, const int* Y, int vertices ) const;

	/**
	 * Returns true if shape would fit to specified position. Clockwise vertex order.
	 * @param minDeltaX [out] Receives minimum needed delta X to fit the shape.
	 */
	bool	shapeFits( const int* X, const int* Y, int vertices, const LineBuffer& buffer, int offsetX, int offsetY, int* minDeltaX ) const;

	/** Returns true if shape would fit to specified position. Clockwise vertex order. */
	//bool	shapeFits( const int* X, const int* Y, int vertices, const LineBuffer& buffer, int* minDeltaX ) const		{return shapeFits( X, Y, vertices, buffer, 0, 0, minDeltaX );}

	/** Draws a convex shape to specified buffer. Clockwise vertex order. */
	void	drawShape( const int* X, const int* Y, int vertices, int id, LineBuffer& buffer, int offsetX, int offsetY ) const;

	/** Draws a convex shape to specified buffer. Clockwise vertex order. */
	void	drawShape( const int* X, const int* Y, int vertices, int id, LineBuffer& buffer ) const;

	/** Draws a line to the buffer. Assumes clockwise vertex order and convex shape. */
	void	drawLine( int X0, int Y0, int X1, int Y1, int id, LineBuffer& buffer ) const;

	/**
	 * Returns true if buffer shapes overlap.
	 * @param minDeltaX [out] Receives minimum needed delta X to fit the shape.
	 */
	static bool	shapesOverlap( const LineBuffer& buffer, const LineBuffer& source, int Y0, int Y1, int* minDeltaX );

	/** Removes all segments from line buffer. */
	static void	clear( LineBuffer& lines );

	ShapeBuffer( const ShapeBuffer& );
	ShapeBuffer& operator=( const ShapeBuffer& );
};


} // math


#endif // _MATH_SHAPEBUFFER_H
