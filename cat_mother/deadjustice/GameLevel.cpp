#include "GameLevel.h"
#include "GameCamera.h"
#include "GameFlareSet.h"
#include "GameBSPTree.h"
#include "GameCharacter.h"
#include "GameCell.h"
#include "GameRenderPass.h"
#include "GameCutScene.h"
#include "GamePortal.h"
#include "GameWeapon.h"
#include "GameNoiseManager.h"
#include "ProjectileManager.h"
#include "GameBoxTrigger.h"
#include "ScriptUtil.h"
#include <anim/Control.h>
#include <io/File.h>
#include <io/InputStream.h>
#include <io/InputStreamArchive.h>
#include <sg/Dummy.h>
#include <sg/Effect.h>
#include <sg/LineList.h>
#include <ps/ParticleSystemManager.h>
#include <sg/Node.h>
#include <sg/Camera.h>
#include <sg/Mesh.h>
#include <sg/Model.h>
#include <sg/VertexLock.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include <lang/String.h>
#include <lang/Float.h>
#include <lang/Character.h>
#include <math/lerp.h>
#include <math/Vector3.h>
#include <music/MusicManager.h>
#include <script/VM.h>
#include <script/ClassTag.h>
#include <script/ScriptException.h>
#include <sgu/NodeGroupSet.h>
#include <sgu/SceneManager.h>
#include <snd/SoundManager.h>
#include <util/Vector.h>
#include <algorithm>
#include "config.h"

//-----------------------------------------------------------------------------

ScriptMethod<GameLevel> GameLevel::sm_methods[] =
{
	ScriptMethod<GameLevel>( "createBoxTrigger", script_createBoxTrigger ),
	ScriptMethod<GameLevel>( "createCharacter", script_createCharacter ),
	ScriptMethod<GameLevel>( "createNoise", script_createNoise ),
	ScriptMethod<GameLevel>( "createWeapon", script_createWeapon ),
	ScriptMethod<GameLevel>( "createFlareSet", script_createFlareSet ),
	ScriptMethod<GameLevel>( "endLevel", script_endLevel ),
	ScriptMethod<GameLevel>( "getCell", script_getCell ),
	ScriptMethod<GameLevel>( "getPath", script_getPath ),
	ScriptMethod<GameLevel>( "getCharacter", script_getCharacter ),
	ScriptMethod<GameLevel>( "getDynamicObject", script_getDynamicObject ),
	ScriptMethod<GameLevel>( "loadDynamicObjects", script_loadDynamicObjects ),
	ScriptMethod<GameLevel>( "loadProjectiles", script_loadProjectiles ),
	ScriptMethod<GameLevel>( "importGeometry", script_importGeometry ),
	ScriptMethod<GameLevel>( "isActiveCutScene", script_isActiveCutScene ),
	ScriptMethod<GameLevel>( "playCutScene", script_playCutScene ),
	ScriptMethod<GameLevel>( "setBackgroundToCells", script_setBackgroundToCells ),
	ScriptMethod<GameLevel>( "setShadowColor", script_setShadowColor ),
	ScriptMethod<GameLevel>( "setMainCharacter", script_setMainCharacter ),
	ScriptMethod<GameLevel>( "signalExplosion", script_signalExplosion ),
	ScriptMethod<GameLevel>( "skipCutScene", script_skipCutScene ),
	ScriptMethod<GameLevel>( "removeCharacter", script_removeCharacter ),
	ScriptMethod<GameLevel>( "removeWeapon", script_removeWeapon ),
	ScriptMethod<GameLevel>( "removeTrigger", script_removeTrigger ),
	ScriptMethod<GameLevel>( "removeDynamicObjects", script_removeDynamicObjects ),
};

//-----------------------------------------------------------------------------

using namespace io;
using namespace sg;
using namespace ps;
using namespace sgu;
using namespace snd;
using namespace pix;
using namespace anim;
using namespace lang;
using namespace math;
using namespace util;
using namespace music;
using namespace script;

//-----------------------------------------------------------------------------

GameLevel::GameLevel( script::VM* vm, io::InputStreamArchive* arch, 
	snd::SoundManager* soundMgr, ps::ParticleSystemManager* particleMgr, 
	sgu::SceneManager* sceneMgr, music::MusicManager* musicMgr,
	ProjectileManager* projectileMgr, GameNoiseManager* noiseMgr, int bspBuildPolySkip ) :
	GameScriptable( vm, arch, soundMgr, particleMgr ),
	m_methodBase( -1 ),
	m_vm( vm ),
	m_arch( arch ),
	m_soundMgr( soundMgr ),
	m_particleMgr( particleMgr ),
	m_sceneMgr( sceneMgr ),
	m_musicMgr( musicMgr ),
	m_projectileMgr( projectileMgr ),
	m_noiseMgr( noiseMgr ),
	m_levelEnded( false ),
	m_animSet( new NodeGroupSet ),
	m_cells( Allocator<P(GameCell)>(__FILE__) ),
	m_bspBuildPolySkip( bspBuildPolySkip ),
	m_defaultCollisionMaterialType( 0 ),
	m_collisionMaterialTypes( Allocator<P(GameSurface)>(__FILE__) ),
	m_shadowColor(1,1,1,1),
	m_backgrounds( Allocator<P(Node)>(__FILE__) ),
	m_lightmapShader( 0 ),
	m_triggerList( Allocator<P(GameBoxTrigger)>(__FILE__) ),
	m_pathList( Allocator<P(GamePath)>(__FILE__) ),
	m_characterList( Allocator<P(GameCharacter)>(__FILE__) ),
	m_weaponList( Allocator<P(GameWeapon)>(__FILE__) ),
	m_dynamicObjectList( Allocator<P(GameDynamicObject)>(__FILE__) ),
	m_dynamicObjectBSPs( Allocator< HashtablePair<String,P(bsp::BSPTree)> >(__FILE__) ),
	m_flareSetList( Allocator<P(GameFlareSet)>(__FILE__) ),
	m_mainCharacter( 0 ),
	m_cutScene( 0 ),
	m_removed( Allocator<P(GameObject)>(__FILE__) )
{
	m_methodBase = ScriptUtil<GameLevel,GameScriptable>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );

	// load lightmap shader
	P(InputStream) in = m_arch->getInputStream( "misc/lightmap.fx" );
	m_lightmapShader = new Effect( in );
	in->close();
}

GameLevel::~GameLevel()
{
	m_vm = 0;
	m_arch = 0;
	m_soundMgr = 0;
	m_particleMgr = 0;
	m_sceneMgr = 0;
	m_musicMgr = 0;
	m_projectileMgr = 0;
	m_animSet = 0;
	m_characterList.clear();
	m_weaponList.clear();
	m_mainCharacter = 0;

	for ( int i = 0; i < m_cells.size() ; ++i )
		m_cells[i]->m_level = 0;
	m_cells.clear();
}

GameDynamicObject* GameLevel::createDynamicObject( sg::Node* node, const lang::String& bspFileName, const math::Matrix4x4& tm, GameCell* cell )
{
	if ( m_dynamicObjectBSPs[bspFileName] )
		Debug::println( "Creating dynamic object {0} (BSP from cache)", node->name() );
	else
		Debug::println( "Creating dynamic object {0}", node->name() );

	// create DO
	P(GameDynamicObject) dynamicObject = new GameDynamicObject( vm(), archive(), m_sceneMgr, soundManager(), particleSystemManager(), m_projectileMgr, m_noiseMgr, node, bspFileName, m_bspBuildPolySkip, m_collisionMaterialTypes, m_dynamicObjectBSPs[bspFileName] );
	dynamicObject->setTransform( cell, tm );
	dynamicObject->setName( node->name() );

	// cache BSP
	m_dynamicObjectBSPs[bspFileName] = dynamicObject->bspTree()->tree();

	// check for unique names
	for ( int i = 0 ; i < m_dynamicObjectList.size() ; ++i )
	{
		if ( m_dynamicObjectList[i]->name() == dynamicObject->name() )
			throw Exception( Format("Tried to add second dynamic object with name {0}", dynamicObject->name()) );
	}
	
	m_dynamicObjectList.add( dynamicObject );
	return dynamicObject;
}

void GameLevel::loadFile( const String& filename ) 
{
	String path = File(m_arch->getInputStream( filename )->toString()).getParent();
	P(sg::Node) scene = m_sceneMgr->getScene( filename )->clone();

	replaceLightmapMaterialsWithShader( scene, m_lightmapShader );

	// Check that all top level nodes are either cells or portals
	for ( Node* it = scene->firstChild() ; it ; it = scene->getNextChild(it) )
	{
		if ( getNodeClass(it) == NODECLASS_NORMAL )
			Debug::printlnWarning( "Top-level object {0} is not CELL, PORTAL or BACKGROUND", it->name() );
	}

	// Extract background objects
	for ( Node* it = scene; it ; it = it->nextInHierarchy() )
	{
		if ( getNodeClass(it) == NODECLASS_BACKGROUND )
		{
			P(Node) bg = it->clone();
			Debug::println( "Found background {0}", bg->name() );
			bg->setTransform( it->worldTransform() );
			m_backgrounds.add( bg );
		}
	}

	// Build list of Cells
	Vector<Matrix4x4> dynamicObjectTms( Allocator<Matrix4x4>(__FILE__) );
	Vector<P(Node)> dynamicObjects( Allocator<P(Node)>(__FILE__) );
	for ( Node* iterator = scene; iterator; iterator = iterator->nextInHierarchy() )
	{
		// Detect CELL
		if ( getNodeClass( iterator ) == NODECLASS_CELL )
		{
			P(Node) cellScene = iterator->clone();
			Debug::println( "Found cell {0}", cellScene->name() );

			setRenderPasses( cellScene, GameRenderPass::RENDERPASS_LEVEL_SOLID, GameRenderPass::RENDERPASS_LEVEL_TRANSPARENT );

			// translate cell to origin (=cell children to world space)
			// NOTE: this breaks animated objects parented to cell like cameras
			Matrix4x4 celltm = cellScene->transform();
			for ( Node* child = cellScene->firstChild() ; child ; child = cellScene->getNextChild(child) )
				child->setTransform( celltm * child->transform() );
			cellScene->setTransform( Matrix4x4(1.f) );

			// find and remove dynamic object nodes from the scene graph
			dynamicObjectTms.clear();
			dynamicObjects.clear();
			for ( Node* child = cellScene ; child ; child = child->nextInHierarchy() )
			{
				if ( child->name().indexOf("DYNAMIC") != -1 )
				{
					dynamicObjectTms.add( child->worldTransform() );
					dynamicObjects.add( child );
				}
			}
			for ( int i = 0 ; i < dynamicObjects.size() ; ++i )
				dynamicObjects[i]->unlink();
			
			// create cell object
			String name = cellScene->name();
			String path = File(m_arch->getInputStream( filename )->toString()).getParent();
			String bspFileName = path + "/" + name + ".bsp";
			P(GameCell) cell = new GameCell( m_vm, m_arch, m_soundMgr, m_particleMgr, name, cellScene, bspFileName, m_bspBuildPolySkip, m_collisionMaterialTypes );
			cell->m_level = this;
			m_cells.add( cell );

			// create dynamic objects
			for ( int i = 0 ; i < dynamicObjects.size() ; ++i )
			{
				P(Node) node = dynamicObjects[i];
				Matrix4x4 tm = dynamicObjectTms[i];
				String bspFileName = path + "/" + removeEndNumberSuffix(node->name()) + ".bsp";

				createDynamicObject( node, bspFileName, tm, cell );
			}
		}
	}

	// Add portals to cells, each portal is mirrored to two cells (the ones it is connecting)
	for ( sg::Node* iterator = scene; iterator; iterator = iterator->nextInHierarchy() )
	{
		// Detect PORTAL
		if ( getNodeClass( iterator ) == NODECLASS_PORTAL )
		{
			// check that we have Dummy node
			Dummy* dummy = dynamic_cast<Dummy*>( iterator );
			if ( !dummy )
				throw Exception( Format("Portal {0} must be dummy object", iterator->name()) );

			// check name
			const String& name = dummy->name();
			int separator = name.indexOf('-');
			if ( separator == -1 )
				throw Exception( Format("Cell Separator '-' not found in Portal {0} node name!", dummy->name()) );
			
			// get cell names
			String firstcell = name.substring( 7, separator );
			String secondcell = name.substring( separator + 1 );
		
			// make portal corners
			Matrix4x4 worldtransform = dummy->worldTransform();
			Matrix3x3 wrot = worldtransform.rotation();

			Vector3 dx = wrot.getColumn(0) * dummy->boxMax().x;
			Vector3 dy = wrot.getColumn(1) * dummy->boxMax().y;
			Vector3 dz = wrot.getColumn(2) * dummy->boxMax().z;
			Vector3 center = worldtransform.translation();
			float scalex = dx.length();
			float scaley = dy.length();
			float scalez = dz.length();
			float absminscale = Math::min( Math::abs(scalex), Math::min(Math::abs(scaley),Math::abs(scalez)) );
			Vector3 corners[4];

			if ( Math::abs(scalex) <= absminscale )
			{
				// portal in y,z plane
				corners[0] = center - dz + dy;
				corners[1] = center + dz + dy;
				corners[2] = center + dz - dy;
				corners[3] = center - dz - dy;
			}
			else if ( Math::abs(scaley) <= absminscale )
			{
				// portal in x,z plane
				corners[0] = center - dx + dz;
				corners[1] = center + dx + dz;
				corners[2] = center + dx - dz;
				corners[3] = center - dx - dz;
			}
			else if ( Math::abs(scalez) <= absminscale )
			{
				// portal in x,y plane
				corners[0] = center - dx + dy;
				corners[1] = center + dx + dy;
				corners[2] = center + dx - dy;
				corners[3] = center - dx - dy;
			}
			else
			{
				throw Exception( Format("Portal {0} is not smallest in X-dimension", dummy->name()) );
			}

			GameCell* first = getCell( firstcell );
			GameCell* second = getCell( secondcell );

			if ( first == 0 || second == 0 )
				throw Exception( Format( "Portal name {0} contains invalid Cell names!", dummy->name()) );
 
			// add portals to the cells
			P(GamePortal) portal1 = new GamePortal( corners, second );
			first->addPortal( portal1 );
			std::reverse( corners, corners+GamePortal::NUM_CORNERS );
			P(GamePortal) portal2 = new GamePortal( corners, first );
			second->addPortal( portal2 );
		}
	}

	// Build list of AI guard paths
	for ( int i = 0 ; i < cells() ; ++i )
	{
		GameCell* cell = getCell(i);
		for ( Node* it = cell->getRenderObject(0) ; it ; it = it->nextInHierarchy() )
		{
			Mesh* mesh = dynamic_cast<Mesh*>( it );
			if ( mesh )
			{
				Matrix4x4 tm = mesh->worldTransform();
				for ( int k = 0 ; k < mesh->primitives() ; ++k )
				{
					P(LineList) lineList = dynamic_cast<LineList*>( mesh->getPrimitive(k) );
					if ( lineList )
					{
						VertexLock<LineList> lk( lineList, LineList::LOCK_READ );
						Debug::println( "Found AI guard path {0} from cell {1}", mesh->name(), cell->name() );
						Debug::println( "AI guard path (lines={1}) found from {0}", mesh->name(), lineList->lines() );
						if ( mesh->primitives() > 1 )
							throw Exception( Format("AI guard path in {0} must be defined by single poly line", mesh->name() ) );
					
						P(GamePath) path = new GamePath( m_vm, m_arch, mesh->name() );
						for ( int n = 0 ; n < lineList->lines() ; ++n )
						{
							Vector3 start, end;
							lineList->getLine( n, &start, &end );
							start = tm.transform( start );
							path->addPoint( start );
						}

						Vector3 xaxis = (path->getPoint(1) - path->getPoint(0)).normalize();
						path->setNumber( "startPointDirection0", xaxis.x );
						path->setNumber( "startPointDirection1", xaxis.y );
						path->setNumber( "startPointDirection2", xaxis.z );

						path->setTable( "startPointCell", cell );
						m_pathList.add( path );
						mesh->removePrimitive(k);
						break;
					}
				}
			}

			Dummy* dummy = dynamic_cast<Dummy*>( it );
			if ( dummy )
			{
				Debug::println( "Found AI guard dummy {0} from cell {1}", dummy->name(), cell->name() );
				
				P(GamePath) path = new GamePath( m_vm, m_arch, dummy->name() );
				path->addPoint( dummy->worldTransform().translation() );

				Vector3 xaxis = dummy->worldTransform().rotation().getColumn(0).normalize();
				path->setNumber( "startPointDirection0", xaxis.x );
				path->setNumber( "startPointDirection1", xaxis.y );
				path->setNumber( "startPointDirection2", xaxis.z );

				path->setTable( "startPointCell", cell );
				m_pathList.add( path );
			}
		}
	}
}

bool GameLevel::ended() const
{
	return m_levelEnded;
}

GameLevel::NodeClass GameLevel::getNodeClass( sg::Node* node )
{
	const String& name = node->name();

	if ( name.indexOf("PORTAL") != -1 )
		return NODECLASS_PORTAL;
	else if ( name.indexOf("CELL") != -1 )
		return NODECLASS_CELL;
	else if ( name.indexOf("PATH") != -1 )
		return NODECLASS_PATH;
	else if ( dynamic_cast<Camera*>(node) )
		return NODECLASS_CAMERA;
	else if ( name.indexOf("BACKGROUND") != -1 )
		return NODECLASS_BACKGROUND;
	else
		return NODECLASS_NORMAL;
}

void GameLevel::update( float dt )
{
	m_removed.clear();

	GameScriptable::update( dt );

	// update cells
	for ( int i = 0 ; i < m_cells.size() ; ++i )
		m_cells[i]->update( dt );

	// update active cut scene if any
	if ( m_cutScene )
	{
		m_cutScene->update( dt );
		if ( m_cutScene->ended() )
			m_cutScene = 0;
	}
}

void GameLevel::removeObject( GameObject* obj )
{
	P(GameObject) o = obj;
	obj->removeTimerEvents();
	obj->setPosition( 0, Vector3(0,0,0) );
	m_removed.add( obj );
}

void GameLevel::removeNonMainCharacters()
{
	for ( int i = 0 ; i < m_characterList.size() ; ++i )
	{
		if ( m_characterList[i] != mainCharacter() )
		{
			removeObject( m_characterList[i] );
			m_characterList.remove( i-- );
		}
	}
}

void GameLevel::skipCutScene()
{
	if ( m_cutScene )
	{
		pushMethod( "signalSkipCutScene" );
		vm()->pushTable( m_cutScene );
		call( 1, 0 );
	}
}

int GameLevel::cells() const 
{
	return m_cells.size();
}

int GameLevel::characters() const 
{
	return m_characterList.size();
}

int GameLevel::triggers() const 
{
	return m_triggerList.size();
}

int GameLevel::paths() const 
{
	return m_pathList.size();
}

GameCell* GameLevel::getCell( int index ) const
{
	return m_cells[index].ptr();
}

GameCell* GameLevel::getCell( const String& name ) const
{
	for ( int i = 0; i < m_cells.size(); ++i )
		if ( m_cells[i]->name() == name )
			return m_cells[i].ptr();

	throw Exception( Format("Cell {0} not found", name) );
	return 0;
}

GameCharacter* GameLevel::getCharacter( const String& name ) const
{
	for ( int i = 0; i < m_characterList.size(); ++i )
		if ( m_characterList[i]->name() == name )
			return m_characterList[i].ptr();

	throw Exception( Format("Character {0} not found", name) );
	return 0;
}

GameCharacter*	GameLevel::getCharacter( int index ) const 
{
	return m_characterList[index];
}

GameBoxTrigger*	GameLevel::getTrigger( int index ) const 
{
	return m_triggerList[index];
}

GamePath* GameLevel::getPath( int index ) const 
{
	return m_pathList[index];
}

int GameLevel::dynamicObjects() const
{
	return m_dynamicObjectList.size();
}

GameDynamicObject* GameLevel::getDynamicObject( int i ) const
{
	return m_dynamicObjectList[i];
}

GameDynamicObject* GameLevel::getDynamicObject( const String& name ) const 
{
	for ( int i = 0; i < m_dynamicObjectList.size(); ++i )
		if ( m_dynamicObjectList[i]->name() == name )
			return m_dynamicObjectList[i].ptr();

	throw Exception( Format("Dynamic object {0} not found", name) );
	return 0;
}

GamePath* GameLevel::getPath( const String& name ) const 
{
	for ( int i = 0; i < m_pathList.size(); ++i )
		if ( m_pathList[i]->name() == name )
			return m_pathList[i].ptr();

	throw Exception( Format("Path {0} not found", name) );
	return 0;
}

GameCharacter*	GameLevel::mainCharacter() const 
{
	assert( m_mainCharacter );
	return m_mainCharacter;
}

GameSurface* GameLevel::defaultCollisionMaterialType() const
{
	return m_defaultCollisionMaterialType;
}

GameWeapon*		GameLevel::getWeapon( int index ) const 
{
	return m_weaponList[index];
}

int GameLevel::weapons() const 
{
	return m_weaponList.size();
}

GameCutScene* GameLevel::activeCutScene() const
{
	assert( m_cutScene );
	return m_cutScene;
}

bool GameLevel::isActiveCutScene() const
{
	return m_cutScene != 0;
}

pix::Color GameLevel::shadowColor() const
{
	return m_shadowColor;
}

void GameLevel::removeWeapon( GameWeapon* weapon )
{
	int index = m_weaponList.indexOf(weapon);
	if ( index != -1 )
	{
		P(GameWeapon) weapon = m_weaponList[index];
		removeObject( m_weaponList[index] );
		m_projectileMgr->removeWeaponProjectiles( m_weaponList[index] );
		m_weaponList.remove( index );
	}
}

void GameLevel::signalViewChanged()
{
	for ( int i = 0 ; i < m_characterList.size() ; ++i )
	{
		GameCharacter* obj = m_characterList[i];
		obj->forceVisible();
		if ( obj->hasWeapon() )
			obj->weapon()->forceVisible();
	}

	for ( int i = 0 ; i < m_flareSetList.size() ; ++i )
	{
		GameFlareSet* flareSet = m_flareSetList[i];
		for ( int k = 0 ; k < flareSet->flares() ; ++k )
			flareSet->getFlare(k).setFade( -1.f );
	}
}

String GameLevel::removeEndNumberSuffix( const String& str )
{
	String s = str;
	int suffix = s.length()-1;
	while ( suffix > 0 && Character::isDigit(s.charAt(suffix)) )
		--suffix;

	if ( suffix != s.length()-1 && suffix > 0 && s.charAt(suffix) == '_' )
		s = s.substring( 0, suffix );

	return s;
}

int GameLevel::methodCall( VM* vm, int i )
{
	return ScriptUtil<GameLevel,GameScriptable>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

int	GameLevel::script_importGeometry( script::VM* vm, const char* funcName )
{
	int tags1[] = {VM::TYPE_STRING, VM::TYPE_TABLE};
	if ( !hasParams(tags1,sizeof(tags1)/sizeof(tags1[0])) )
		throw ScriptException( Format("{0} expects level scene file name and a table of collision material types (first one default)", funcName) );

	String sceneName = vm->toString(1);
	
	// create collision material types
	Vector<P(GameSurface)> collisionMaterialTypes( Allocator<P(GameSurface)>(__FILE__) );
	Table tab = vm->toTable(2);
	for ( int i = 1 ; !tab.isNil(i) ; ++i )
	{
		Table mtl = tab.getTable(i);

		String typeName = mtl.getString( "typeName" );
		float sneakingNoiseLevel = mtl.getNumber( "sneakingNoiseLevel" );
		float walkingNoiseLevel = mtl.getNumber( "walkingNoiseLevel" );
		float runningNoiseLevel = mtl.getNumber( "runningNoiseLevel" );
		float movementNoiseDistance = mtl.getNumber( "movementNoiseDistance" );

		P(GameSurface) gameSurface = new GameSurface( m_vm, m_arch, typeName );
		gameSurface->setString( "typeName", typeName );
		gameSurface->setNumber( "sneakingNoiseLevel", sneakingNoiseLevel );
		gameSurface->setNumber( "walkingNoiseLevel", walkingNoiseLevel );
		gameSurface->setNumber( "runningNoiseLevel", runningNoiseLevel );
		gameSurface->setNumber( "movementNoiseDistance", movementNoiseDistance );

		collisionMaterialTypes.add( gameSurface );
	}
	if ( collisionMaterialTypes.size() == 0 )
		throw ScriptException( Format("No collision material types given to {0} call", funcName) );
	m_collisionMaterialTypes = collisionMaterialTypes;
	m_defaultCollisionMaterialType = collisionMaterialTypes.firstElement();

	loadFile( sceneName );
	return 0;
}

int GameLevel::script_loadDynamicObjects( script::VM* vm, const char* funcName )
{
	int tags1[] = {VM::TYPE_STRING};
	if ( !hasParams(tags1,sizeof(tags1)/sizeof(tags1[0])) )
		throw ScriptException( Format("{0} expects level scene file name", funcName) );

	String filename = vm->toString(1);
	String path = File(m_arch->getInputStream( filename )->toString()).getParent();
	P(Node) scene = m_sceneMgr->getScene( filename )->clone();

	Table names( vm );
	int nameIndex = 0;
	Vector<Matrix4x4> dynamicObjectTms( Allocator<Matrix4x4>(__FILE__) );
	Vector<P(Node)> dynamicObjects( Allocator<P(Node)>(__FILE__) );
	for ( Node* iterator = scene; iterator; iterator = iterator->nextInHierarchy() )
	{
		// Detect CELL
		if ( getNodeClass( iterator ) == NODECLASS_CELL )
		{
			P(Node) cellScene = iterator->clone();
			Debug::println( "loadDynamicObjects: Found cell {0}", cellScene->name() );

			// translate cell to origin (=cell children to world space)
			// NOTE: this breaks animated objects parented to cell like cameras
			Matrix4x4 celltm = cellScene->transform();
			for ( Node* child = cellScene->firstChild() ; child ; child = cellScene->getNextChild(child) )
				child->setTransform( celltm * child->transform() );
			cellScene->setTransform( Matrix4x4(1.f) );

			// find and remove dynamic object nodes from the scene graph
			dynamicObjectTms.clear();
			dynamicObjects.clear();
			for ( Node* child = cellScene ; child ; child = child->nextInHierarchy() )
			{
				if ( child->name().indexOf("DYNAMIC") != -1 )
				{
					dynamicObjectTms.add( child->worldTransform() );
					dynamicObjects.add( child );
				}
			}
			for ( int i = 0 ; i < dynamicObjects.size() ; ++i )
				dynamicObjects[i]->unlink();
			
			// find cell object
			String name = cellScene->name();
			GameCell* cell = getCell( name );

			// create dynamic objects
			for ( int i = 0 ; i < dynamicObjects.size() ; ++i )
			{
				P(Node) node = dynamicObjects[i];
				Matrix4x4 tm = dynamicObjectTms[i];
				tm.setTranslation( Vector3(0,-1e6f,0) );
				String bspFileName = path + "/" + removeEndNumberSuffix(node->name()) + ".bsp";

				P(GameDynamicObject) dynamicObject = createDynamicObject( node, bspFileName, tm, cell );

				// return names of added dynamic objects
				names.setString( ++nameIndex, dynamicObject->name() );
			}
		}
	}

	vm->pushTable( names );
	return 1;
}

int GameLevel::script_removeCharacter( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_TABLE};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects character object", funcName) );

	P(GameCharacter) obj = dynamic_cast<GameCharacter*>( getThisPtr(vm, 1) );
	if ( !obj )
		throw ScriptException( Format("{0} expects character object", funcName) );
	int index = m_characterList.indexOf( obj );
	if ( index != -1 )
	{
		removeObject( m_characterList[index] );
		m_characterList.remove( index );
		Debug::println( "Removed character {0}", obj->name() );

		P(GameWeapon) weapon = obj->weapon();
		if ( weapon )
		{
			int index = m_weaponList.indexOf( weapon );
			if ( index != -1 )
			{
				removeObject( m_weaponList[index] );
				m_projectileMgr->removeWeaponProjectiles( m_weaponList[index] );
				m_weaponList.remove( index );
				Debug::println( "Removed weapon {0}", weapon->name() );
			}
		}
	}

	return 0;
}

int GameLevel::script_removeWeapon( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_TABLE};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects weapon object", funcName) );

	P(GameWeapon) obj = dynamic_cast<GameWeapon*>( getThisPtr(vm, 1) );
	if ( !obj )
		throw ScriptException( Format("{0} expects weapon object", funcName) );
	int index = m_weaponList.indexOf( obj );
	if ( index != -1 )
	{
		removeObject( m_weaponList[index] );
		m_projectileMgr->removeWeaponProjectiles( m_weaponList[index] );
		m_weaponList.remove( index );
		Debug::println( "Removed weapon {0}", obj->name() );
	}

	return 0;
}

int GameLevel::script_removeTrigger( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_TABLE};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects trigger object", funcName) );

	P(GameBoxTrigger) obj = dynamic_cast<GameBoxTrigger*>( getThisPtr(vm, 1) );
	if ( !obj )
		throw ScriptException( Format("{0} expects trigger object", funcName) );
	int index = m_triggerList.indexOf( obj );
	if ( index != -1 )
	{
		removeObject( m_triggerList[index] );
		m_triggerList.remove( index );
		Debug::println( "Removed trigger {0}", obj->name() );
	}

	return 0;
}

int GameLevel::script_removeDynamicObjects( script::VM* vm, const char* funcName )
{
	int tags1[] = {VM::TYPE_TABLE};
	if ( !hasParams(tags1,sizeof(tags1)/sizeof(tags1[0])) )
		throw ScriptException( Format("{0} expects table of dynamic object names", funcName) );

	Table names = vm->toTable(1);
	for ( int i = 1 ; !names.isNil(i) ; ++i )
	{
		String name = names.getString(i);
		P(GameDynamicObject) obj = getDynamicObject(name);
		removeObject( obj );
		m_dynamicObjectList.remove( m_dynamicObjectList.indexOf(obj) );
		Debug::println( "Removed dynamic object {0}", name );
	}
	
	return 0;
}

int GameLevel::script_getCell( script::VM* vm, const char* funcName )
{
	int tags1[] = {VM::TYPE_STRING};
	int tags2[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags1,sizeof(tags1)/sizeof(tags1[0])) &&
		!hasParams(tags2,sizeof(tags2)/sizeof(tags2[0])) )
		throw ScriptException( Format("{0} returns cell by name or index.", funcName) );

	if ( vm->isNumber(1) )
	{
		int index = (int)vm->toNumber(1);
		if ( index < 1 || index > cells() )
			throw ScriptException( Format("{0}: Invalid index {1}, number of cells is {2}.", funcName, index, cells()) );
		index -= 1;

		vm->pushTable( getCell(index) );
	}
	else
	{
		String name = vm->toString(1);
		GameCell* cell = getCell( name );
		if ( !cell )
			throw ScriptException( Format("{0}: Cell {1} not found", funcName, name) );

		vm->pushTable( cell );
	}
	return 1;
}

int GameLevel::script_createBoxTrigger( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects trigger script filename and trigger dummy object name", funcName ) );

	String scriptName = vm->toString(1);
	String dummyName = vm->toString(2);

	P(GameBoxTrigger) trigger = new GameBoxTrigger( m_vm, m_arch, m_soundMgr );
	trigger->compile( scriptName );

	bool triggerCellFound = false;
	for ( int i = 0 ; i < m_cells.size() && !triggerCellFound ; ++i )
	{
		for ( Node* node = m_cells[i]->getRenderObject(0) ; node ; node = node->nextInHierarchy() )
		{
			if ( node->name() == dummyName )
			{
				Dummy* dummy = dynamic_cast<Dummy*>( node );
				if ( !dummy )
					throw ScriptException( Format("Tried to create box trigger {0} but {1} is not dummy object", scriptName, dummyName) );

				trigger->setTransform( m_cells[i], dummy->worldTransform() );
				trigger->setDimensions( dummy->boxMax() );
				triggerCellFound = true;
				break;
			}
		}
	}
	if ( !triggerCellFound )
		throw ScriptException( Format("Tried to create box trigger {0} but dummy object {1} was not found", scriptName, dummyName) );

	m_triggerList.add( trigger );
	vm->pushTable( trigger );
	return 1;
}

int GameLevel::script_createCharacter( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects script filename", funcName ) );

	P(GameCharacter) nu = new GameCharacter( m_vm, m_arch, m_animSet, m_sceneMgr, m_soundMgr, m_particleMgr, m_projectileMgr, m_noiseMgr );
	nu->compile( vm->toString(1) );

	m_characterList.add( nu );

	vm->pushTable( nu );
	return 1;
}

int GameLevel::script_createWeapon( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects script filename", funcName ) );

	P(GameWeapon) nu = new GameWeapon( m_vm, m_arch, m_soundMgr, m_particleMgr, m_sceneMgr, m_projectileMgr, m_noiseMgr );
	nu->compile( vm->toString(1) );

	m_weaponList.add( nu );

	vm->pushTable( nu );
	return 1;
}

int GameLevel::script_setMainCharacter( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_TABLE};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects GameCharacter object table", funcName ) );

	GameCharacter* c = dynamic_cast<GameCharacter*>(getThisPtr(vm, 1 ));
	if ( !c )
		throw ScriptException( Format("{0} expects GameCharacter object table", funcName ) );

	m_mainCharacter = c;
	m_mainCharacter->setControlSource( GameCharacter::CONTROL_USER );
	return 0;
}

int GameLevel::script_getPath( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} returns guard path by name", funcName ) );

	String pathName = vm->toString(1);
	GamePath* path = getPath( pathName );

	vm->pushTable( path );
	return 1;
}

int GameLevel::script_getCharacter( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} returns character by name", funcName ) );

	String characterName = vm->toString(1);
	GameCharacter* character = getCharacter( characterName );

	vm->pushTable( character );
	return 1;
}

int GameLevel::script_getDynamicObject( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} returns dynamic level geometry object by name", funcName ) );

	String name = vm->toString(1);
	GameDynamicObject* obj = getDynamicObject( name );

	vm->pushTable( obj );
	return 1;
}

int	GameLevel::script_createNoise( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_TABLE, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects noise source game object, noise level, fade distance start, fade distance end, fade out time start, fade out time end", funcName) );

	int param = 1;
	GameObject*		source				= dynamic_cast<GameObject*>( getThisPtr(vm, param++) );
	float			noiseLevel			= vm->toNumber( param++ );
	float			fadeDistanceStart	= vm->toNumber( param++ );
	float			fadeDistanceEnd		= vm->toNumber( param++ );
	float			fadeOutTimeStart	= vm->toNumber( param++ );
	float			fadeOutTimeEnd		= vm->toNumber( param++ );

	if ( isActiveCutScene() )
	{
		noiseLevel = 0.f;
		fadeDistanceStart = 0.f;
		fadeDistanceEnd = 0.f;
	}

	if ( !source )
		throw ScriptException( Format("{0} expects noise source game object, noise level, fade distance start, fade distance end, fade out time start, fade out time end", funcName) );

	Debug::println( "{0} created noise: level={1}, distance={2}", source->name(), noiseLevel, fadeDistanceEnd );
	GameNoise* noise = m_noiseMgr->createNoise( source, noiseLevel, fadeDistanceStart, fadeDistanceEnd, fadeOutTimeStart, fadeOutTimeEnd );
	vm->pushTable( noise );
	return 1;
}

int GameLevel::script_signalExplosion( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_TABLE, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects explosion source, damage, damage fade out start distance, damage fade out end distance", funcName) );

	int param = 1;
	GameObject*		source				= dynamic_cast<GameObject*>( getThisPtr(vm, param++) );
	float			damage				= vm->toNumber( param++ );
	float			fadeDistanceStart	= vm->toNumber( param++ );
	float			fadeDistanceEnd		= vm->toNumber( param++ );
	float			endDistanceSqr		= fadeDistanceEnd * fadeDistanceEnd;

	if ( !source )
		throw ScriptException( Format("{0} expects explosion source, damage, damage fade out start distance, damage fade out end distance", funcName) );

	for ( int i = 0 ; i < m_characterList.size() ; ++i )
	{
		GameCharacter* obj = m_characterList[i];
		float distSqr = Math::max( 0.f, (obj->position() - source->position()).lengthSquared() - obj->boundSphere()*obj->boundSphere() );
		if ( distSqr < endDistanceSqr )
		{
			float causedDamage = lerp( damage, 0.f, (Math::sqrt(distSqr)-fadeDistanceStart)/Math::max(fadeDistanceEnd-fadeDistanceStart,1e-6f) );
			obj->receiveExplosion( source, causedDamage );
		}
	}

	for ( int i = 0 ; i < m_dynamicObjectList.size() ; ++i )
	{
		GameDynamicObject* obj = m_dynamicObjectList[i];
		if ( obj != source )
		{
			float distSqr = Math::max( 0.f, (obj->position() - source->position()).lengthSquared() - obj->boundSphere()*obj->boundSphere() );
			if ( distSqr < endDistanceSqr )
			{
				float causedDamage = lerp( damage, 0.f, (Math::sqrt(distSqr)-fadeDistanceStart)/Math::max(fadeDistanceEnd-fadeDistanceStart,1e-6f) );
				obj->receiveExplosion( source, causedDamage );
			}
		}
	}
	return 0;
}

int GameLevel::script_isActiveCutScene( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns true if cut scene is active.", funcName) );

	vm->pushBoolean( m_cutScene != 0 );
	return 1;
}

int GameLevel::script_playCutScene( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects cut scene lua file name", funcName) );

	skipCutScene();

	String cutSceneName = vm->toString(1);

	P(GameCutScene) cutScene = new GameCutScene( vm, m_arch, m_sceneMgr, m_soundMgr, m_particleMgr, m_projectileMgr );
	cutScene->compile( cutSceneName );
	m_cutScene = cutScene;

	vm->pushTable( m_cutScene );
	return 1;
}

int GameLevel::script_loadProjectiles( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} pre-loads 'count' projectiles to projectilemanager cache, expects projectile source file name and count", funcName) );

	String projectileName = vm->toString(1);
	int count = (int)vm->toNumber(2);

	m_projectileMgr->allocateProjectiles( projectileName, count );
	return 0;
}

int	GameLevel::script_setShadowColor( VM* vm, const char* funcName )
{
	float v[4];
	int rv = getParams( vm, funcName, "RGBA 0-255", v, 4 );
	for ( int i = 0 ; i < 4 ; ++i )
		if ( v[i] < 0.f || v[i] > 255.f )
			throw ScriptException( Format("Func {0} expects integer RGBA 0-255", funcName) );
	m_shadowColor = Color( (uint8_t)v[0], (uint8_t)v[1], (uint8_t)v[2], (uint8_t)v[3] );
	return rv;
}

int GameLevel::script_setBackgroundToCells( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_TABLE};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} background (BACKGROUND tagged) object name and table of cell names", funcName) );

	String	backgroundName	= vm->toString(1);
	Table	cellNames		= vm->toTable(2);

	// find background object
	P(Node) background = 0;
	for ( int i = 0 ; i < m_backgrounds.size() ; ++i )
	{
		if ( m_backgrounds[i]->name() == backgroundName )
		{
			background = m_backgrounds[i];
			break;
		}
	}
	if ( !background )
		throw ScriptException( Format("{0}: Cannot find background object {1} (no BACKGROUND tag?)", funcName, backgroundName) );

	// set background object to cells
	for ( int i = 1 ; !cellNames.isNil(i) ; ++i )
	{
		String cellName = cellNames.getString(i);
		GameCell* cell = getCell(cellName);
		cell->setBackground( background );
	}
	return 0;
}

int GameLevel::script_createFlareSet( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects flare image name, flare world diameter, fade time, max count and cell name", funcName) );
	
	int param = 1;
	String imageName = vm->toString( param++ );
	float worldSize = vm->toNumber( param++ );
	float fadeTime = vm->toNumber( param++ );
	int maxFlares = (int)vm->toNumber( param++ );
	String cellName = vm->toString( param++ );

	P(GameFlareSet) flareSet = new GameFlareSet( vm, m_arch, imageName, worldSize, fadeTime, maxFlares );
	flareSet->setPosition( getCell(cellName), Vector3(0,0,0) );

	m_flareSetList.add( flareSet );
	vm->pushTable( flareSet );
	return 1;
}

int GameLevel::script_endLevel( VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} ends active level", funcName) );

	m_levelEnded = true;
	return 0;
}

int GameLevel::script_skipCutScene( VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} skips any active cut scene", funcName) );

	if ( m_cutScene )
	{
		m_cutScene->skip();
		m_cutScene = 0;
	}
	return 0;
}
