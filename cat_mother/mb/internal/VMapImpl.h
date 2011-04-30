#include "Index.h"
#include <mb/VertexMapFormat.h>
#include <lang/String.h>
#include <util/Vector.h>
#include <util/Hashtable.h>


namespace mb
{


/**
 * Vertex map implementation.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class VMapImpl :
	public lang::Object
{
public:
	VMapImpl();

	void	setDimensions( int dim );
	void	setName( const lang::String& name );
	void	setFormat( const VertexMapFormat& format );
	void	addValue( int vertexIndex, int polygonIndex, const float* data, int size );
	bool	getValue( int vertexIndex, int polygonIndex, float* data, int size ) const;
	void	removeValue( int vertexIndex, int polygonIndex );
	void	polygonRemoved( int index );
	void	vertexRemoved( int index );
	int		dimensions() const;
	int		size() const;
	const lang::String&		name() const;
	const VertexMapFormat&	format() const;
	
	bool	operator==( const VMapImpl& other ) const;
	bool	operator!=( const VMapImpl& other ) const;

private:
	int					m_dimensions;
	lang::String		m_name;
	VertexMapFormat		m_format;

	util::Vector<float>					m_data;
	mutable util::Hashtable<Index,int>	m_indices;
};


} // mb
