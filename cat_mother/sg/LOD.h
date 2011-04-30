#ifndef _SG_LOD_H
#define _SG_LOD_H


#include <sg/Node.h>


namespace sg
{


class Node;


/** 
 * Level of detail container. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class LOD :
	public Node
{
public:
	///
	LOD();

	///
	~LOD();

	/** 
	 * Selects current level of detail. 
	 * @return false always (container does not need to be rendered).
	 */
	bool		updateVisibility( Camera* camera );

	/** 
	 * Selects current level of detail using specified LOD world position. 
	 * @return true if LOD container is visible.
	 */
	bool		selectLOD( const math::Vector3& lodWorldPos, Camera* camera );

	/** Does nothing; LOD is only container for other hierarchies. */
	void		render( Camera* camera, int pass );

	/** 
	 * Adds a level of detail to the container. 
	 * @param lod Level of detail.
	 * @param lod Minimum size (incl) for the level of detail to be visible.
	 * @param lod Maximum size (excl) for the level of detail to be visible.
	 */
	void		add( Node* lod, float lodMin, float lodMax );

	/** Removes a lod from the container. */
	void		remove( int index );

	/** Sets world space radius used for selecting detail level. */
	void		setRadius( float d );

	/** Returns specified lod of the container. */
	Node*		get( int index ) const;

	/** Returns number of lods in the container. */
	int			lods() const;

	/** Returns world space radius used for selecting detail level. */
	float		radius() const;

	/** Returns active LOD level (0==highest) or -1 if not visible. */
	int			level() const;

	/** Returns bounding radius of the node. (same as radius()) */
	float		boundSphere() const;

private:
	class LODImpl;
	P(LODImpl) m_this;

	LOD( const LOD& other );
	LOD& operator=( const LOD& );
};


} // sg


#endif // _SG_LOD_H
