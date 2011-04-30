#ifndef _GMMODEL_H
#define _GMMODEL_H


#include "GmMorphChannel.h"
#include "GmPatch.h"
#include "GmMaterial.h"
#include "GmModelPrimitive.h"
#include "GmLineList.h"
#include <mb/MeshBuilder.h>
#include <lang/String.h>
#include <util/Vector.h>


namespace io {
	class ChunkOutputStream;}

namespace mb {
	class Vertex;
	class Polygon;
	class DiscontinuousVertexMap;}

class GmModelPrimitive;


/** 
 * Geometry model.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GmModel :
	public mb::MeshBuilder
{
public:
	/** Source 3ds node. */
	INode*										node3ds;

	/** Morpher channels if present. */
	util::Vector<P(GmMorphChannel)>				morphChannels;

	/** Ptr to morpher base if this model is a morph target. */
	GmModel*									morphBase;

	/** Name of the geometry object. */
	lang::String								name;

	/** Name of the geometry object file name. */
	lang::String								filename;

	/** Materials used by the geometry. */
	util::Vector<P(GmMaterial)>					materials;

	/** Cubic Bezier patches. */
	util::Vector<GmPatch>						patches;

	/** Primitives created from the model. */
	util::Vector<P(GmModelPrimitive)>			primitives;

	/** Line lists. */
	util::Vector<P(GmLineList)>					lineLists;

	GmModel();
	~GmModel();

	void	splitToPrimitives();

	void	write( io::ChunkOutputStream* out );

	/** Two models are equal if all materials, vertex and polygon data match. */
	bool	operator==( const GmModel& other ) const;

	/** Returns name of the morpher channel using this morph target. Requires that the model has morphBase set. */
	lang::String	morphChannelName() const;

private:
	void	writeLineList( io::ChunkOutputStream* out, GmLineList* lineList );
	void	writeLineLists( io::ChunkOutputStream* out );
	void	writeMaterials( io::ChunkOutputStream* out );
	void	writeModelPrimitives( io::ChunkOutputStream* out );
	void	writeModelPrimitive( io::ChunkOutputStream* out, GmModelPrimitive* mp );
	void	writeDVMap( io::ChunkOutputStream* out, const lang::String& chunkName, mb::DiscontinuousVertexMap* vmad, const util::Vector<mb::Vertex*>& vertices );
	void	writeDVMapByteValues( io::ChunkOutputStream* out, const lang::String& chunkName, mb::DiscontinuousVertexMap* vmad, const util::Vector<mb::Vertex*>& vertices );
	void	writePatchList( io::ChunkOutputStream* out );
	void	writeMorpher( io::ChunkOutputStream* out );
	void	writeMorphTargets( io::ChunkOutputStream* out );
	void	writeMorphTarget( io::ChunkOutputStream* out, GmModelPrimitive* mp );
	void	getMorphDeltas( GmModelPrimitive* base, util::Vector<int>* vertexIndices, util::Vector<math::Vector3>* vertexDeltas, float* maxDeltaLen );

	GmModel( const GmModel& );
	GmModel& operator=( const GmModel& );
};


#endif // _GMMODEL_H

