#include "Camera.h"
#include "Light.h"
#include "Scene.h"
#include "Context.h"
#include "NodeDistanceToCameraLess.h"
#include "BoundVolume.h"
#include <gd/GraphicsDevice.h>
#include <pix/Color.h>
#include <dev/Profile.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include <lang/Float.h>
#include <lang/Object.h>
#include <util/Vector.h>
#include <math/Vector4.h>
#include <math/Matrix4x4.h>
#include <algorithm>
#include <time.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace pix;
using namespace math;
using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

namespace sg
{


static Vector<Node*>	s_objs( Allocator<Node*>(__FILE__,__LINE__) );
static Vector<Light*>	s_lights( Allocator<Light*>(__FILE__,__LINE__) );
static Object			s_globalMutex( Object::OBJECT_INITMUTEX );
static int				s_cameras = 0;

//-----------------------------------------------------------------------------

/** Converts scene fog mode to graphics device fog mode. */
static gd::GraphicsDevice::FogMode togd( Scene::FogMode fog )
{
	switch ( fog )
	{
	case Scene::FOG_LINEAR:		return gd::GraphicsDevice::FOG_LINEAR;
	case Scene::FOG_EXP:		return gd::GraphicsDevice::FOG_EXP;
	case Scene::FOG_EXP2:		return gd::GraphicsDevice::FOG_EXP2;
	default:					return gd::GraphicsDevice::FOG_NONE;
	}
}

/** Sets fog to graphics device. */
static void setFog( gd::GraphicsDevice* dev, Scene* scene )
{
	gd::GraphicsDevice::FogMode gdfog = togd( scene->fog() );
	dev->setFog( gdfog );
	if ( scene->fog() != Scene::FOG_NONE )
	{
		dev->setFogColor( scene->fogColor() );
		if ( scene->fog() == Scene::FOG_LINEAR )
		{
			dev->setFogStart( scene->fogStart() );
			dev->setFogEnd( scene->fogEnd() );
		}
		else if ( scene->fog() == Scene::FOG_EXP || 
			scene->fog() == Scene::FOG_EXP2 )
		{
			dev->setFogDensity( scene->fogDensity() );
		}
	}
}

//-----------------------------------------------------------------------------

Camera::Camera() 
{
	defaults();
	++s_cameras;
}

Camera::Camera( const Camera& other ) : 
	Node(other)
{
	assign(other);
	++s_cameras;
}

Camera::~Camera()
{
	if ( --s_cameras == 0 )
	{
		s_objs.clear();
		s_objs.trimToSize();
		s_lights.clear();
		s_lights.trimToSize();
	}
}

Node* Camera::clone() const
{
	return new Camera( *this );
}

void Camera::blendState( anim::Animatable** anims,
	const float* times, const float* weights, int n )
{
	Node::blendState( anims, times, weights, n );

	float hfov	= 0.f;
	float invn = 1.f / (float)n;
	for ( int i = 0 ; i < n ; ++i )
	{
		Camera* cam = dynamic_cast<Camera*>( anims[i] );
		assert( cam );
		
		if ( cam )
			hfov += cam->horizontalFov() * invn;
	}

	setHorizontalFov( hfov );
}

void Camera::setViewport( int x, int y, int width, int height ) 
{
	m_x			= x;
	m_y			= y;
	m_width		= width;
	m_height	= height;
	m_aspect	= (float)width / (float)height;

	m_cachedWidthProjectionScalar = 0.f;
}

void Camera::getViewport( int* x, int* y, int* width, int* height ) const
{
	if (x)		*x			= m_x;
	if (y)		*y			= m_y;
	if (width)	*width		= viewportWidth();
	if (height)	*height		= viewportHeight();
}

int Camera::viewportWidth() const
{
	int w = m_width;
	gd::GraphicsDevice* dev = Context::device();
	if ( 0 == w && dev )
		w = dev->width();
	return w;
}

int Camera::viewportHeight() const
{
	int h = m_height;
	gd::GraphicsDevice* dev = Context::device();
	if ( 0 == h && dev )
		h = dev->height();
	return h;
}

int Camera::viewportCenterX() const
{
	return m_x + m_width/2;
}

int Camera::viewportCenterY() const
{
	return m_y + m_height/2;
}

void Camera::setHorizontalFov( float horzFov )
{
	m_viewFrustum.setHorizontalFov( horzFov );
	m_cachedWidthProjectionScalar = 0.f;
}

void Camera::setFront( float front )
{
	m_viewFrustum.setFront( front );
}

void Camera::setBack( float back )													
{
	m_viewFrustum.setBack( back );
}

void Camera::prepareRender()
{
	//Profile pr( "camera.prepareRender" );

	// - update transform hierarchy
	// - find out front and back plane distances
	// - collect visible objects and lights
	// - update node visibility
	// - sort visible objects by ascending distance
	// - set viewport, view- and projection transformation
	// - add affecting lights to rendering device
	// - set fog and ambient if any

	// - update transform hierarchy
	Node* root = this->root();
	Scene* scene = dynamic_cast<Scene*>( root );
	root->validateHierarchy();
	updateCachedTransforms();
	Vector3 camWorldPos = cachedWorldTransform().translation();
	Vector3 camWorldDir = cachedWorldTransform().rotation().getColumn(2);
	
	// - collect visible objects and lights
	s_objs.clear();
	s_lights.clear();

	for ( Node* obj = root ; obj ; )
	{
		//assert( obj->name().length() > 0 );
		obj->m_flags &= ~NODE_RENDEREDINLASTFRAME;

		if ( obj->enabled() )
		{
			if ( obj->renderable() )
			{
				Light* light = dynamic_cast<Light*>( obj );

				if ( light )
				{
					s_lights.add( light );
					++m_renderedLights;
				}
				else
				{
					obj->m_distanceToCamera = obj->boundSphere() + 
						(obj->m_worldTransform.translation() 
						- camWorldPos).dot( camWorldDir );

					if ( obj->updateVisibility(this) )
					{
						obj->m_flags |= NODE_RENDEREDINLASTFRAME;
						s_objs.add( obj );
						++m_renderedObjects;
					}
				}
			}
		}

		++m_processedObjects;
		obj = obj->nextInHierarchy( Node::NODE_ENABLED );
	}

	// - sort visible objects by ascending distance
	std::sort( s_objs.begin(), s_objs.end(), NodeDistanceToCameraLess() );

	// - set viewport, view- and projection transformation
	gd::GraphicsDevice* dev = Context::device();
	dev->setViewport( m_x, m_y, viewportWidth(), viewportHeight() );
	dev->setViewTransform( m_worldToCamera );
	dev->setProjectionTransform( projectionTransform() );

	// - add affecting lights to rendering device
	dev->removeLights();
	for ( int i = 0 ; i < (int)s_lights.size() ; ++i )
		s_lights[i]->apply();

	// - set fog and ambient if any
	if ( scene )
	{
		setFog( dev, scene );

		Colorf amb = Colorf( scene->ambientColor() );
		dev->setAmbient( Color(amb) );
	}
}

Matrix4x4 Camera::projectionTransform() const
{
	Matrix4x4 proj;
	proj.setPerspectiveProjection( m_viewFrustum.horizontalFov(), front(), back(), m_aspect );
	return proj;
}

Matrix4x4 Camera::viewTransform() const
{
	return worldTransform().inverse();
}

Matrix4x4 Camera::viewProjectionTransform() const
{
	Matrix4x4 view = worldTransform().inverse();
	return projectionTransform() * view;
}

void Camera::render() 
{
	synchronized( s_globalMutex );

	// - prepare scene rendering
	// - render objects in n passes
	// - update statistics
	// - reset temporaries

	gd::GraphicsDevice* dev = Context::device();
	int renderedPrimitives	= dev->renderedPrimitives();
	int renderedTriangles	= dev->renderedTriangles();
	int materialChanges		= dev->materialChanges();

	// - DEBUG: warn if we have non-square pixels
	Matrix4x4 proj = projectionTransform();
	Vector4 cp( 1,1,10,1 );
	Vector4 pp = (proj * cp);
	pp *= 1.f / pp.w;
	pp.x = pp.x * (viewportWidth()*.5f);
	pp.y = pp.y * (viewportHeight()*.5f);
	if ( Math::abs(pp.x-pp.y) > 1.f )
		Debug::println( "Non-square pixels! (fov={0}, w={1}, h={2})", Math::toDegrees(horizontalFov()), viewportWidth(), viewportHeight() );

	prepareRender();

	// - render objects in n passes

	// pass 1<<0: solid objects which are affected by shadows
	// pass 1<<1: (unused)
	// pass 1<<2: shadow volumes
	// pass 1<<3: shadow filler polygon
	// pass 1<<4: solid objects which are not affected by shadows
	// pass 1<<5: transparent objects which are not affected by shadows
	
	// the first pass is front to back...
	//Debug::println( "{0}({1})", __FILE__, __LINE__ );
	{//dev::Profile pr( "Camera.render( pass 1 )" );
	int i;
	for ( i = 0 ; i < (int)s_objs.size() ; ++i )
	{
		Node* obj = s_objs[i];
		//Debug::println( "rendering {0}({1})", obj->name(), i );
		obj->render( this, 1 );
	}
	}

	// ...and the rest back to front
	//Debug::println( "{0}({1})", __FILE__, __LINE__ );
	{//dev::Profile pr( "Camera.render( other passes )" );
	for ( int pass = 2 ; pass <= LAST_RENDERING_PASS ; pass <<= 1 )
	{
		for ( int i = (int)s_objs.size() ; i-- > 0 ; )
		{
			Node* obj = s_objs[i];
			obj->render( this, pass );
		}
	}
	}

	// - update statistics
	m_renderedPrimitives	+= dev->renderedPrimitives() - renderedPrimitives;
	m_renderedTriangles		+= dev->renderedTriangles()	- renderedTriangles;
	m_materialChanges		+= dev->materialChanges()	- materialChanges;

	// - reset temporaries
	m_worldToCamera = Matrix4x4(0);
	dev->setViewport( 0, 0, dev->width(), dev->height() );
	//Debug::println( "{0}({1})", __FILE__, __LINE__ );
}

void Camera::defaults()
{
	setRenderable( false );

	m_x								= 0;
	m_y								= 0;
	m_width							= 0;
	m_height						= 0;
	m_viewFrustum					= ViewFrustum();
	m_cachedWidthProjectionScalar	= 0.f;
	m_worldToCamera					= Matrix4x4(0);
	m_view							= Matrix4x4(0);
	m_projection					= Matrix4x4(0);
	m_viewProjection				= Matrix4x4(0);
	m_aspect						= 4.f/3.f;

	resetStatistics();
}

void Camera::assign( const Camera& other )
{
	m_x								= other.m_x;
	m_y								= other.m_y;
	m_width							= other.m_width;
	m_height						= other.m_height;
	m_viewFrustum					= other.m_viewFrustum;
	m_cachedWidthProjectionScalar	= other.m_cachedWidthProjectionScalar;
	m_worldToCamera					= other.m_worldToCamera;
	m_view							= other.m_view;
	m_projection					= other.m_projection;
	m_viewProjection				= other.m_viewProjection;
	m_aspect						= other.m_aspect;
}

void Camera::resetStatistics()
{
	m_processedObjects		= 0;
	m_renderedLights		= 0;
	m_renderedObjects		= 0;
	m_renderedPrimitives	= 0;
	m_renderedTriangles		= 0;
	m_materialChanges		= 0;
}

float Camera::front() const															
{
	return m_viewFrustum.front();
}

float Camera::back() const															
{
	return m_viewFrustum.back();
}

float Camera::horizontalFov() const
{
	return m_viewFrustum.horizontalFov();
}

float Camera::verticalFov() const
{
	return m_viewFrustum.verticalFov();
}

int	Camera::processedObjects() const												
{
	return m_processedObjects;
}

int	Camera::renderedLights() const													
{
	return m_renderedLights;
}

int	Camera::renderedObjects() const													
{
	return m_renderedObjects;
}

int	Camera::renderedPrimitives() const												
{
	return m_renderedPrimitives;
}

int	Camera::renderedTriangles() const												
{
	return m_renderedTriangles;
}

int Camera::materialChanges() const
{
	return m_materialChanges;
}

float Camera::getProjectedSize( float distanceZ, float size ) const
{
	assert( size >= 0.f );

	if ( distanceZ < m_viewFrustum.front() )
		distanceZ = m_viewFrustum.front();
	if ( 0.f == m_cachedWidthProjectionScalar )
		m_cachedWidthProjectionScalar = (viewportWidth() * 0.5f) / Math::tan( m_viewFrustum.horizontalFov() * 0.5f );

	float sizePixels = size/distanceZ * m_cachedWidthProjectionScalar;
	return sizePixels;
}

const ViewFrustum& Camera::viewFrustum() const
{
	return m_viewFrustum;
}

const Matrix4x4& Camera::cachedWorldToCamera() const
{
#ifdef _DEBUG
	assert( cachedWorldTransformValid() );
	Matrix4x4 invtm = cachedWorldTransform().inverse();
	assert( (m_worldToCamera.getColumn(0)-invtm.getColumn(0)).length() < 1e-6f );
	assert( (m_worldToCamera.getColumn(1)-invtm.getColumn(1)).length() < 1e-6f );
	assert( (m_worldToCamera.getColumn(2)-invtm.getColumn(2)).length() < 1e-6f );
	assert( (m_worldToCamera.getColumn(3)-invtm.getColumn(3)).length() < 1e-6f );
#endif
	return m_worldToCamera;
}

const Matrix4x4& Camera::cachedViewTransform() const
{
	assert( cachedWorldTransformValid() );
	return m_view;
}

const Matrix4x4& Camera::cachedProjectionTransform() const
{
	assert( cachedWorldTransformValid() );
	return m_projection;
}

const Matrix4x4& Camera::cachedViewProjectionTransform() const
{
	assert( cachedWorldTransformValid() );
	return m_viewProjection;
}

bool Camera::isInView( const Vector3& center, float radius ) const
{
	Vector3 centerInCamera = cachedWorldToCamera().transform( center );
	
	return BoundVolume::testSphereVolume( centerInCamera, radius, 
		viewFrustum().planes(), ViewFrustum::PLANE_COUNT );
}

void Camera::updateCachedTransforms()
{
	m_worldToCamera = worldTransform().inverse();
	m_view = viewTransform();
	m_projection = projectionTransform();
	m_viewProjection = viewProjectionTransform();
}


} // sg
