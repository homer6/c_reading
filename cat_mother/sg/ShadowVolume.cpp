#include "ShadowVolume.h"
#include <sg/Model.h>
#include <sg/PolygonAdjacency.h>
#include <sg/TriangleList.h>
#include <sg/VertexLock.h>
#include <sg/ViewFrustum.h>
#include <sg/Context.h>
#include <sg/DirectLight.h>
#include <sg/VertexFormat.h>
#include <sg/ShadowShader.h>
#include <sg/VertexAndIndexLock.h>
#include <gd/GraphicsDevice.h>
#include <dev/Profile.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <util/Vector.h>
#include <math/Intersection.h>
#include <math/OBBox.h>
#include <algorithm>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace dev;
using namespace lang;
using namespace math;
using namespace util;

//-----------------------------------------------------------------------------

namespace sg
{


static int		s_volumeCapPolygons			= 0;
static int		s_renderedShadows			= 0;
static int		s_renderedShadowTriangles	= 0;

static Vector<bool>		s_polylit( Allocator<bool>(__FILE__,__LINE__) );
static Vector<bool>		s_polyvalid( Allocator<bool>(__FILE__,__LINE__) );
static Vector<Vector3>	s_vertices( Allocator<Vector3>(__FILE__,__LINE__) );
static Vector<Vector3>	s_triangles( Allocator<Vector3>(__FILE__,__LINE__) );
static int				s_shadowVolumes = 0;

//-----------------------------------------------------------------------------

/**
 * Adds a convex polygon to triangle list.
 * The polygon has vertices defined in clockwise order.
 * @param v Polygon points in clockwise order.
 * @param n Number of points in the polygon.
 * @param geom Triangle list to receive triangulated polygon.
 * @param firstVertex The first vertex of the polygon in triangle list.
 * @return Number of vertices added to the triangle list.
 */
static inline int addPolygonToTriangleListCW( const Vector3* v, int n,
	TriangleList* geom, int firstVertex )
{
	//Profile pr( "addpoly" );

	int newVerts = 0;
	if ( n > 2 )
	{
		// allocate more space to the output buffer if needed
		newVerts = (unsigned)(n-2) * 3U;
		if ( firstVertex+newVerts > geom->maxVertices() )
		{
			const int moreTriangles = 100;
			geom->setMaxVertices( firstVertex + newVerts + moreTriangles*3 );
		}

		// store triangles
		Vector3 buf[3];
		int j = 2;
		for ( ; j < n ; ++j )
		{
			buf[0] = v[0];
			buf[1] = v[j-1];
			buf[2] = v[j];

			geom->setVertexPositions( firstVertex, buf, 3 );
			firstVertex += 3;
		}
	}
	assert( (firstVertex+newVerts) % 3 == 0 );
	return newVerts;
}

/**
 * Adds a convex polygon to triangle list.
 * The polygon has vertices defined in counter-clockwise order.
 * @param v Polygon points in clockwise order.
 * @param n Number of points in the polygon.
 * @param geom Triangle list to receive triangulated polygon.
 * @param firstVertex The first vertex of the polygon in triangle list.
 * @return Number of vertices added to the triangle list.
 */
static inline int addPolygonToTriangleListCCW( const Vector3* v, int n,
	TriangleList* geom, int firstVertex )
{
	//Profile pr( "addpoly" );

	int newVerts = 0;
	if ( n > 2 )
	{
		// allocate more space to the output buffer if needed
		newVerts = (unsigned)(n-2) * 3U;
		if ( firstVertex+newVerts > geom->maxVertices() )
		{
			const int moreTriangles = 100;
			geom->setMaxVertices( firstVertex + newVerts + moreTriangles*3 );
		}

		// store triangles
		Vector3 buf[3];
		int j = 2;
		for ( ; j < n ; ++j )
		{
			buf[0] = v[0];
			buf[1] = v[j];
			buf[2] = v[j-1];

			geom->setVertexPositions( firstVertex, buf, 3 );
			firstVertex += 3;
		}
	}
	assert( (firstVertex+newVerts) % 3 == 0 );
	return newVerts;
}

/**
 * Creates a shadow volume from the model. Detects interior edges,
 * which cannot be shadow boundary. Produces silhuette and
 * list of triangles which face away from the light.
 * 
 * @param model Source geometry for the shadow volume.
 * @param dirWorld Direction and length of the shadow volume in world space.
 * @param worldTM Model to world transforms.
 * @param worldTMCount Number of model to world transforms.
 * @param oldSilhuette Old shadow silhuette if any.
 * @param oldVolume Old shadow volume generator if any.
 * @param shadowSilhuette [out] Receives new shadow silhuette if any.
 * @param shadowVolume [out] Receives new shadow volume generator if any.
 * @param capPlaneNormal [out] Receives shadow end cap plane normal.
 * @param capPlanePoint [out] Receives shadow end cap plane point.
 */
static void createShadowVolume( 
	Model* model, const Vector3& dirWorld, 
	const Matrix4x4* worldTM, int worldTMCount,
	P(TriangleList) oldSilhuette, P(TriangleList) oldVolume,
	P(TriangleList)* shadowSilhuette, P(TriangleList)* shadowVolume,
	Vector3* capPlaneNormal, Vector3* capPlanePoint )
{
	assert( model );
	assert( worldTMCount > 0 ); worldTMCount = worldTMCount;

	const Matrix4x4				inverseWorldTM = worldTM[0].inverse();
	const int					indices = model->indices();
	VertexAndIndexLock<Model>	lockModel( model, Model::LOCK_READ );
	const PolygonAdjacency&		modelAdj = model->getPolygonAdjacency( 0.001f );
	const Vector3				dirModel = inverseWorldTM.rotate( dirWorld );
	const Vector3				unitDirModel = dirModel.normalize();
	// volume cap plane distance along shadow direction
	float						shadowEndPlaneDist = -Float::MAX_VALUE;

	// poly state buffers (lit/valid)
	s_polylit.setSize( indices/3 );
	s_polyvalid.setSize( indices/3 );

	// reuse old silhuette if possible
	P(TriangleList) silhuette = oldSilhuette;
	if ( !silhuette )
	{
		int tri = indices/2;
		tri -= tri % 3;
		silhuette = new TriangleList( tri, VertexFormat() );
	}
	int numVerts = 0;

	// reuse old volume generator mesh if possible
	P(TriangleList) volume = oldVolume;
	if ( !volume )
		volume = new TriangleList( (indices/3/2+1)*3, VertexFormat() );
	int volumeVerts = 0;

	// get triangles in geometry space
	s_triangles.setSize( indices );
	s_vertices.setSize( model->vertices() );
	int	face[3];
	Vector3 v0, v1;
	if ( 0 == model->vertexFormat().weights() )
	{
		model->getVertexPositions( 0, s_vertices.begin(), s_vertices.size() );
	}
	else
	{
		//Profile pr( "ShadowVolume.skinning" );
		bool mostSignifigantBoneOnly = false;
		model->getTransformedVertexPositions( worldTM, worldTMCount, inverseWorldTM, s_vertices.begin(), s_vertices.size(), mostSignifigantBoneOnly );
	}
	for ( int i = 0 ; i < indices ; i += 3 )
	{
		model->getIndices( i, face, 3 );
		for ( int j = 0 ; j < 3 ; ++j )
			s_triangles[i+j] = s_vertices[ face[j] ];
	}

	// compute lit/unlit status of the polygons 
	// add polygons generating the volume
	Vector3						v[6];
	int							adj[3];	
	int							faceindex = 0;
	VertexLock<TriangleList>	lockVolume( volume, TriangleList::LOCK_WRITE );

	for ( int i = 0 ; i < indices ; i += 3 )
	{
		const Vector3* verts = s_triangles.begin() + i;

		// polygon lit/unlit or invalid?
		Vector3 edge1 = verts[1] - verts[0];
		Vector3 edge2 = verts[2] - verts[0];
		Vector3 normal = edge1.cross( edge2 );
		float n2 = normal.lengthSquared();
		bool valid = n2 > 1e-10f;
		bool lit = normal.dot(dirModel) < 0.f;

		// set lit/validity status
		s_polylit[faceindex] = lit;
		s_polyvalid[faceindex] = valid;

		// list polygons generating the volume
		if ( valid )
		{
			if ( !lit )
				volumeVerts += addPolygonToTriangleListCW( verts, 3, volume, volumeVerts );
			//else
			//	volumeVerts += addPolygonToTriangleListCCW( verts, 3, volume, volumeVerts );

			// move shadow cap plane distance if needed
			for ( int k = 0 ; k < 3 ; ++k )
			{
				float dist = unitDirModel.dot( verts[k] + dirModel );
				if ( dist > shadowEndPlaneDist )
					shadowEndPlaneDist = dist;
			}
		}

		++faceindex;
	}
	volume->setVertices( volumeVerts );

	// collect shadow silhuette edges
	Vector3 shadowEndPlanePoint = unitDirModel * shadowEndPlaneDist;
	VertexLock<TriangleList> lockSilhuette( silhuette, TriangleList::LOCK_WRITE );
	faceindex = 0;
	for ( int i = 0 ; i < indices ; i += 3 )
	{
		if ( s_polyvalid[faceindex] )
		{
			// allocate more silhuette space if needed
			if ( numVerts+6*3 > silhuette->maxVertices() )
			{
				const int moreTriangles = 100;
				silhuette->setMaxVertices( silhuette->maxVertices() + moreTriangles*3 );
			}

			// add silhuette edges
			modelAdj.getAdjacent( faceindex, adj, 3 );
			const Vector3* verts = s_triangles.begin() + i;
			int k = 2;
			for ( int j = 0 ; j < 3 ; k = j++ )
			{
				int poly = adj[j];
				if ( -1 == poly || /*s_polylit[poly] != s_polylit[faceindex]*/ s_polylit[poly] && !s_polylit[faceindex] )
				{
					const Vector3& v0 = verts[k];
					const Vector3& v1 = verts[j];
					
					Vector3 v2 = v1 + dirModel;
					Vector3 v3 = v0 + dirModel;

					v2 += unitDirModel * ( (shadowEndPlanePoint-v2).dot(unitDirModel) );
					v3 += unitDirModel * ( (shadowEndPlanePoint-v3).dot(unitDirModel) );

					if ( !s_polylit[faceindex] )
					{
						v[0] = v0;
						v[1] = v1;
						v[2] = v2;
						v[3] = v0;
						v[4] = v2;
						v[5] = v3;
					}
					else
					{
						v[0] = v0;
						v[1] = v2;
						v[2] = v1;
						v[3] = v0;
						v[4] = v3;
						v[5] = v2;
					}
					
					silhuette->setVertexPositions( numVerts, v, 6 );
					numVerts += 6;
				}
			}
		}

		++faceindex;
	}
	silhuette->setVertices( numVerts );

	// return results
	*shadowSilhuette = silhuette;
	*shadowVolume = volume;
	*capPlanePoint = shadowEndPlanePoint;
	*capPlaneNormal = -unitDirModel;
}

//-----------------------------------------------------------------------------

ShadowVolume::ShadowVolume( Model* model, const Vector3& light, float shadowLength ) :
	m_light( light ),
	m_shadowLength( shadowLength ),
	m_shadowSilhuette( 0 ),
	m_shadowVolume( 0 ),
	m_dynamicModel( model ),
	m_polys( 0 ),
	m_capPlaneNormal( 0, 0, 0 ),
	m_capPlanePoint( 0, 0, 0 ),
	m_rot( 1.f ),
	m_viewOffset( 0.f )
{
	++s_shadowVolumes;
}

ShadowVolume::ShadowVolume( TriangleList* silhuette, TriangleList* volume,
	const Vector4& endCap, const Vector3& light ) :
	m_light( light ),
	m_shadowLength( 0.f ),
	m_shadowSilhuette( silhuette ),
	m_shadowVolume( volume ),
	m_dynamicModel( 0 ),
	m_polys( silhuette->vertices()/3 * 2 ),
	m_capPlaneNormal( endCap.x, endCap.y, endCap.z ),
	m_capPlanePoint( Vector3(endCap.x,endCap.y,endCap.z) * -endCap.w ),
	m_rot( 1.f ),
	m_viewOffset( 0.f )
{
	assert( m_light.finite() && m_light.length() > Float::MIN_VALUE );
	++s_shadowVolumes;
}

ShadowVolume::ShadowVolume( const ShadowVolume& other, int shareFlags ) :
	Primitive( other, shareFlags ),
	m_light( other.m_light ),
	m_shadowLength( other.m_shadowLength ),
	m_shadowSilhuette( other.m_shadowSilhuette ),
	m_shadowVolume( other.m_shadowVolume ),
	m_dynamicModel( other.m_dynamicModel ),
	m_polys( other.m_polys ),
	m_capPlaneNormal( other.m_capPlaneNormal ),
	m_capPlanePoint( other.m_capPlanePoint ),
	m_rot( other.m_rot ),
	m_viewOffset( other.m_viewOffset )
{
	++s_shadowVolumes;
}

ShadowVolume::~ShadowVolume()
{
	if ( --s_shadowVolumes == 0 )
	{
		s_polylit.clear();
		s_polylit.trimToSize();
		s_polyvalid.clear();
		s_polyvalid.trimToSize();
		s_vertices.clear();
		s_vertices.trimToSize();
		s_triangles.clear();
		s_triangles.trimToSize();
	}
}

Primitive* ShadowVolume::clone( int shareFlags ) const
{
	return new ShadowVolume( *this, shareFlags );
}

void ShadowVolume::destroy()
{
	Primitive::destroy();
}

void ShadowVolume::load()
{
	if ( m_shadowSilhuette )
		m_shadowSilhuette->load();
	if ( m_shadowVolume )
		m_shadowVolume->load();
}

void ShadowVolume::unload()
{
	if ( m_shadowSilhuette )
		m_shadowSilhuette->unload();
	if ( m_shadowVolume )
		m_shadowVolume->unload();
}

void ShadowVolume::setDynamicShadow( const math::Vector3& light, float shadowLength )
{
	assert( dynamicShadow() );

	m_light = light;
	m_shadowLength = shadowLength;
}

void ShadowVolume::draw()
{
	gd::GraphicsDevice* dev = Context::device();

	if ( dev->stencil() )
	{
		Profile pr( "ShadowVolume.draw" );

		// update animated model shadow
		if ( dynamicShadow() )
		{
			if ( m_shadowLength <= 0.f )
				return;

			createShadowVolume( m_dynamicModel,
				m_light*m_shadowLength,
				dev->worldTransforms(), dev->worldTransformCount(),
				m_shadowSilhuette, m_shadowVolume, 
				&m_shadowSilhuette, &m_shadowVolume,
				&m_capPlaneNormal, &m_capPlanePoint );
		}

		// render shadow volume
		if ( m_shadowSilhuette && m_shadowVolume )
		{
			m_polys = dev->renderedTriangles();

			// store original world tm
			Matrix4x4 worldtm;
			dev->getWorldTransform( &worldtm );

			Matrix4x4 viewtm;
			dev->getViewTransform( &viewtm );
			Matrix4x4 viewtm2 = viewtm;
			viewtm2.setTranslation( viewtm.translation() + Vector3(0,0,m_viewOffset) );
			dev->setViewTransform( viewtm2 );

			// set shadow volume shader
			ShadowShader* shadowShader = dynamic_cast<ShadowShader*>( shader() );
			assert( shadowShader );
			m_shadowVolume->setShader( shadowShader );
			m_shadowSilhuette->setShader( shadowShader );
			
			// render silhuette
			shadowShader->setFlip( false );
			m_shadowSilhuette->draw();

			// render start cap
			shadowShader->setFlip( true );
			m_shadowVolume->draw();

			// set up end cap projection
			Vector3 planePoint = worldtm.transform( m_capPlanePoint );
			Vector3 planeNormal = worldtm.rotate( m_capPlaneNormal );
			Matrix4x4 proj;
			proj.setDirectPlaneProjection( m_light, planeNormal, planePoint );

			// render end cap
			shadowShader->setFlip( false );
			dev->setWorldTransform( proj * worldtm );
			m_shadowVolume->draw();

			// restore transforms
			dev->setWorldTransform( worldtm );
			dev->setViewTransform( viewtm );

			// update statistics
			m_polys = dev->renderedTriangles() - m_polys;
			s_renderedShadowTriangles += m_polys;
			++s_renderedShadows;
		}
	}
}

float ShadowVolume::boundSphere() const
{
	if ( m_shadowSilhuette )
		return m_shadowSilhuette->boundSphere();
	else
		return 0.f;
}

int	ShadowVolume::buildSilhuettes()
{
	return 0;
}

int	ShadowVolume::clippedSilhuettes()
{
	return 0;
}

int	ShadowVolume::clippedVolumes()
{
	return 0;
}

int ShadowVolume::clippedTriangleVolumes()
{
	return 0;
}

int ShadowVolume::volumeCapPolygons()
{
	return 0;
}

int ShadowVolume::clippedSilhuetteQuads()
{
	return 0;
}

int	ShadowVolume::renderedShadows()
{
	return s_renderedShadows;
}

int	ShadowVolume::renderedShadowTriangles()
{
	return s_renderedShadowTriangles;
}

bool ShadowVolume::updateVisibility( const math::Matrix4x4& modelToCamera, 
	const ViewFrustum& viewFrustum )
{
	if ( !m_shadowSilhuette )
		return true;

	bool visible = m_shadowSilhuette->updateVisibility( modelToCamera, viewFrustum );
	/*if ( visible )
	{
		Vector3 viewDim;
		viewFrustum.getViewDimensions( &viewDim.x, &viewDim.y, &viewDim.z );
		
		const OBBox& shadowBox = m_shadowSilhuette->boundBox();
		Matrix4x4 shadowBoxInView = modelToCamera * shadowBox.transform();

		m_clipNeeded = Intersection::testBoxBox( viewDim, shadowBoxInView, shadowBox.dimensions() );
	}*/
	return visible;
}

int ShadowVolume::shadowTriangles() const
{
	return m_polys;
}

bool ShadowVolume::dynamicShadow() const
{
	return m_dynamicModel != 0;
}

void ShadowVolume::setViewOffset( float viewOffset )
{
	m_viewOffset = viewOffset;
}

VertexFormat ShadowVolume::vertexFormat() const
{
	return VertexFormat();
}

const int* ShadowVolume::usedBoneArray() const
{
	if ( m_dynamicModel )
		return m_dynamicModel->usedBoneArray();
	else
		return 0;
}

int ShadowVolume::usedBones() const
{
	if ( m_dynamicModel )
		return m_dynamicModel->usedBones();
	else
		return 0;
}


} // sg
