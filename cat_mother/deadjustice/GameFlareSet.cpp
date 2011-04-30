#include "GameFlareSet.h"
#include "ScriptUtil.h"
#include "CollisionInfo.h"
#include "GameRenderPass.h"
#include "GamePointObject.h"
#include "ScriptMethod.h"
#include <sg/Mesh.h>
#include <sg/Camera.h>
#include <sg/VertexLock.h>
#include <sg/VertexFormat.h>
#include <sg/TriangleList.h>
#include <sg/Effect.h>
#include <io/InputStream.h>
#include <io/InputStreamArchive.h>
#include <dev/Profile.h>
#include <sgu/NodeUtil.h>
#include <lang/Float.h>
#include <math/Vector2.h>
#include <script/ScriptException.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace sg;
using namespace sgu;
using namespace lang;
using namespace math;
using namespace util;
using namespace script;

//-----------------------------------------------------------------------------

ScriptMethod<GameFlareSet> GameFlareSet::sm_methods[] =
{
	//ScriptMethod<GameBoxTrigger>( "funcName", script_funcName ),
	ScriptMethod<GameFlareSet>( "addFlare", script_addFlare ),
};

//-----------------------------------------------------------------------------

GameFlareSet::GameFlareSet( script::VM* vm, InputStreamArchive* arch, const String& imageName, float worldSize, float fadeTime, int maxFlares ) :
	GameObject(vm,arch,0,0,0),
	m_tracer( new GamePointObject(CollisionInfo::COLLIDE_GEOMETRY_SOLID+CollisionInfo::COLLIDE_OBJECT) ),
	m_mesh( new Mesh ),
	m_tri( 0 ),
	m_worldSize( worldSize ),
	m_fadeTime( fadeTime ),
	m_maxFlares( maxFlares ),
	m_dt( 0.f ),
	m_flares( Allocator<GameFlare>(__FILE__) ),
	m_nextUpdated( 0 )
{
	m_methodBase = ScriptUtil<GameFlareSet,GameObject>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );

	// load flare texture
	P(InputStream) texin = arch->getInputStream( imageName );
	P(Texture) tex = new Texture( texin, imageName );
	texin->close();

	// flare vertex format
	VertexFormat vf;
	vf.addTextureCoordinate(2);
	vf.addTextureCoordinate(2);

	// init shader
	P(InputStream) fxin = arch->getInputStream( "misc/flare.fx" );
	P(Effect) fx = new Effect( fxin );
	fx->setTexture( "tDiffuse", tex );
	fx->setVertexFormat( vf );
	fx->setPass( GameRenderPass::RENDERPASS_FLARES );
	fx->setName( "flare shader" );
	fxin->close();

	// init flare triangle list
	m_tri = new TriangleList( maxFlares*3*2, vf, TriangleList::USAGE_DYNAMIC );
	m_tri->setShader( fx );
	m_tri->setVertices( 0 );
	m_mesh->addPrimitive( m_tri );
	m_mesh->setName( "GameFlareSet: " + imageName );
}

void GameFlareSet::update( float dt )
{
	GameObject::update( dt );

	m_dt += dt;
}

Node* GameFlareSet::getRenderObject( Camera* camera )
{
	if ( camera && m_flares.size() > 0 )
	{
		dev::Profile pr( "flares" );

		// camera info
		Matrix4x4 camtm = camera->cachedWorldTransform();
		Matrix3x3 camrot = camtm.rotation();
		Vector3 campos = camtm.translation();

		// flare world space info
		float flareRadius = m_worldSize * .5f;
		Vector3 dx = camrot.getColumn(0) * flareRadius;
		Vector3 dy = camrot.getColumn(1) * flareRadius;

		// fading speed
		float fadeDelta = 1.f;
		if ( m_fadeTime > Float::MIN_VALUE && m_dt < m_fadeTime )
			fadeDelta = m_dt/m_fadeTime;
		m_dt = 0.f;

		// maximum number of flares to update in a frame
		int maxUpdates = m_flares.size() / 3;
		if ( maxUpdates < 10 )
			maxUpdates = 10;
		if ( maxUpdates > m_flares.size() )
			maxUpdates = m_flares.size();
		fadeDelta *= (float)m_flares.size() / (float)maxUpdates;
		if ( fadeDelta > 1.f )
			fadeDelta = 1.f;

		// ranges of flares to update
		int updateStart1 = m_nextUpdated;
		int updateEnd1 = m_nextUpdated + maxUpdates;
		int updateStart2 = 0;
		int updateEnd2 = m_nextUpdated + maxUpdates;
		if ( updateEnd2 > m_flares.size() )
			updateEnd2 = updateEnd2 % m_flares.size();
		else
			updateEnd2 = 0;

		// next set to update
		m_nextUpdated = (m_nextUpdated + maxUpdates) % m_flares.size();

		// update flare visibility
		int visibleFlares = 0;
		for ( int i = 0 ; i < m_flares.size() ; ++i )
		{
			GameFlare& flare = m_flares[i];

			// compute new target fade level
			float fade = 0.f;
			if ( i >= updateStart1 && i < updateEnd1 || i >= updateStart2 && i < updateEnd2 || flare.fade() == -1.f )
			{
				Vector3 pos = flare.parent()->worldTransform().translation();
				if ( camera->isInView(pos,flareRadius) )
				{
					m_tracer->setPosition( cell(), pos );
					CollisionInfo cinfo;
					m_tracer->move( campos-pos, &cinfo );
					
					if ( !cinfo.isCollision(CollisionInfo::COLLIDE_ALL) )
						fade = 1.f;
				}
			}
			else
			{
				fade = flare.fade();
			}

			// update fade level
			float f = flare.fade();
			if ( f < 0.f )
				f = fade;
			if ( f < fade )
			{
				f += fadeDelta;
				if ( f > fade )
					f = fade;
			}
			else if ( f > fade )
			{
				f -= fadeDelta;
				if ( f < fade )
					f = fade;
			}
			if ( f > 0.f )
				++visibleFlares;
			flare.setFade( f );
		}

		// prepare triangle list
		m_tri->setVertices( visibleFlares*3*2 );
		if ( visibleFlares > 0 )
		{
			Vector2 tc[4] =
			{
				Vector2(0,0), Vector2(1,0), Vector2(1,1), Vector2(0,1)
			};
			int indices[6] =
			{
				0,1,2,
				0,2,3
			};

			VertexLock<TriangleList> lk( m_tri, TriangleList::LOCK_WRITE );
			float* vpos;
			int vpitch;
			int vcount = 0; vcount=vcount;
			m_tri->getVertexPositionData( &vpos, &vpitch );

			for ( int i = 0 ; i < m_flares.size() ; ++i )
			{
				GameFlare& flare = m_flares[i];
				float fade = flare.fade();
				if ( fade > 0.f )
				{
					Vector3 pos = flare.parent()->worldTransform().translation();
					Vector3 v[4] =
					{
						pos-dx+dy, pos+dx+dy, pos+dx-dy, pos-dx-dy
					};

					for ( int k = 0 ; k < 6 ; ++k )
					{
						const int ix = indices[k];
						const Vector3& vert = v[ix];
						const Vector2& texcoord = tc[ix];
						vpos[0] = vert.x;
						vpos[1] = vert.y;
						vpos[2] = vert.z;
						vpos[3] = texcoord.x;
						vpos[4] = texcoord.y;
						vpos[5] = fade;
						vpos[6] = fade;
						vpos += vpitch;
						assert( ++vcount <= m_tri->vertices() );
					}
				}
			}
		}
	}
	return m_mesh;
}

int GameFlareSet::methodCall( script::VM* vm, int i )
{
	return ScriptUtil<GameFlareSet,GameObject>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

int	GameFlareSet::script_addFlare( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_TABLE, VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects scriptable game object/cell and parent node name", funcName) );

	GameScriptable*		obj			= dynamic_cast<GameScriptable*>( getThisPtr(vm,1) );
	String				nodeName	= vm->toString(2);

	if ( !obj )
		throw ScriptException( Format("{0} expects scriptable game object/cell and parent node name, first parameter invalid type", funcName) );

	// find parent node
	Node* root = obj->getRenderObject(0);
	if ( !root )
		throw ScriptException( Format("{0} expects scriptable game object/cell and parent node name, first parameter not renderable object", funcName) );
	Node* parent = NodeUtil::findNodeByName( root, nodeName );
	if ( !parent )
		throw ScriptException( Format("{0} expects scriptable game object/cell and parent node name, node {1} not found from {2}", funcName, nodeName, root->name()) );

	// check number of flares
	if ( m_flares.size() >= m_maxFlares )
		throw ScriptException( Format("{0} expects scriptable game object/cell and parent node name, too many flares ({1}+) in set", funcName, m_flares.size()+1) );

	m_flares.add( GameFlare(parent) );
	return 0;
}
