#include "ShapeBuffer.h"
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace math
{


// Finds minimum and maximum values from array of integers.
static void minmax( const int* X, int count, int* minX, int* maxX )
{
	assert( count > 0 );

	int min = X[0];
	int max = X[0];
	for ( int i = 1 ; i < count ; ++i )
	{
		int x = X[i];
		if ( x < min ) min = x;
		if ( x > max ) max = x;
	}
	*minX = min;
	*maxX = max;
}

// Swaps two ints
inline static void swap( int& X, int& Y )
{
	int tmp = X;
	X = Y;
	Y = tmp;
}

//-----------------------------------------------------------------------------

ShapeBuffer::Segment::Segment()
{
}

ShapeBuffer::Segment::Segment( const Segment& other )
{
	*this = other;
}

ShapeBuffer::Segment& ShapeBuffer::Segment::operator=( const Segment& other )
{
	m_left = other.m_left;
	m_right = other.m_right;
	m_id = other.m_id;
	return *this;
}

ShapeBuffer::Segment::Segment( int left, int right, int id ) : 
	m_left(left), m_right(right), m_id(id) 
{
}

void ShapeBuffer::Segment::setLeft( int x )		
{
	m_left = x;
}

void ShapeBuffer::Segment::setRight( int x )
{
	m_right=x;
}

int ShapeBuffer::Segment::left() const													
{
	return m_left;
}

int ShapeBuffer::Segment::right() const													
{
	return m_right;
}

int ShapeBuffer::Segment::id() const														
{
	return m_id;
}

int ShapeBuffer::Segment::overlaps( const Segment& other ) const
{
	int left = ( m_left > other.m_left ? m_left : other.m_left );
	int right = ( m_right < other.m_right ? m_right : other.m_right );
	if ( right >= left )
		return right-left+1;
	return 0;
}

bool ShapeBuffer::isShapeInside( const int* X, const int* Y, int vertices ) const	
{
	for ( int i = 0 ; i < vertices ; ++i ) 
		if ( X[i]<0 || Y[i]<0 || X[i]>=m_width || Y[i]>=m_height ) 
			return false; 
	return true;
}

//-----------------------------------------------------------------------------

ShapeBuffer::ShapeBuffer()
{
	m_width = 0;
	m_height = 0;
}

ShapeBuffer::ShapeBuffer( int width, int height )
{
	setSize( width, height );
}

void ShapeBuffer::setSize( int width, int height )
{
	assert( width > 0 );
	assert( height > 0 );

	m_width = width;
	m_height = height;
	m_lines.clear();
	m_lines.setSize( height );
	m_tempLines.clear();
	m_tempLines.setSize( height );

	assert( m_height == (int)m_lines.size() );
}

void ShapeBuffer::clear()
{
	m_lines.clear();
	m_lines.setSize( m_height );
	m_tempLines.clear();
	m_tempLines.setSize( m_height );
}

int	ShapeBuffer::getShape( int X, int Y ) const
{
	assert( Y >= 0 && Y < m_height );
	assert( X >= 0 && X < m_width );
	assert( m_height == (int)m_lines.size() );
	
	const LineBufferValue& line = m_lines[Y];
	if ( line.size() > 0 )
	{
		const Segment* it = line.end();
		do
		{
			--it;
			const Segment& seg = *it;
			if ( seg.left() <= X && seg.right() >= X )
				return seg.id();
		} while ( it != line.begin() );
	}

	return -1;
}

void ShapeBuffer::drawShape( const int* X, const int* Y, int vertices, int id, LineBuffer& buffer, int offsetX, int offsetY ) const
{
	int i0 = vertices-1;
	for ( int i = 0 ; i < vertices ; ++i )
	{
		drawLine( X[i0]+offsetX, Y[i0]+offsetY, X[i]+offsetX, Y[i]+offsetY, id, buffer );
		i0 = i;
	}
}

void ShapeBuffer::drawShape( const int* X, const int* Y, int vertices, int id, LineBuffer& buffer ) const
{
	drawShape( X, Y, vertices, id, buffer, 0, 0 );
}

void ShapeBuffer::drawLine( int X0, int Y0, int X1, int Y1, int id, LineBuffer& buffer ) const
{
	assert( (int)buffer.size() == m_height );
	assert( X0 >= 0 && X0 < m_width );
	assert( X1 >= 0 && X1 < m_width );
	assert( Y0 >= 0 && Y0 < (int)buffer.size() );
	assert( Y1 >= 0 && Y1 < (int)buffer.size() );

	// find correct side (of convex shape)
	// (assumes cw vertex order and convex shapes)
	bool leftSide = false;
	if ( X0 <= X1 )
	{
		if ( Y1 < Y0 )
			leftSide = true;
		else if ( Y1 > Y0 )
			leftSide = false;
	}
	else // X0 > X1
	{
		if ( Y1 < Y0 )
			leftSide = true;
		else if ( Y1 > Y0 )
			leftSide = false;
	}

	// handle horizontal line
	if ( Y0 == Y1 )
	{
		if ( X0 > X1 )
			swap( X0, X1 );

		LineBufferValue& line = buffer[Y0];
		if ( line.size() > 0 && line[line.size()-1].id() == id )
		{
			Segment& seg = line[line.size()-1];
			if ( seg.left() > X0 )
				seg.setLeft( X0 );
			if ( seg.right() < X1 )
				seg.setRight( X1 );
		}
		else
		{
			line.add( Segment(X0,X1,id) );
		}
	}
	else // normal slope line
	{
		// reorder coordinates so that we can draw with uniform loop
		if ( Y0 > Y1 )
		{
			swap( Y0, Y1 );
			swap( X0, X1 );
		}

		// draw slope
		int dy = Y1 - Y0;
		int dx16 = (int)( (X1-X0)*65536.f / (float)dy );
		int x16 = X0 << 16;
		for ( int y = Y0 ; y < Y1 ; ++y )
		{
			int x = x16>>16;
			x16 += dx16;

			LineBufferValue& line = buffer[y];
			if ( line.size() > 0 && line[line.size()-1].id() == id )
			{
				// continue existing shape
				if ( leftSide )
					line[line.size()-1].setLeft( x );
				else // right
					line[line.size()-1].setRight( x );
			}
			else 
			{
				// new shape
				line.add( Segment(x,x,id) );
			}
		}

		// draw end
		LineBufferValue& line = buffer[Y1];
		if ( line.size() > 0 && line[line.size()-1].id() == id )
		{
			// continue existing shape
			if ( leftSide )
				line[line.size()-1].setLeft( X1 );
			else // right
				line[line.size()-1].setRight( X1 );
		}
		else 
		{
			// new shape
			line.add( Segment(X1,X1,id) );
		}
	}
}

void ShapeBuffer::clear( LineBuffer& lines )
{
	for ( LineBufferIterator it = lines.begin() ; it != lines.end() ; ++it )
	{
		LineBufferValue& line = *it;
		line.clear();
	}
}

bool ShapeBuffer::shapesOverlap( const LineBuffer& buffer, const LineBuffer& source, int Y0, int Y1, int* minDeltaX )
{
	assert( buffer.size() == source.size() );
	assert( Y0 >= 0 && Y0 < (int)buffer.size() );
	assert( Y1 >= 0 && Y1 < (int)buffer.size() );
	assert( Y0 <= Y1 );

	int maxOverlapCount = 0;

	for ( int y = Y0 ; y <= Y1 ; ++y )
	{
		// check all source segments against all buffer segments
		const LineBufferValue& sourceLine = source[y];
		for ( const Segment* sourceIt = sourceLine.begin() ; sourceIt != sourceLine.end() ; ++sourceIt )
		{
			const Segment& sourceSeg = *sourceIt;

			const LineBufferValue& bufferLine = buffer[y];
			for ( const Segment* bufferIt = bufferLine.begin() ; bufferIt != bufferLine.end() ; ++bufferIt )
			{
				const Segment& bufferSeg = *bufferIt;
				int overlapCount = bufferSeg.overlaps(sourceSeg);
				if ( overlapCount > maxOverlapCount )
				{
					// overlaps, we continue to find maximum overlap count
					maxOverlapCount = overlapCount;
				}
			}
		}
	}

	if (minDeltaX) *minDeltaX = maxOverlapCount;
	return maxOverlapCount > 0;
}

bool ShapeBuffer::shapeFits( const int* X, const int* Y, int vertices, const LineBuffer& buffer, int offsetX, int offsetY, int* minDeltaX ) const
{
	assert( (int)m_lines.size() == m_height );
	assert( m_tempLines.size() == m_lines.size() );

	clear( m_tempLines );
	drawShape( X, Y, vertices, 0, m_tempLines, offsetX, offsetY );

	int minY, maxY;
	minmax( Y, 3, &minY, &maxY );
	minY += offsetY;
	maxY += offsetY;

	return !shapesOverlap( buffer, m_tempLines, minY, maxY, minDeltaX );
}

bool ShapeBuffer::fitShape( const int* X, const int* Y, int vertices, const LineBuffer& buffer, int* offsetX, int* offsetY ) const
{
	// find shape origin and size
	int originX, originY;
	int width, height;
	minmax( X, vertices, &originX, &width ); width -= originX;
	minmax( Y, vertices, &originY, &height ); height -= originY;
	*offsetX = -originX;
	*offsetY = -originY;

	// try to fit the shape to every possible position
	for ( int y = 0 ; y+height < m_height ; ++y )
		for ( int x = 0 ; x+width < m_width ; )
		{
			int minDeltaX;
			if ( shapeFits(X,Y,vertices,buffer,x-originX,y-originY,&minDeltaX) )
			{
				*offsetX = x-originX;
				*offsetY = y-originY;
				return true;
			}
			else
			{
				x += minDeltaX;
			}
		}

	return false;
}

void ShapeBuffer::drawTriangle( int X0, int Y0, int X1, int Y1, int X2, int Y2, int id )
{
	int X[3] = { X0, X1, X2 };
	int Y[3] = { Y0, Y1, Y2 };
	drawShape( X, Y, 3, id, m_lines );
}

bool ShapeBuffer::fitTriangle( int X0, int Y0, int X1, int Y1, int X2, int Y2, int* offsetX, int* offsetY ) const
{
	int X[3] = { X0, X1, X2 };
	int Y[3] = { Y0, Y1, Y2 };
	return fitShape( X, Y, 3, m_lines, offsetX, offsetY );
}

void ShapeBuffer::drawRectangle( int X0, int Y0, int X1, int Y1, int id )
{
	int X[4] = { X0, X1, X1, X0 };
	int Y[4] = { Y0, Y0, Y1, Y1 };
	drawShape( X, Y, 4, id, m_lines );
}

bool ShapeBuffer::fitRectangle( int X0, int Y0, int X1, int Y1, int* offsetX, int* offsetY ) const
{
	int X[4] = { X0, X1, X1, X0 };
	int Y[4] = { Y0, Y0, Y1, Y1 };
	return fitShape( X, Y, 4, m_lines, offsetX, offsetY );
}

int ShapeBuffer::width() const
{
	return m_width;
}

int ShapeBuffer::height() const
{
	return m_height;
}


} // math
