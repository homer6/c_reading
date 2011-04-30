#ifndef _MESHBUILDER_VERTEXREFERENCE_H
#define _MESHBUILDER_VERTEXREFERENCE_H


#include "LinkedItem.h"


namespace mb
{


class Vertex;


/**
 * Link to a vertex.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class VertexReference :
	public LinkedItem
{
public:
	Vertex*		vertex;

	VertexReference()																: vertex(0) {}

private:
	VertexReference( const VertexReference& );
	VertexReference& operator=( const VertexReference& );
};


} // mb


#endif // _MESHBUILDER_VERTEXREFERENCE_H
