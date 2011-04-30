#ifndef _MESHBUILDER_POLYGONREFERENCE_H
#define _MESHBUILDER_POLYGONREFERENCE_H


#include "LinkedItem.h"


namespace mb
{


class Polygon;


/**
 * Link to a polygon.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class PolygonReference :
	public LinkedItem
{
public:
	Polygon*	polygon;

	PolygonReference()																: polygon(0) {}

private:
	PolygonReference( const PolygonReference& );
	PolygonReference& operator=( const PolygonReference& );
};


} // mb


#endif // _MESHBUILDER_POLYGONREFERENCE_H
