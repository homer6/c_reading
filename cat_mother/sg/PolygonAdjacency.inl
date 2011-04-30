inline void PolygonAdjacency::getAdjacent( int poly, int* adj, int edges ) const
{
	assert( poly >= 0 && poly*m_edges < m_adj.size() );
	assert( edges == m_edges );

	const int* p = m_adj.begin() + poly*edges;
	for ( int i = 0 ; i < edges ; ++i )
		adj[i] = p[i];
}
