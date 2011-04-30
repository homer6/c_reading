#include "VertexMap.h"
#include "VMapImpl.h"
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace mb
{


VertexMap::VertexMap()
{
	m_this = new VMapImpl;
}

VertexMap::~VertexMap()
{
}

void VertexMap::setDimensions( int dim )
{
	m_this->setDimensions( dim );
}

void VertexMap::setName( const String& name )
{
	m_this->setName( name );
}

void VertexMap::setFormat( const VertexMapFormat& format )
{
	m_this->setFormat( format );
}

void VertexMap::addValue( int vertexIndex, const float* data, int size )
{
	m_this->addValue( vertexIndex, 0, data, size );
}

bool VertexMap::getValue( int vertexIndex, float* data, int size ) const
{
	return m_this->getValue( vertexIndex, 0, data, size );
}

void VertexMap::removeValue( int vertexIndex )
{
	m_this->removeValue( vertexIndex, 0 );
}

int VertexMap::dimensions() const
{
	return m_this->dimensions();
}

const String&	VertexMap::name() const
{
	return m_this->name();
}

const VertexMapFormat& VertexMap::format() const
{
	return m_this->format();
}

void VertexMap::vertexRemoved( int index )
{
	m_this->vertexRemoved( index );
}

bool VertexMap::operator==( const VertexMap& other ) const
{
	return *m_this == *other.m_this;
}

bool VertexMap::operator!=( const VertexMap& other ) const
{
	return *m_this != *other.m_this;
}

int VertexMap::size() const
{
	return m_this->size();
}


} // mb
