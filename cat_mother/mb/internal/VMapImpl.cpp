#include "VMapImpl.h"
#include <dev/Profile.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

namespace mb
{


VMapImpl::VMapImpl() :
	m_dimensions( -1 ),
	m_name( "" ),
	m_format( VertexMapFormat::VERTEXMAP_UNKNOWN ),
	m_data( Allocator<float>(__FILE__,__LINE__) ),
	m_indices( Allocator< HashtablePair<Index,int> >(__FILE__,__LINE__) )
{
}

void VMapImpl::setDimensions( int dim )
{
	m_dimensions = dim;
}

void VMapImpl::setName( const String& name )
{
	m_name = name;
}

void VMapImpl::setFormat( const VertexMapFormat& format )
{
	m_format = format;
}

void VMapImpl::addValue( int vertexIndex, int polygonIndex, const float* data, int size )
{
	assert( m_dimensions == size );

	//Profile pr( "VMap.addValue" );

	// clamp count
	int count = size;
	if ( m_dimensions < count )
		count = m_dimensions;

	// get data index of (vertex,polygon) map entry
	Index key( vertexIndex, polygonIndex );
	int dataIndex = -1;
	if ( m_indices.containsKey(key) )
	{
		dataIndex = m_indices.get(key);
	}
	else
	{
		dataIndex = m_data.size();
		m_data.setSize( dataIndex + count );
		m_indices[key] = dataIndex;
	}

	// copy data
	assert( dataIndex >= 0 && dataIndex+m_dimensions <= m_data.size() );
	for ( int i = 0 ; i < count ; ++i )
		m_data[dataIndex+i] = data[i];
}

bool VMapImpl::getValue( int vertexIndex, int polygonIndex, float* data, int size ) const
{
	assert( m_dimensions == size );

	//Profile pr( "VMap.getValue" );

	Index key( vertexIndex, polygonIndex );
	bool found = m_indices.containsKey( key );
	if ( found )
	{
		// get data index
		int dataIndex = m_indices.get(key);
		assert( dataIndex >= 0 && dataIndex+m_dimensions <= m_data.size() );

		// clamp count
		int count = size;
		if ( m_dimensions < count )
			count = m_dimensions;

		// copy data
		for ( int i = 0 ; i < count ; ++i )
			data[i] = m_data[dataIndex+i];
	}

	return found;
}

void VMapImpl::removeValue( int vertexIndex, int polygonIndex )
{
	m_indices.remove( Index(vertexIndex,polygonIndex) );
}

void VMapImpl::polygonRemoved( int index )
{
	//Profile pr( "VMap.polygonRemoved" );

	Hashtable<Index,int> indices( Allocator< HashtablePair<Index,int> >(__FILE__,__LINE__) );
	for ( HashtableIterator<Index,int> it = m_indices.begin() ; it != m_indices.end() ; ++it )
	{
		if ( it.key().polygonIndex < index )
		{
			indices.put( Index( it.key().vertexIndex, it.key().polygonIndex ), it.value() );
		}
		if ( it.key().polygonIndex > index )
		{
			indices.put( Index( it.key().vertexIndex, it.key().polygonIndex-1 ), it.value() );
		}
	}

	m_indices = indices;
}

void VMapImpl::vertexRemoved( int index )
{
	//Profile pr( "VMap.vertexRemoved" );

	Hashtable<Index,int> indices( Allocator< HashtablePair<Index,int> >(__FILE__,__LINE__) );
	for ( HashtableIterator<Index,int> it = m_indices.begin() ; it != m_indices.end() ; ++it )
	{
		if ( it.key().vertexIndex < index )
		{
			indices.put( Index( it.key().vertexIndex, it.key().polygonIndex ), it.value() );
		}
		if ( it.key().vertexIndex > index )
		{
			indices.put( Index( it.key().vertexIndex-1, it.key().polygonIndex ), it.value() );
		}
	}

	m_indices = indices;
}

int VMapImpl::dimensions() const
{
	return m_dimensions;
}

const String& VMapImpl::name() const
{
	return m_name;
}

const VertexMapFormat& VMapImpl::format() const
{
	return m_format;
}

bool VMapImpl::operator==( const VMapImpl& other ) const
{
	if ( m_data.size() != other.m_data.size() )
		return false;

	if ( m_indices.size() != other.m_indices.size() )
		return false;

	for ( int i = 0 ; i < m_data.size() ; ++i )
	{
		if ( m_data[i] != other.m_data[i] )
			return false;
	}

	HashtableIterator<Index,int> it2 = other.m_indices.begin();
	for ( HashtableIterator<Index,int> it = m_indices.begin() ; it != m_indices.end() ; ++it )
	{
		if ( it.key() != it2.key() || it.value() != it2.value() )
			return false;
		++it2;
	}

	return true;
}

bool VMapImpl::operator!=( const VMapImpl& other ) const
{
	return !(*this == other);
}

int VMapImpl::size() const
{
	int c = m_indices.size();
	return c;
}


} // mb
