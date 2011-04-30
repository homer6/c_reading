#ifndef _GMMODELPRIMITIVE_H
#define _GMMODELPRIMITIVE_H


#include <util/Vector.h>


namespace mb {
	class DiscontinuousVertexMap;
	class VertexMap;
	class Vertex;
	class Polygon;}

class GmMaterial;


/** 
 * Part of model using only single material. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GmModelPrimitive :
	public lang::Object
{
public:
	util::Vector<mb::Vertex*>					vertices;
	util::Vector<mb::Polygon*>					polys;
	P(GmMaterial)								mat;
	int											matIndex;
	util::Vector<mb::DiscontinuousVertexMap*>	texlayers;
	mb::DiscontinuousVertexMap*					vertcolor;
	mb::DiscontinuousVertexMap*					normals;
	util::Vector<mb::VertexMap*>				weightMaps;
	int											maxWeightsPerVertex;
	util::Vector<int>							vertexBoneIndices;
	util::Vector<float>							vertexBoneWeights;
	util::Vector<int>							vertexBoneCounts;

	GmModelPrimitive();
	~GmModelPrimitive();

private:
	GmModelPrimitive( const GmModelPrimitive& );
	GmModelPrimitive& operator=( const GmModelPrimitive& );
};


#endif // _GMMODELPRIMITIVE_H
