#ifndef _GAMEOBJECTLIST_H
#define _GAMEOBJECTLIST_H


class GameObjectListItem;


/** 
 * Linked list of GameObjects. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GameObjectList
{
public:
	GameObjectList();
	~GameObjectList();

	void				insert( GameObjectListItem* item );
	void				remove( GameObjectListItem* item );
	GameObjectListItem*	begin() const;

private:
	GameObjectListItem*	m_begin;

	GameObjectList( const GameObjectList& );
	GameObjectList& operator=( const GameObjectList& );
};


#endif // _GAMEOBJECTLIST_H
