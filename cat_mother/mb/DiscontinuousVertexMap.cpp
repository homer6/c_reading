#include "DiscontinuousVertexMap.h"
#include "VMapImpl.h"
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace mb
{


DiscontinuousVertexMap::DiscontinuousVertexMap()
{
	m_this = new VMapImpl;
}

DiscontinuousVertexMap::~DiscontinuousVertexMap()
{
}

void DiscontinuousVertexMap::setDimensions( int dim )
{
	m_this->setDimensions( dim );
}

void DiscontinuousVertexMap::setName( const String& name )
{
	m_this->setName( name );
}

void DiscontinuousVertexMap::setFormat( const VertexMapFormat& format )
{
	m_this->setFormat( format );
}

void DiscontinuousVertexMap::addValue( int vertexIndex, int polygonIndex, const float* data, int size )
{
	m_this->addValue( vertexIndex, polygonIndex, data, size );
}

bool DiscontinuousVertexMap::getValue( int vertexIndex, int polygonIndex, float* data, int size ) const
{
	return m_this->getValue( vertexIndex, polygonIndex, data, size );
}

void DiscontinuousVertexMap::removeValue( int vertexIndex, int polygonIndex )
{
	m_this->removeValue( vertexIndex, polygonIndex );
}

int DiscontinuousVertexMap::dimensions() const
{
	return m_this->dimensions();
}

const String&	DiscontinuousVertexMap::name() const
{
	return m_this->name();
}

const VertexMapFormat& DiscontinuousVertexMap::format() const
{
	return m_this->format();
}

void DiscontinuousVertexMap::polygonRemoved( int index )
{
	m_this->polygonRemoved( index );
}

void DiscontinuousVertexMap::vertexRemoved( int index )
{
	m_this->vertexRemoved( index );
}

bool DiscontinuousVertexMap::operator==( const DiscontinuousVertexMap& other ) const
{
	return *m_this == *other.m_this;
}

bool DiscontinuousVertexMap::operator!=( const DiscontinuousVertexMap& other ) const
{
	return *m_this != *other.m_this;
}


} // mb
