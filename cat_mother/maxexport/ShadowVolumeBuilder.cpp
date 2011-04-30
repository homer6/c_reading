#include "StdAfx.h"
#include "SgMesh.h"
#include "GmModel.h"
#include "SgLight.h"
#include "ShadowVolumeBuilder.h"
#include "ProgressInterface.h"
#include "BSPTreeBuilderThread.h"
#include "BSPUtil.h"
#include <mb/Polygon.h>
#include <mb/Vertex.h>
#include <dev/Profile.h>
#include <bsp/BSPCollisionUtil.h>
#include <bsp/BSPTreeBuilder.h>
#include <lang/Debug.h>
#include <lang/Float.h>
#include <lang/System.h>
#include <lang/Exception.h>
#include <lang/Format.h>
#include <math/Matrix4x4.h>
#include <math/OBBoxBuilder.h>
#include <math/Intersection.h>
#include <util/Hashtable.h>

#include <io/FileOutputStream.h>
#include <pix/Image.h>
#include <pix/SurfaceFormat.h>

//-----------------------------------------------------------------------------

#define MIN_SHADOW_LENGTH 0.01f
#define MAX_SHADOW_LENGTH 1000.f
#define SHADOW_LENGTH_EXTRA 0.10f

#define MIN_SHADOW_SAMPLE_DISTANCE 1.f
#define MAX_SHADOW_SAMPLE_DISTANCE 1000.f

//-----------------------------------------------------------------------------

using namespace bsp;
using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

/** Edge structure used in shadow volume creation. */
class ShadowEdge
{
public:
	ShadowEdge()											{}
	ShadowEdge( const Vector3& v0, const Vector3& v1 )		: m_v0(v0), m_v1(v1) {m_hash = *reinterpret_cast<int*>(&m_v0.x) + *reinterpret_cast<int*>(&m_v0.y) + *reinterpret_cast<int*>(&m_v0.z) + *reinterpret_cast<int*>(&m_v1.x) + *reinterpret_cast<int*>(&m_v1.y) + *reinterpret_cast<int*>(&m_v1.z);}

	const Vector3&	v0() const								{return m_v0;}
	const Vector3&	v1() const								{return m_v1;}
	int				hashCode() const						{return m_hash;}

	bool	operator==( const ShadowEdge& other ) const 	{return m_v0 == other.m_v0 && m_v1 == other.m_v1;}
	bool	operator!=( const ShadowEdge& other ) const 	{return m_v0 != other.m_v0 || m_v1 != other.m_v1;}

private:
	Vector3	m_v0;
	Vector3	m_v1;
	int		m_hash;
};

//-----------------------------------------------------------------------------

/**
 * Creates a shadow volume from the model.
 * @param model Source geometry for the shadow volume.
 * @param dir Direction of the shadow volume in geometry space.
 * @param volume [out] Receives triangles generating the volume.
 */
static void createVolume( 
	mb::MeshBuilder* model, const Vector3& dir, 
	Vector<Vector3>& volume )
{
	require( dir.length() > Float::MIN_VALUE && dir.finite() );

	// reset old containers
	volume.clear();

	// compute lit/unlit status of the polygons 
	// add polygons generating the volume
	Vector3				verts[6];
	int					faceindex = 0;

	for ( int i = 0 ; i < model->polygons() ; ++i )
	{
		// get face vertices
		mb::Polygon* poly = model->getPolygon(i);
		require( poly->vertices() == 3 );
		for ( int j = 0 ; j < poly->vertices() ; ++j )
		{
			poly->getVertex(j)->getPosition( &verts[j].x, &verts[j].y, &verts[j].z );
			require( verts[j].finite() );
		}

		// polygon lit/unlit or invalid?
		bool lit = false;
		bool valid = true;
		Vector3 edge1 = verts[1] - verts[0];
		Vector3 edge2 = verts[2] - verts[0];
		Vector3 normal = edge1.cross( edge2 );
		float n2 = normal.lengthSquared();
		if ( n2 > 1e-8f )
			lit = normal.dot(dir) < 0.f;
		else
			valid = false;

		// list polygons generating the volume
		if ( !lit && valid )
		{
			for ( int j = 0 ; j < poly->vertices() ; ++j )
			{
				require( verts[j].finite() );
				volume.add( verts[j] );
			}
		}

		++faceindex;
	}
}

/**
 * Creates a shadow silhuette from the volume.
 * which cannot be shadow boundary.
 * @param volume Triangles generating the volume.
 * @param volumeLengths Length from the each volume triangle vertex to light direction.
 * @param dir Direction of the shadow volume in geometry space.
 * @param silhuette [out] Receives silhuette triangles of the volume.
 */
static void createSilhuette( 
	const Vector<Vector3>& volume, const Vector<float>& volumeLengths,
	const Vector3& dir, Vector<Vector3>& silhuette )
{
	require( dir.length() > Float::MIN_VALUE && dir.finite() );
	require( volumeLengths.size() == volume.size() );

	// get polygon adjacency
	Vector3						verts[6];
	int							faceindex = 0;
	Hashtable<ShadowEdge,int>	edges( volume.size()/3, 0.75f, -1, Hash<ShadowEdge>(), Equal<ShadowEdge>(), Allocator< HashtablePair<ShadowEdge,int> >(__FILE__,__LINE__) );

	for ( int i = 0 ; i < volume.size() ; i += 3 )
	{
		// get volume source vertices
		for ( int k = 0 ; k < 3 ; ++k )
			verts[k] = volume[i+k];

		// add edges to the hashtable
		int k = 2;
		for ( int j = 0 ; j < 3 ; k = j++ )
		{
			ShadowEdge edge( verts[k], verts[j] );
			edges[edge] = faceindex;
		}

		++faceindex;
	}

	// collect shadow edges
	faceindex = 0;
	silhuette.clear();
	for ( int i = 0 ; i < volume.size() ; i += 3 )
	{
		int k = 2;
		for ( int j = 0 ; j < 3 ; k = j++ )
		{
			ShadowEdge edge( volume[i+j], volume[i+k] );
			int poly = edges[edge];
			
			if ( -1 == poly && 
				(volumeLengths[i+k] >= MIN_SHADOW_LENGTH ||
				volumeLengths[i+j] >= MIN_SHADOW_LENGTH) )
			{
				const Vector3&	v0 = volume[i+k];
				const Vector3&	v1 = volume[i+j];
				const Vector3	v2 = v1 + dir * volumeLengths[i+j];
				const Vector3	v3 = v0 + dir * volumeLengths[i+k];
				require( v0.finite() );
				require( v1.finite() );
				require( v2.finite() );
				require( v3.finite() );

				silhuette.add( v0 );
				silhuette.add( v1 );
				silhuette.add( v2 );
				silhuette.add( v0 );
				silhuette.add( v2 );
				silhuette.add( v3 );
			}
		}
		++faceindex;
	}
}

//-----------------------------------------------------------------------------

ShadowVolumeBuilder::ShadowVolumeBuilder( SgMesh* mesh, SgLight* keylight,
	 bool forceDynamic ) :
	m_silhuette( Allocator<Vector3>(__FILE__,__LINE__) ),
	m_volume( Allocator<Vector3>(__FILE__,__LINE__) ),
	m_volumeLengths( Allocator<float>(__FILE__,__LINE__) ),
	m_silhuetteDirty( true )
{
	require( mesh->model );

	m_modelName = mesh->model->filename;
	m_shadowLength = MAX_SHADOW_LENGTH;
	m_dynamicShadow = (mesh->isAnimated() || forceDynamic);
	m_capPlaneNormal = Vector3(0,0,0);
	m_capPlanePoint = Vector3(0,0,0);
	m_worldToModel = mesh->getWorldTransform(0).inverse();

	// light in world/model space
	m_lightWorld = Vector3(0,-1,0);
	if ( keylight )
		m_lightWorld = keylight->getWorldTransform(0).rotation().getColumn(2);
	if ( m_lightWorld.length() < Float::MIN_VALUE )
		throw Exception( Format("Keylight cannot have zero length.") );
	m_lightWorld = m_lightWorld.normalize();
	m_lightModel = m_worldToModel.rotation() * m_lightWorld;
	m_lightModel = m_lightModel.normalize();

	if ( !m_dynamicShadow )
	{
		// static shadow
		Debug::println( "Creating static shadow for {0}", mesh->model->name );

		createVolume( mesh->model, m_lightModel, m_volume );

		m_volumeLengths.clear();
		m_volumeLengths.setSize( m_volume.size(), m_shadowLength );
		setSilhuetteDirty();
	}
	else
	{
		// dynamic shadow
		Debug::println( "Created dynamic shadow for {0} (len={1})", mesh->model->name, m_shadowLength );
	}
}

ShadowVolumeBuilder::~ShadowVolumeBuilder()
{
}

const Vector<Vector3>& ShadowVolumeBuilder::silhuette() const
{
	if ( m_silhuetteDirty )
		const_cast<ShadowVolumeBuilder*>(this)->refreshSilhuette();
	return m_silhuette;
}

const Vector<Vector3>& ShadowVolumeBuilder::volume() const
{
	return m_volume;
}

const Vector<float>& ShadowVolumeBuilder::volumeLengths() const
{
	return m_volumeLengths;
}

Vector<Vector3>& ShadowVolumeBuilder::silhuette() 
{
	if ( m_silhuetteDirty )
		refreshSilhuette();
	return m_silhuette;
}

Vector<Vector3>& ShadowVolumeBuilder::volume() 
{
	return m_volume;
}

Vector<float>& ShadowVolumeBuilder::volumeLengths() 
{
	return m_volumeLengths;
}

Vector4 ShadowVolumeBuilder::capPlane() const
{
	require( m_capPlaneNormal != Vector3(0,0,0) );
	Vector4 plane( m_capPlaneNormal.x, m_capPlaneNormal.y, m_capPlaneNormal.z, -m_capPlaneNormal.dot(m_capPlanePoint) );
	return plane;
}

void ShadowVolumeBuilder::projectVolumes()
{
	if ( m_volumeLengths.size() < 3 )
		return;
	
	setSilhuetteDirty();

	// compute shadow volume end cap bounding box
	/*OBBoxBuilder boxb;
	while ( boxb.nextPass() )
	{
		for ( int i = 0 ; i < m_volumeLengths.size() ; ++i )
		{
			Vector3 v = m_volume[i] + m_lightModel * m_volumeLengths[i];
			boxb.addPoints( &v, 1 );
		}	
	}
	OBBox box = boxb.box();

	// compute end cap plane from obbox
	m_capPlaneNormal = box.rotation().getColumn(2);
	if ( m_capPlaneNormal.dot(m_lightModel) > 0.f )
		m_capPlaneNormal = -m_capPlaneNormal;
	m_capPlanePoint = box.translation() - m_capPlaneNormal * box.dimensions()[2];*/
	
	// use ground plane as cap plane
	m_capPlaneNormal = m_worldToModel.rotate( Vector3(0,1,0) );
	if ( m_capPlaneNormal.dot(m_lightModel) > 0.f )
		m_capPlaneNormal = -m_capPlaneNormal;
	m_capPlanePoint = Vector3(0,0,0);

	// adjust end cap plane position
	float minDist = FLT_MAX;
	for ( int i = 0 ; i < m_volumeLengths.size() ; ++i )
	{
		require( m_volumeLengths[i] >= 0.f );
		Vector3 v = m_volume[i] + m_lightModel * m_volumeLengths[i];
		float dist = m_capPlaneNormal.dot( v );
		if ( dist < minDist )
			minDist = dist;
	}
	if ( minDist == FLT_MAX )
	{
		Debug::println( "    shadow projection minDist == FLT_MAX, disabling shadow" );
		m_volume.clear();
		m_volumeLengths.clear();
		return;
	}
	m_capPlanePoint = m_capPlaneNormal * minDist;
	Debug::println( "    shadow cap plane point = {0} {1} {2}", m_capPlanePoint.x, m_capPlanePoint.y, m_capPlanePoint.z );
	Debug::println( "    shadow cap plane normal = {0} {1} {2}", m_capPlaneNormal.x, m_capPlaneNormal.y, m_capPlaneNormal.z );

	// project volume to end cap plane and store lengths
	Matrix4x4 proj;
	proj.setDirectPlaneProjection( m_lightModel, m_capPlaneNormal, m_capPlanePoint );
	bool errOnce = true;
	for ( int i = 0 ; i < m_volumeLengths.size() ; ++i )
	{
		Vector3 v = m_volume[i];
		
		Vector4 p0( v.x, v.y, v.z, 1.f );
		Vector4 p1;
		proj.transform( p0, &p1 );
		p1 *= 1.f / p1.w;
		Vector3 v1( p1.x, p1.y, p1.z );
		require( v.finite() );
		require( v1.finite() );

		float len = (v1-v).dot( m_lightModel );
		if ( len < 0.f )
		{
			if ( errOnce )
				Debug::printlnError( "Projected shadow length is {0}", len );
			errOnce = false;
			len = 0.f;
		}
		//require( _finite(len) );
		//require( len >= 0.f );
		m_volumeLengths[i] = len;
	}
}

void ShadowVolumeBuilder::refreshSilhuette()
{
	createSilhuette( m_volume, m_volumeLengths, m_lightModel, m_silhuette );
	m_silhuetteDirty = false;
}

bool ShadowVolumeBuilder::dynamicShadow() const
{
	return m_dynamicShadow;
}

const Vector3& ShadowVolumeBuilder::lightModel() const
{
	return m_lightModel;
}

const Vector3& ShadowVolumeBuilder::lightWorld() const
{
	return m_lightWorld;
}

const String& ShadowVolumeBuilder::modelName() const
{
	return m_modelName;
}

float ShadowVolumeBuilder::shadowLength() const
{
	return m_shadowLength;
}

void ShadowVolumeBuilder::setSilhuetteDirty()
{
	m_silhuetteDirty = true;
}

void ShadowVolumeBuilder::optimize( Vector<P(SgMesh)>& meshes, 
	ProgressInterface* progress,
	float maxShadowSampleDistance, const 
	const Vector3& lightWorld )
{
	// limit shadow sampling
	if ( maxShadowSampleDistance < MIN_SHADOW_SAMPLE_DISTANCE )
		maxShadowSampleDistance = MIN_SHADOW_SAMPLE_DISTANCE;
	else if ( maxShadowSampleDistance > MAX_SHADOW_SAMPLE_DISTANCE )
		maxShadowSampleDistance = MAX_SHADOW_SAMPLE_DISTANCE;

	// compute scene BSP trees
	BSPTreeBuilder builderLit;
	BSPTreeBuilder builderUnlit;
	Vector<Vector3> v( Allocator<Vector3>(__FILE__,__LINE__) );
	for ( int i = 0 ; i < meshes.size() ; ++i )
	{
		SgMesh* mesh = meshes[i];
		require( mesh );
		//require( -1 != mesh->lodNum || mesh->lodID == "" );

		if ( (!mesh->shadow || !mesh->shadow->dynamicShadow()) &&
			(mesh->recvShadows || mesh->castShadows) )
		{
			Matrix4x4 tm = mesh->getWorldTransform(0);

			// find minimum detail level to
			// avoid adding many models to BSP trees
			int selectedLOD = i;
			if ( -1 != mesh->lodNum )
			{
				for ( int k = 0 ; k < meshes.size() ; ++k )
				{
					SgMesh* other = meshes[k];
					if ( other->lodNum == mesh->lodNum )
					{
						// mesh and other belong to the same LOD group
						if ( other->model->polygons() <
							meshes[selectedLOD]->model->polygons() )
						{
							selectedLOD = k;
						}
					}
				}
			}

			// add mesh polygons to the BSP tree builders
			if ( i == selectedLOD )
			{
				for ( int j = 0 ; j < mesh->model->polygons() ; ++j )
				{
					const mb::Polygon*	poly	= mesh->model->getPolygon( j );
					const int			n		= poly->vertices();

					// list unique points of the polygon
					v.clear();
					Vector3 p, p0;
					for ( int k = 0 ; k < n ; ++k )
					{
						poly->getVertex(k)->getPosition( &p.x, &p.y, &p.z );
						if ( 0 == k || (p0-p).length() > Float::MIN_VALUE )
						{
							v.add( tm.transform(p) );
							p0 = p;
						}
					}

					// add valid polygons to BSP trees
					if ( v.size() >= 3 )
					{
						// BSP polygon id is used here to avoid
						// invalid shadow optimizations in a LOD group
						// (see below where the shadow length is 
						// cut down with BSP intersections)
						int id = mesh->lodNum;
						Vector3 normal = (v[1]-v[0]).cross( v[2]-v[0] );

						if ( normal.dot(lightWorld) < 0.f && mesh->recvShadows )
						{
							builderLit.addPolygon( v.begin(), v.size(), id, -1 );
						}
						else if ( mesh->castShadows )
						{
							builderUnlit.addPolygon( v.begin(), v.size(), id, -1 );
						}
					}
				}
			} // if ( i == maxLODIndex )
		}
	}

	//pix::Image img( 1000, 500, pix::SurfaceFormat::SURFACE_R8G8B8 );
	long timeBeforeBSP = System::currentTimeMillis();
	progress->setText( Format("Preparing to optimize shadows... ({0} polygons)", builderLit.polygons()+builderUnlit.polygons()).format() );
	P(BSPTreeBuilderThread) workerLit = new BSPTreeBuilderThread( builderLit );
	P(BSPTreeBuilderThread) workerUnlit = new BSPTreeBuilderThread( builderUnlit );
	workerLit->start();
	workerUnlit->start();
	while ( !workerLit->done() || !workerUnlit->done() )
	{
		// get progress
		float workerLitProgress = workerLit->progress();
		float workerUnlitProgress = workerUnlit->progress();
		float bspBuildProgress = .5f * (workerLitProgress + workerUnlitProgress);

		progress->setProgress( bspBuildProgress );
		Thread::sleep( 500 );
		
		// plot progress
		static int plot = 0;
		plot++;
		if ( plot < 0 )
			plot = 0;
		//img.setPixel( (plot/2)%img.width(), int((1.f-bspBuildProgress)*(img.height()-1))%img.height(), 0xFF );
	}
	workerLit->join();
	workerUnlit->join();
	P(BSPNode) bspLit = workerLit->root();
	P(BSPNode) bspUnlit = workerUnlit->root();
	/*try {io::FileOutputStream imgout( "f:/tmp/progress.bmp" );
	img.save( &imgout, "f:/tmp/progress.bmp" );
	imgout.close(); } catch ( ... ) {}*/

	// optimize shadow lengths
	for ( int i = 0 ; i < meshes.size() ; ++i )
	{
		SgMesh* mesh = meshes[i];
		require( mesh );
		progress->setText( Format("Optimizing shadows... ({0})", mesh->name).format() );

		int counts[3] = {0,0,0};

		ShadowVolumeBuilder* shadow = mesh->shadow;
		if ( shadow && !shadow->dynamicShadow() )
		{
			const Vector<Vector3>&	silhuette		= shadow->silhuette();
			Vector<Vector3>&		volume			= shadow->volume();
			Vector<float>&			volumeLengths	= shadow->volumeLengths();
			const Vector3&			lightWorld		= shadow->m_lightWorld;
			const Vector3&			lightModel		= shadow->m_lightModel;

			// default (max) shadow length
			volumeLengths.clear();
			volumeLengths.setSize( volume.size(), MAX_SHADOW_LENGTH );

			// shadow->world transform
			Matrix4x4 thisToWorld = mesh->getWorldTransform(0);

			// optimize shadow against scene BSPs
			Vector3 shadowDelta = lightWorld * MAX_SHADOW_LENGTH;
			Vector3 shadowOffset = lightWorld * MIN_SHADOW_LENGTH;
			for ( int j = 0 ; j < volume.size() ; j += 3 )
			{
				if ( j % 30 == 0 )
					progress->setProgress( (float)(j+1) / (float)volume.size() );

				bool brokenVolume = false;
				bool nonZeroLength = false;
				bool alreadyInShadow = true;
				float validLen = 0.f;

				// find longest intersecting shadow
				Vector3 avgp = thisToWorld.transform( (volume[j]+volume[j+1]+volume[j+2])*.3333f );
				for ( int k = 0 ; k < 3 ; k++ )
				{
					Vector3 p = thisToWorld.transform( volume[j+k] );
					float t;

					// find needed length for the shadow volume
					if ( BSPCollisionUtil::findLastLineIntersection(bspLit, p+shadowOffset, shadowDelta, -1, &t) )
					{
						float len = t * MAX_SHADOW_LENGTH;
						volumeLengths[j+k] = len + SHADOW_LENGTH_EXTRA;
						if ( len >= MIN_SHADOW_LENGTH )
						{
							nonZeroLength = true;
							validLen = len;
						}
					}
					else
					{
						brokenVolume = true;
						volumeLengths[j+k] = 0.f;
					}

					// is the point already in shadow?
					if ( alreadyInShadow )
					{
						// shift polygon point towards polygon center so that
						// for example 2nd triangle of cube face doesn't shadow the other triangle
						Vector3 shiftedp = (p-avgp)*.95f + avgp;
						const BSPPolygon* cpoly = 0;
						BSPCollisionUtil::findLastLineIntersection( bspUnlit, shiftedp-shadowOffset, -shadowDelta, -1, 0, &cpoly );
						if ( !cpoly || cpoly->id() != -1 && cpoly->id() == mesh->lodNum )
							alreadyInShadow = false;
					}
				}

				// fix partial but otherwise valid volumes
				if ( brokenVolume && nonZeroLength && !alreadyInShadow )
				{
					counts[0] += 1;
					for ( int k = 0 ; k < 3 ; ++k )
						volumeLengths[j+k] = MIN_SHADOW_LENGTH;
				}

				// remove degenerate shadows
				if ( !nonZeroLength || alreadyInShadow )
				{
					if ( !nonZeroLength ) 
						counts[1] += 1;
					if ( alreadyInShadow ) 
						counts[2] += 1;

					volume.remove( j, j+3 );
					volumeLengths.remove( j, j+3 );
					j -= 3;
				}
			}
			Debug::println( "Mesh {0} shadow polygons optimized: brokenVolume={1} / zeroLength={2} / alreadyInShadow={3}", mesh->name, counts[0], counts[1], counts[2] );

			// disable shadow if we ended up with null shadow volume,
			// otherwise refresh the silhuette
			if ( volume.size() < 3 )
			{
				mesh->shadow = 0;
				Debug::println( "Mesh {0} shadow disabled", mesh->name );
			}
			else
			{
				shadow->setSilhuetteDirty();
			}
		}
	}
}

