inline void Model::setIndices( int firstIndex, const int* vertices, int count )
{
	assert( firstIndex >= 0 && firstIndex+count <= maxIndices() );
	assert( canWriteIndices() );
	IndexType* ind = m_indexData + firstIndex;
	for ( int i = 0 ; i < count ; ++i )
	{
		assert( vertices[i] >= 0 && vertices[i] < this->vertices() );
		ind[i] = (IndexType)vertices[i];
	}
	//m_this->mesh->setIndices( firstIndex, vertices, count );
}

inline void Model::getIndices( int firstIndex, int* vertices, int count ) const
{
	assert( firstIndex >= 0 && firstIndex+count <= maxIndices() );
	assert( canReadIndices() );

	IndexType* ind = m_indexData + firstIndex;
	for ( int i = 0 ; i < count ; ++i )
		vertices[i] = ind[i];
	//m_this->mesh->getIndices( firstIndex, vertices, count );
}

inline void Model::setVertexPositions( int firstVertex, const math::Vector3* positions, int count )
{
	assert( firstVertex >= 0 && firstVertex+count <= maxVertices() );
	assert( canWriteVertices() );
	
	float* begin = m_posData + firstVertex * m_posPitch;
	const math::Vector3* endv = positions + count;
	for ( ; positions != endv ; ++positions )
	{
		begin[0] = positions->x;
		begin[1] = positions->y;
		begin[2] = positions->z;
		begin += m_posPitch;
	}
	//m_this->mesh->setVertexPositions( firstVertex, positions, count );
}

inline void Model::getVertexPositions( int firstVertex, math::Vector3* positions, int count ) const
{
	assert( firstVertex >= 0 && firstVertex+count <= maxVertices() );
	assert( canReadVertices() );
	
	const float* begin = m_posData + firstVertex * m_posPitch;
	math::Vector3* endv = positions + count;
	for ( ; positions != endv ; ++positions )
	{
		positions->x = begin[0];
		positions->y = begin[1];
		positions->z = begin[2];
		begin += m_posPitch;
	}
	//m_this->mesh->getVertexPositions( firstVertex, positions, count );
}
