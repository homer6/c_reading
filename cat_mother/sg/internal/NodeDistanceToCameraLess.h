#ifndef _SG_NODEDISTANCETOCAMERALESS_H
#define _SG_NODEDISTANCETOCAMERALESS_H


namespace sg
{


/** 
 * Predicate for comparing node distances to camera along camera Z-axis. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class NodeDistanceToCameraLess
{ 
public: 
	bool operator()( const Node* const a, const Node* const b ) const 
	{
		return a->cachedDistanceToCamera() < b->cachedDistanceToCamera();
	} 
};


} // sg


#endif // _SG_NODEDISTANCETOCAMERALESS_H
