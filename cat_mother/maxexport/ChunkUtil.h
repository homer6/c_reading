#include <util/Vector.h>
#include <math/Vector3.h>


class KeyFrameContainer;

namespace io {
	class ChunkOutputStream;}

namespace lang {
	class String;}


/** 
 * Utilities for writing simple data chunks. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ChunkUtil
{
public:
	static void writeAnimChunk( io::ChunkOutputStream* out, const lang::String& name, const KeyFrameContainer& anim );

	static void writeIntChunk( io::ChunkOutputStream* out, const lang::String& name, int x );
	static void writeIntChunk2( io::ChunkOutputStream* out, const lang::String& name, int x1, int x2 );
	static void writeIntChunk3( io::ChunkOutputStream* out, const lang::String& name, int x1, int x2, int x3 );
	static void writeFloatChunk( io::ChunkOutputStream* out, const lang::String& name, float x );
	static void writeFloatChunk2( io::ChunkOutputStream* out, const lang::String& name, float x1, float x2 );
	static void writeFloatChunk3( io::ChunkOutputStream* out, const lang::String& name, float x1, float x2, float x3 );
	static void writeFloatChunk4( io::ChunkOutputStream* out, const lang::String& name, float x1, float x2, float x3, float x4 );
	static void writeStringChunk( io::ChunkOutputStream* out, const lang::String& name, const lang::String& x );

	static void writeAnim( io::ChunkOutputStream* out, const KeyFrameContainer& anim );
	static void writeVector3( io::ChunkOutputStream* out, const math::Vector3& v );
	static void writeTriangleList( io::ChunkOutputStream* out, const util::Vector<math::Vector3>& vec );
};
