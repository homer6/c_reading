#ifndef _GAMEOBJECTLISTITEM_H
#define _GAMEOBJECTLISTITEM_H


class GameObject;
class GameObjectList;


/** 
 * Linked list item of GameObjects. 
 * @see GameObjectList
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GameObjectListItem
{
public:
	GameObjectListItem()									: m_next(0), m_obj(0) {}

	explicit GameObjectListItem( GameObject* obj )			: m_next(0), m_obj(obj) {}

	~GameObjectListItem();

	/** Returns next object in the list. */
	GameObjectListItem*		next() const					{return m_next;}

	/** Returns object associated with this item. */
	GameObject*				object() const					{return m_obj;}

private:
	friend class GameObjectList;

	GameObjectListItem*		m_next;
	GameObject*				m_obj;
};


#endif // _GAMEOBJECTLISTITEM_H
