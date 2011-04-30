namespace mb
{


/**
 * Vertex/polygon index pair.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Index
{
public:
	int		vertexIndex;
	int		polygonIndex;	

	Index()																		: vertexIndex(-1), polygonIndex(-1) {}
	Index( int vertexIndex, int polygonIndex )									{this->vertexIndex=vertexIndex; this->polygonIndex=polygonIndex;}

	bool	operator<( const Index& other ) const								{if ( vertexIndex < other.vertexIndex ) return true; if ( vertexIndex > other.vertexIndex ) return false; return polygonIndex < other.polygonIndex;}
	bool	operator==( const Index& other ) const								{return vertexIndex == other.vertexIndex && polygonIndex == other.polygonIndex;}
	bool	operator!=( const Index& other ) const								{return vertexIndex != other.vertexIndex || polygonIndex != other.polygonIndex;}
	int		hashCode() const													{return (polygonIndex << 16) + vertexIndex;}
};


} // mb
