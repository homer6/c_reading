#ifndef _GAMECELL_H
#define _GAMECELL_H


#include "GamePortal.h"
#include "GameScriptable.h"
#include "GameObjectList.h"
#include "GameSurface.h"
#include <sg/Mesh.h>
#include <sg/Model.h>
#include <sg/Light.h>
#include <lang/Object.h>
#include <lang/String.h>
#include <util/Vector.h>


namespace bsp {
	class BSPPolygon;
	class BSPNode; }

namespace sg {
	class Camera;
	class Mesh;
	class Light;
	class Node;}

namespace pix {
	class Colorf;}

namespace snd {
	class SoundManager;}

namespace ps {
	class ParticleSystemManager;}

class GameBSPTree;
class GameLevel;
class GameObject;


/** 
 * Every object in a game level is in some cell.
 * There can be many cells in single game level.
 * Cells are connected by portals.
 * @see GamePortal
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GameCell :
	public GameScriptable
{
public:
	GameCell( script::VM* vm, io::InputStreamArchive* arch, 
		snd::SoundManager* soundMgr, ps::ParticleSystemManager* particleMgr,
		const lang::String& name, sg::Node* geometry,
		const lang::String& bspFileName, int bspBuildPolySkip,
		const util::Vector<P(GameSurface)>& collisionMaterialTypes );

	~GameCell();

	/** Returns geometry. */
	sg::Node*			getRenderObject( sg::Camera* camera );

	/** Adds portal to another cell. */
	void				addPortal( GamePortal* portal );

	/** Updates game cell. */
	void				update( float dt );

	/** Sets background object for this cell. */
	void				setBackground( sg::Node* background );

	/** Set true by camera if cell was visible in last rendering. */
	void				setVisible( bool visible )									{m_visible=visible;}

	/** Returns cell name. */
	const lang::String& name() const;

	/** Returns count of portals. */
	int					portals() const;

	/** Returns portal by index. */
	GamePortal*			getPortal( int index ) const;

	/** Returns cell BSP tree. */
	GameBSPTree*		bspTree() const;

	/** Returns iterator to the first object in cell. */
	GameObjectListItem*	objectsInCell() const;

	/** 
	 * Returns mesh, model and triangle index by BSP polygon id. 
	 * @return true if data retrieved successfully.
	 */
	bool				getVisualByBSPPolygonID( int polyid, sg::Mesh** mesh, sg::Model** model, int* triangleIndex, GameSurface** surface ) const;

	/** Returns closest light (if any) in the cell. */
	sg::Light*			getClosestLight( const math::Vector3& point ) const;

	/** Returns number of CASTSHADOW name tagged lights in the cell. */
	int					lights() const;

	/** Returns number of objects in the cell. */
	int					objects() const;

	/** Returns number of objects in the cell which have collidable() flag set (=can be collided against). */
	int					collidableObjects() const;

	/** Returns ith CASTSHADOW name tagged lights in the cell. */
	sg::Light*			getLight( int i ) const;

	/** Returns background object. */
	sg::Node*			background() const;

	/** Returns true if cell was visible in last rendering. */
	bool				visible() const												{return m_visible;}

private:
	friend class GameLevel;
	friend class GameObject;

	lang::String					m_name;
	P(sg::Node)						m_geometry;
	P(snd::SoundManager)			m_soundMgr;
	P(ps::ParticleSystemManager)	m_particleMgr;
	util::Vector<P(GamePortal)>		m_portals;
	P(GameBSPTree)					m_bsptree;
	GameObjectList					m_objectsInCell;
	GameLevel*						m_level;
	util::Vector<P(sg::Light)>		m_lights;
	P(sg::Node)						m_background;
	bool							m_visible;

	void				setShaderParams( sg::Shader* fx, sg::Mesh* mesh, sg::Light* keylight );

	GameCell& operator=( GameCell& other);
	GameCell( const GameCell& other );
};


#endif // _GAMECELL_H
