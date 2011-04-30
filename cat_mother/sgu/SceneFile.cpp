#include "SceneFile.h"
#include "ModelFile.h"
#include "ChunkUtil.h"
#include "ModelFileCache.h"
#include "LookAtControl.h"
#include <sg/Mesh.h>
#include <sg/Dummy.h>
#include <sg/PointLight.h>
#include <sg/DirectLight.h>
#include <sg/SpotLight.h>
#include <sg/Scene.h>
#include <sg/Camera.h>
#include <sg/LOD.h>
#include <sg/Model.h>
#include <sg/VertexAndIndexLock.h>
#include <sg/Primitive.h>
#include <sg/ShadowVolume.h>
#include <sg/VertexFormat.h>
#include <sg/TriangleList.h>
#include <io/File.h>
#include <io/IOException.h>
#include <io/FileInputStream.h>
#include <io/ChunkInputStream.h>
#include <io/InputStreamArchive.h>
#include <pix/Colorf.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include <lang/String.h>
#include <util/Vector.h>
#include <math/Quaternion.h>
#include <math/OBBoxBuilder.h>
#include <anim/VectorInterpolator.h>
#include <anim/QuaternionInterpolator.h>
#include <algorithm>
#include "config.h"

//-----------------------------------------------------------------------------

#define SG_FILE_VER 0x122
#define SG_FILE_VER_MASK 0xFF0

//-----------------------------------------------------------------------------

using namespace io;
using namespace sg;
using namespace pix;
using namespace lang;
using namespace util;
using namespace math;
using namespace anim;

//-----------------------------------------------------------------------------

namespace sgu
{


class SceneFile::SceneFileImpl :
	public lang::Object
{
public:
	class Bone
	{
	public:
		int				index;
		Matrix4x4		rest;
	};

	class DynamicShadow :
		public Object
	{
	public:
		String		model;
		Vector3		light;
		float		length;
	};

	class Item :
		public Object
	{
	public:
		P(Node)				node;
		int					parent;
		int					target;
		String				model;
		Vector<Bone>		bones;
		int					lodID;
		float				lodMin;
		float				lodMax;
		P(DynamicShadow)	dynamicShadow;

		Item() :
			node(0),
			parent(-1),
			target(-1),
			bones( Allocator<Bone>(__FILE__,__LINE__) ),
			lodID(-1),
			lodMin(0),
			lodMax(0),
			dynamicShadow(0)
		{
		}
	};

	class BoneNameLess
	{
	public:
		explicit BoneNameLess( const Vector< P(Item) >* items )	: 
			m_items( items ) 
		{
		}

		bool operator()( const Bone& a, const Bone& b )								
		{
			return m_items->operator[](a.index)->node->name() < 
				m_items->operator[](b.index)->node->name();
		}
	
	private:
		const Vector< P(Item) >* m_items;
	};

	int							ver;
	int							loadFlags;
	String						name;
	String						path;
	P(Scene)					scene;
	P(Camera)					camera;
	Vector< P(Item) >			items;

	SceneFileImpl( const String& sceneName, ModelFileCache* modelcache,
		InputStreamArchive* zip, int flags ) :
		items( Allocator<P(Item)>(__FILE__,__LINE__) )
	{
		try
		{
			create( sceneName, modelcache, zip, flags );
		}
		catch ( ... )
		{
			Debug::println( "sgu.SceneFile: Failed to load scene {0}", sceneName );
			throw;
		}
	}

	void create( const String& sceneName,
		ModelFileCache* modelcache, InputStreamArchive* zip, int flags )
	{
		assert( zip );

		loadFlags = flags;
		name = sceneName;
		path = File(sceneName).getParent();
		P(InputStream) ins = zip->getInputStream( sceneName );
		P(ChunkInputStream) in = new ChunkInputStream( ins );

		// create scene
		scene = new Scene;
		scene->setName( name );
		scene->setAmbientColor( Color(32,32,32) );
	
		// read top level scene chunk
		String str;
		long end;
		in->beginChunk( &str, &end );
		if ( str != "sg" )
			throw IOException( Format("Corrupted scene file: {0}", name) );
		readScene( in, end );
		in->endChunk( end );

		// DEBUG: print nodes and indices
		//for ( int i = 0 ; i < items.size() ; ++i )
		//	Debug::println( "Node[{0}] = {1}", i, items[i]->node->name() );

		// connect parents and targets
		for ( int i = 0 ; i < items.size() ; ++i )
		{
			Item* item = items[i];

			// parent
			int index = item->parent;
			if ( -1 == index )
			{
				item->node->linkTo( scene );
			}
			else
			{
				if ( index < 0 || index >= items.size() )
					throw IOException( Format("Item \"{0}\" has invalid parent index ({1}) in scene file {2}.", item->node->name(), index, name) );
				item->node->linkTo( items[index]->node );
				//if ( item->node->name() == "Bip01 Spine" )
				//	Debug::printlnWarning( "{2}: {0} is parented to {1}", item->node->name(), items[index]->node->name(), name );
			}

			// target
			index = item->target;
			if ( -1 != index )
			{
				if ( index < 0 || index >= items.size() )
					throw IOException( Format("Item \"{0}\" has invalid target index ({1}) in scene file {2}.", item->node->name(), index, name) );
				item->node->setRotationController( new LookAtControl(item->node,items[index]->node) );
				//Debug::println( "Node {0} is targeted to {1} (index={2})", item->node->name(), items[index]->node->name(), index );
			}
		}

		// add bones, geometry and dynamic shadows
		Vector<String> boneNames( Allocator<String>(__FILE__,__LINE__) );
		for ( int i = 0 ; i < items.size() ; ++i )
		{
			Item* item = items[i];
			Mesh* mesh = dynamic_cast<Mesh*>( item->node.ptr() );

			if ( mesh && item->model.length() > 0 )
			{
				// check bone index ranges
				int k;
				for ( k = 0 ; k < item->bones.size() ; ++k )
				{
					int index = item->bones[k].index;
					if ( index < 0 || index >= items.size() )
						throw IOException( Format("Mesh \"{0}\" has invalid bone index ({1}) in scene file {2}.", item->node->name(), index, name) );
				}
	
				// sort mesh bones to abc order
				std::sort( item->bones.begin(), item->bones.end(), BoneNameLess(&items) );

				// add bones to mesh and list their names
				boneNames.clear();
				for ( k = 0 ; k < item->bones.size() ; ++k )
				{
					int index = item->bones[k].index;
					Matrix4x4 rest = item->bones[k].rest;
					Node* bonenode = items[index]->node;
					mesh->addBone( bonenode, rest );
					boneNames.add( bonenode->name() );
					bonenode->setRenderable( false ); // bones don't need to be rendered
				}

				if ( loadFlags & LOAD_GEOMETRY )
				{
					// add geometry to mesh
					assert( item->model.length() > 0 );
					String modelPath = File(path,item->model).getPath();
					P(ModelFile) modelfile = modelcache->getByName( modelPath, boneNames.begin(), boneNames.size(), Colorf(scene->ambientColor()) );
					for ( int j = 0 ; j < modelfile->primitives() ; ++j )
					{
						Primitive* prim = modelfile->getPrimitive( j );
						mesh->addPrimitive( prim );
					}

					// add dynamic shadows
					DynamicShadow* dynamicShadow = item->dynamicShadow;
					if ( dynamicShadow )
					{
						assert( dynamicShadow->model.length() > 0 );
						String modelPath = File(path,dynamicShadow->model).getPath();
						P(ModelFile) modelfile = modelcache->getByName( modelPath, boneNames.begin(), boneNames.size(), Colorf(scene->ambientColor()) );
						for ( int j = 0 ; j < modelfile->primitives() ; ++j )
						{
							Primitive* prim = modelfile->getPrimitive( j );
							Model* model = dynamic_cast<Model*>( prim );
							if ( model )
							{
								P(ShadowVolume) svol = new ShadowVolume( model, dynamicShadow->light, dynamicShadow->length );
								mesh->addPrimitive( svol );
							}
						}
					}
				}
			}
		}

		// connect LODs
		for ( int i = 0 ; i < items.size() ; ++i )
		{
			Item* item = items[i];
			if ( -1 != item->lodID )
			{
				// add mesh to LOD container
				assert( item->parent != -1 );
				Node* parentnode = items[item->parent]->node;
				LOD* lod = dynamic_cast<LOD*>( parentnode );
				assert( lod );
				Node* node = item->node;
				Mesh* mesh = dynamic_cast<Mesh*>( node );
				assert( mesh );
				lod->add( mesh, item->lodMin, item->lodMax );
			}
		}

		// compute LOD diameters
		for ( int i = 0 ; i < items.size() ; ++i )
		{
			Node* node = items[i]->node;
			LOD* lod = dynamic_cast<LOD*>( node );
			if ( lod )
			{
				// compute oriented bounding box from mesh models
				OBBoxBuilder obbb;
				while ( obbb.nextPass() )
				{
					for ( int k = 0 ; k < lod->lods() ; ++k )
					{
						Mesh* mesh = dynamic_cast<Mesh*>( lod->get(k) );
						if ( mesh )
						{
							Matrix4x4 tm = mesh->worldTransform();
							Vector3 v;

							for ( int j = 0 ; j < mesh->primitives() ; ++j )
							{
								Primitive* prim = mesh->getPrimitive(j);
								Model* model = dynamic_cast<Model*>( prim );

								if ( model )
								{
									VertexAndIndexLock<Model> lock( model, Model::LOCK_READ );
									for ( int n = 0 ; n < model->vertices() ; ++n )
									{
										model->getVertexPositions( n, &v, 1 );
										v = tm.transform( v );
										obbb.addPoints( &v, 1 );
									}
								}
							} // for ( int j = 0 ; j < mesh->primitives() ; ++j )
						}
					}
				}
				OBBox box = obbb.box();
				
				// set diameter used for selecting detail level
				Vector3 dim = box.dimensions();
				float maxr = 0.f;
				for ( int k = 0 ; k < 3 ; ++k )
				{
					if ( dim[k] > maxr )
						maxr = dim[k];
				}
				lod->setRadius( maxr );
			}
		}

		// find the first camera
		for ( int i = 0 ; i < items.size() ; ++i )
		{
			Node* node = items[i]->node;
			camera = dynamic_cast<Camera*>( node );
			if ( camera )
				break;
		}

		// set initial animation state
		for ( Node* node = scene ; node ; node = node->nextInHierarchy() )
			node->setState( 0.f );

		items.clear();
		items.trimToSize();

		//Debug::println( "Scene {0} animation end {1} frames", name, scene->animationEnd()*30.f );
	}

	void readScene( ChunkInputStream* in, long end )
	{
		String str;
		long subend;

		// version
		ver = in->readInt();
		if ( (ver&SG_FILE_VER_MASK) != (SG_FILE_VER&SG_FILE_VER_MASK) )
			throw IOException( Format("Invalid scene file version (expected {1,x}, got {2,x}): {0}", name, SG_FILE_VER, ver) );
		
		// subchunks
		int nodeIndex = 0;
		for ( ; in->size() < end ; in->endChunk(subend) )
		{
			in->beginChunk( &str, &subend );

			if ( str == "environment" )
				readEnvironment( in, subend );
			else if ( str == "node" )
				readNode( in, subend ), ++nodeIndex;
			else if ( str == "dummy" )
				readDummy( in, subend ), ++nodeIndex;
			else if ( str == "mesh" )
				readMesh( in, subend ), ++nodeIndex;
			else if ( str == "pointlight" )
				readPointLight( in, subend ), ++nodeIndex;
			else if ( str == "spotlight" )
				readSpotLight( in, subend ), ++nodeIndex;
			else if ( str == "directlight" )
				readDirectLight( in, subend ), ++nodeIndex;
			else if ( str == "ambientlight" )
				readAmbientLight( in, subend ), ++nodeIndex;
			else if ( str == "camera" )
				readCamera( in, subend ), ++nodeIndex;
			else if ( str == "lod" )
				readLOD( in, subend ), ++nodeIndex;
			else
				throw IOException( Format("Unknown top level chunk ({0}) in {1}", str, name) );
		}
	}

	Vector3 readVector3( ChunkInputStream* in )
	{
		Vector3 v;
		for ( int i = 0 ; i < 3 ; ++i )
			v[i] = in->readFloat();
		return v;
	}

	Matrix4x4 readMatrix4x4( ChunkInputStream* in )
	{
		Matrix4x4 m;
		for ( int j = 0 ; j < 4 ; ++j )
			for ( int i = 0 ; i < 4 ; ++i )
				m(j,i) = in->readFloat();
		return m;
	}

	Colorf readColorf( ChunkInputStream* in )
	{
		Colorf c;
		c.setRed( in->readFloat() );
		c.setGreen( in->readFloat() );
		c.setBlue( in->readFloat() );
		c.setAlpha( 1.f );
		return c;
	}

	P(TriangleList) readTriangleList( ChunkInputStream* in,
		const VertexFormat& vf )
	{
		int triangles = in->readInt();
		int verts = triangles * 3;
		if ( triangles < 1 || triangles >= 65535/3 )
			throw IOException( Format( "Invalid triangle count ({1,#}) in triangle list {0}", in->toString(), triangles ) );

		P(TriangleList) tri = new TriangleList( verts, vf );
		VertexLock<TriangleList> trilock( tri, TriangleList::LOCK_WRITE );

		Vector3 v;
		for ( int i = 0 ; i < verts ; ++i )
		{
			// position
			for ( int k = 0 ; k < 3 ; ++k )
				v[k] = in->readFloat();
			if ( !v.finite() )
				throw IOException( Format( "Invalid triangle list data in {0}", in->toString() ) );
			tri->setVertexPositions( i, &v, 1 );
		}

		return tri;
	}

	Vector4 readVector4( ChunkInputStream* in )
	{
		Vector4 v;
		for ( int i = 0 ; i < 4 ; ++i )
			v[i] = in->readFloat();
		return v;
	}

	Bone readBone( ChunkInputStream* in )
	{
		Bone bone;
		bone.index = in->readInt();
		bone.rest = readMatrix4x4( in );
		return bone;
	}

	Item* addItem()
	{
		items.add( new Item );
		return items.lastElement();
	}

	void readAnim( ChunkInputStream* in, Interpolator* anim, const String& name )
	{
		if ( loadFlags & LOAD_ANIMATIONS )
			ChunkUtil::readAnim( in, anim, name );
		else
			ChunkUtil::readAnim( in, anim, name, ChunkUtil::NO_ANIMATIONS );

		// update end-of-animation mark
		float animEnd = 0.f;
		if ( anim->keys() > 0 )
			animEnd = anim->getKeyTime( anim->keys()-1 );
		if ( animEnd > scene->animationEnd() )
			scene->setAnimationEnd( animEnd );
	}

	bool readNodeChunk( const String& name, ChunkInputStream* in, long /*end*/, Item* item, Node* node )
	{
		if ( name == "name" )
		{
			String str = in->readString();
			node->setName( str );
			return true;
		}
		else if ( name == "enabled" )
		{
			node->setEnabled( 0 != in->readInt() );
			return true;
		}
		else if ( name == "renderable" )
		{
			node->setRenderable( 0 != in->readInt() );
			return true;
		}
		else if ( name == "parent" )
		{
			item->parent = in->readInt();
			return true;
		}
		else if ( name == "target" )
		{
			item->target = in->readInt();
			return true;
		}
		else if ( name == "pos" )
		{
			P(VectorInterpolator) anim = new VectorInterpolator(3);
			readAnim( in, anim, "position" );
			if ( anim->keys() > 1 )
			{
				node->setPositionController( anim );
			}
			else
			{
				float v[3];
				anim->getKeyValue( 0, v, 3 );
				node->setPosition( Vector3(v[0],v[1],v[2]) );
			}
			//Debug::println( "SceneFile: Read position animation of {0}, start={1}, end={2}", node->name(), anim->getKeyTime(0)*30.f, anim->endTime()*30.f );
			return true;
		}
		else if ( name == "rotq" )
		{
			P(QuaternionInterpolator) anim = new QuaternionInterpolator;
			readAnim( in, anim, "rotation" );
			if ( anim->keys() > 1 )
			{
				node->setRotationController( anim );
			}
			else
			{
				float v[4];
				anim->getKeyValue( 0, v, 4 );
				node->setRotation( Matrix3x3(Quaternion(v[0],v[1],v[2],v[3])) );
			}
			return true;
		}
		else if ( name == "scl" )
		{
			P(VectorInterpolator) anim = new VectorInterpolator(3);
			readAnim( in, anim, "scale" );
			float scl[3] = {1,1,1};
			anim->getKeyValue( 0, scl, 3 );
			if ( anim->keys() > 1 ||
				Math::abs(scl[0]-1.f) > 1e-4f ||
				Math::abs(scl[1]-1.f) > 1e-4f ||
				Math::abs(scl[2]-1.f) > 1e-4f )
			{
				node->setScaleController( anim );
			}
			return true;
		}
		return false;
	}

	bool readMeshChunk( const String& name, ChunkInputStream* in, long end, Item* item, Mesh* mesh )
	{
		if ( readNodeChunk(name, in, end, item, mesh) )
			return true;

		if ( name == "model" )
		{
			item->model = in->readString();
			return true;
		}
		else if ( name == "bones" )
		{
			int count = in->readInt();
			for ( int i = 0 ; i < count ; ++i )
			{
				Bone bone = readBone( in );
				//bone.rest.setRotation( bone.rest.rotation().orthonormalize() );
				item->bones.add( bone );
			}
			return true;
		}
		else if ( name == "lod" )
		{
			item->lodID = in->readInt();
			item->lodMin = in->readFloat();
			item->lodMax = in->readFloat();
			return true;
		}
		else if ( name == "staticshadow" )
		{
			P(ShadowVolume) shadow = 0;

			P(TriangleList) silhuette = readTriangleList( in, VertexFormat() );
			P(TriangleList) volume = readTriangleList( in, VertexFormat() );
			if ( ver == 0x120 )
				throw IOException( Format("Scene file format of static shadows is old, please re-export {0}", this->name) );
			if ( ver == 0x121 )
				throw IOException( Format("Scene file format of static shadows is old, please re-export {0}", this->name) );
			Vector4 plane = readVector4( in );
			
			String type;
			long subend;
			in->beginChunk( &type, &subend );
			if ( type == "dir" )
			{
				Vector3 lightWorld = readVector3( in );
				shadow = new ShadowVolume( silhuette, volume, plane, lightWorld );
			}
			else
			{
				throw IOException( Format("Invalid static shadow file format in scene {0}", this->name) );
			}
			in->endChunk( subend );

			mesh->addPrimitive( shadow );
			return true;
		}
		else if ( name == "dynamicshadow" )
		{
			P(DynamicShadow) dynamicShadow = new DynamicShadow;
			dynamicShadow->model = in->readString();

			String type;
			long subend;
			in->beginChunk( &type, &subend );
			if ( type == "dir" )
			{
				dynamicShadow->light = readVector3( in );
				dynamicShadow->length = in->readFloat();
			}
			in->endChunk( subend );

			item->dynamicShadow = dynamicShadow;
			return true;
		}
		return false;
	}

	bool readLightChunk( const String& name, ChunkInputStream* in, long end, Item* item, Light* light )
	{
		if ( readNodeChunk(name, in, end, item, light) )
			return true;

		if ( name == "color" )
		{
			Colorf c = readColorf( in );
			light->setDiffuseColor( c );
			light->setSpecularColor( c );
			return true;
		}
		else if ( name == "intensity" )
		{
			light->setIntensity( in->readFloat() );
			return true;
		}
		return false;
	}

	bool readPointLightChunk( const String& name, ChunkInputStream* in, long end, Item* item, PointLight* light )
	{
		if ( readLightChunk(name, in, end, item, light) )
			return true;

		if ( name == "range" )
		{
			light->setRange( in->readFloat() );
			return true;
		}
		else if ( name == "atten" )
		{
			float a[3];
			for ( int k = 0 ; k < 3 ; ++k )
				a[k] = in->readFloat();
			light->setAttenuation( a[0], a[1], a[2] );
			return true;
		}
		return false;
	}

	bool readSpotLightChunk( const String& name, ChunkInputStream* in, long end, Item* item, SpotLight* light )
	{
		if ( readLightChunk(name, in, end, item, light) )
			return true;

		if ( name == "range" )
		{
			light->setRange( in->readFloat() );
			return true;
		}
		else if ( name == "atten" )
		{
			float a[3];
			for ( int k = 0 ; k < 3 ; ++k )
				a[k] = in->readFloat();
			light->setAttenuation( a[0], a[1], a[2] );
			return true;
		}
		else if ( name == "innercone" )
		{
			light->setInnerCone( in->readFloat() );
			return true;
		}
		else if ( name == "outercone" )
		{
			light->setOuterCone( in->readFloat() );
			return true;
		}
		return false;
	}

	bool readCameraChunk( const String& name, ChunkInputStream* in, long end, Item* item, Camera* camera )
	{
		if ( readNodeChunk(name, in, end, item, camera) )
			return true;

		if ( name == "fov" )
		{
			float fov = in->readFloat();
			camera->setHorizontalFov( fov );
		}
		return true;
	}

	bool readDummyChunk( const String& name, ChunkInputStream* in, long end, Item* item, Dummy* dummy )
	{
		if ( readNodeChunk(name, in, end, item, dummy) )
			return true;

		if ( name == "box" )
		{
			Vector3 boxMin, boxMax;
			for ( int i = 0 ; i < 3 ; ++i )
				boxMin[i] = in->readFloat();
			for ( int i = 0 ; i < 3 ; ++i )
				boxMax[i] = in->readFloat();

			dummy->setBox( boxMin, boxMax );
		}
		return true;
	}

	void readNode( ChunkInputStream* in, long end )
	{
		String str;
		long subend;

		Item* item = addItem();
		Node* node;
		item->node = node = new Node;

		for ( ; in->size() < end ; in->endChunk(subend) )
		{
			in->beginChunk( &str, &subend );
			readNodeChunk( str, in, subend, item, node );
		}
	}

	void readMesh( ChunkInputStream* in, long end )
	{
		String str;
		long subend;

		Item* item = addItem();
		Mesh* node;
		item->node = node = new Mesh;

		for ( ; in->size() < end ; in->endChunk(subend) )
		{
			in->beginChunk( &str, &subend );
			readMeshChunk( str, in, subend, item, node );
		}
	}

	void readPointLight( ChunkInputStream* in, long end )
	{
		String str;
		long subend;

		Item* item = addItem();
		PointLight* node;
		item->node = node = new PointLight;

		for ( ; in->size() < end ; in->endChunk(subend) )
		{
			in->beginChunk( &str, &subend );
			readPointLightChunk( str, in, subend, item, node );
		}
	}

	void readSpotLight( ChunkInputStream* in, long end )
	{
		String str;
		long subend;

		Item* item = addItem();
		SpotLight* node;
		item->node = node = new SpotLight;

		for ( ; in->size() < end ; in->endChunk(subend) )
		{
			in->beginChunk( &str, &subend );
			readSpotLightChunk( str, in, subend, item, node );
		}
	}

	void readDirectLight( ChunkInputStream* in, long end )
	{
		String str;
		long subend;

		Item* item = addItem();
		DirectLight* node;
		item->node = node = new DirectLight;

		for ( ; in->size() < end ; in->endChunk(subend) )
		{
			in->beginChunk( &str, &subend );
			readLightChunk( str, in, subend, item, node );
		}
	}

	void readAmbientLight( ChunkInputStream* in, long end )
	{
		String str;
		long subend;

		Item* item = addItem();
		Node* node;
		item->node = node = new Node;

		for ( ; in->size() < end ; in->endChunk(subend) )
		{
			in->beginChunk( &str, &subend );
			readNodeChunk( str, in, subend, item, node );
		}
	}

	void readCamera( ChunkInputStream* in, long end )
	{
		String str;
		long subend;

		Item* item = addItem();
		Camera* node;
		item->node = node = new Camera;

		for ( ; in->size() < end ; in->endChunk(subend) )
		{
			in->beginChunk( &str, &subend );
			readCameraChunk( str, in, subend, item, node );
		}
	}

	void readDummy( ChunkInputStream* in, long end )
	{
		String str;
		long subend;

		Item* item = addItem();
		Dummy* node;
		item->node = node = new Dummy;

		for ( ; in->size() < end ; in->endChunk(subend) )
		{
			in->beginChunk( &str, &subend );
			readDummyChunk( str, in, subend, item, node );
		}
	}

	void readLOD( ChunkInputStream* in, long end )
	{
		String str;
		long subend;

		Item* item = addItem();
		LOD* node;
		item->node = node = new LOD;

		for ( ; in->size() < end ; in->endChunk(subend) )
		{
			in->beginChunk( &str, &subend );
			readNodeChunk( str, in, subend, item, node );
		}
	}

	void readEnvironment( ChunkInputStream* in, long end )
	{
		String str;
		long subend;

		for ( ; in->size() < end ; in->endChunk(subend) )
		{
			in->beginChunk( &str, &subend );

			if ( str == "ambient" )
				scene->setAmbientColor( Color( readColorf(in) ) );
		}
	}

private:
	SceneFileImpl( const SceneFileImpl& );
	SceneFileImpl& operator=( const SceneFileImpl& );
};

//-----------------------------------------------------------------------------

SceneFile::SceneFile( const lang::String& name, ModelFileCache* modelcache,
	InputStreamArchive* zip, int loadFlags )
{
	m_this = new SceneFileImpl( name, modelcache, zip, loadFlags );
}

sg::Node* SceneFile::scene() const
{
	return m_this->scene;
}

sg::Camera* SceneFile::camera() const
{
	return m_this->camera;
}

const lang::String& SceneFile::name() const
{
	return m_this->name;
}

SceneFile::~SceneFile()
{
}


} // sgu
