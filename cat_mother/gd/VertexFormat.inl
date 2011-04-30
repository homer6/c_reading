inline VertexFormat::VertexFormat()
{
	m_vf		= VF_XYZ;
	m_texcoords = 0;
	m_weights	= 0;
}

inline bool VertexFormat::operator==( const VertexFormat& other ) const
{
	for ( int i = 0 ; i < m_texcoords ; ++i )
		if ( m_texcoordSizes[i] != other.m_texcoordSizes[i] )
			return false;

	return 
		m_vf == other.m_vf && 
		m_texcoords == other.m_texcoords &&
		m_weights == other.m_weights;
}

inline bool VertexFormat::operator!=( const VertexFormat& other ) const
{
	for ( int i = 0 ; i < m_texcoords ; ++i )
		if ( m_texcoordSizes[i] != other.m_texcoordSizes[i] )
			return true;

	return 
		m_vf != other.m_vf ||
		m_texcoords != other.m_texcoords ||
		m_weights != other.m_weights;
}

inline VertexFormat& VertexFormat::addRHW()
{
	m_vf |= VF_RHW;
	return *this;
}

inline VertexFormat& VertexFormat::addNormal()
{
	m_vf |= VF_NORMAL;
	return *this;
}


inline VertexFormat& VertexFormat::addDiffuse()
{
	m_vf |= VF_DIFFUSE;
	return *this;
}

inline VertexFormat& VertexFormat::addSpecular()
{
	m_vf |= VF_SPECULAR;
	return *this;
}

inline VertexFormat& VertexFormat::removeRHW()
{
	m_vf &= ~VF_RHW;
	return *this;
}

inline VertexFormat& VertexFormat::removeNormal()
{
	m_vf &= ~VF_NORMAL;
	return *this;
}
	
inline VertexFormat& VertexFormat::removeDiffuse()
{
	m_vf &= ~VF_DIFFUSE;
	return *this;
}
	
inline VertexFormat& VertexFormat::removeSpecular()
{
	m_vf &= ~VF_SPECULAR;
	return *this;
}

inline VertexFormat& VertexFormat::setWeights( int weights )
{
	assert( weights >= 0 );
	
	if ( weights > 3 ) // max 4 affecting bones per vertex
		weights = 3;
	m_weights = (unsigned char)weights;
	return *this;
}

inline VertexFormat& VertexFormat::addTextureCoordinate( int dim )
{
	assert( dim == 1 || dim == 2 || dim == 3 || dim == 4 );
	assert( m_texcoords < MAX_LAYERS );

	if ( m_texcoords < MAX_LAYERS )
		m_texcoordSizes[m_texcoords++] = (unsigned char)dim;
	return *this;
}

inline void VertexFormat::setTextureCoordinateSize( int layer, int dim )
{
	assert( layer >= 0 && layer < m_texcoords );
	assert( dim == 1 || dim == 2 || dim == 3 || dim == 4 );

	m_texcoordSizes[layer] = (unsigned char)dim;
}

inline void VertexFormat::setTextureCoordinates( int layers )
{
	assert( layers >= 0 );
	assert( layers <= MAX_LAYERS );
	
	if ( layers > MAX_LAYERS )
		layers = MAX_LAYERS;
	m_texcoords = (unsigned char)layers;
}

inline bool VertexFormat::hasRHW() const
{
	return VF_RHW == (m_vf & VF_RHW);
}

inline bool VertexFormat::hasNormal() const
{
	return VF_NORMAL == (m_vf & VF_NORMAL);
}

inline bool VertexFormat::hasDiffuse() const
{
	return VF_DIFFUSE == (m_vf & VF_DIFFUSE);
}

inline bool VertexFormat::hasSpecular() const
{
	return VF_SPECULAR == (m_vf & VF_SPECULAR);
}

inline int VertexFormat::textureCoordinates() const
{
	return m_texcoords;
}

inline int VertexFormat::weights() const
{
	return m_weights;
}

inline int	VertexFormat::getTextureCoordinateSize( int layerIndex ) const
{
	assert( layerIndex < m_texcoords );
	return m_texcoordSizes[ layerIndex ];
}

inline int	VertexFormat::totalTextureCoordinateSize() const
{
	int size = 0;
	for ( int i = 0 ; i < m_texcoords ; ++i )
		size += m_texcoordSizes[i];
	return size;
}
