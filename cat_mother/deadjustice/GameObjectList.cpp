#include "GameObjectList.h"
#include "GameObjectListItem.h"
#include "config.h"

//-----------------------------------------------------------------------------

GameObjectList::GameObjectList() :
	m_begin(0)
{
}

GameObjectList::~GameObjectList()
{
}

void GameObjectList::insert( GameObjectListItem* item )
{
	item->m_next = m_begin;
	m_begin = item;
}

void GameObjectList::remove( GameObjectListItem* item ) 
{
	if ( m_begin == item )
	{
		m_begin = item->m_next;
		item->m_next = 0;
	}
	else
	{
		GameObjectListItem* list = m_begin;
		while ( list && list->m_next != item )
			list = list->m_next;

		if ( list )
		{
			list->m_next = item->m_next;
			item->m_next = 0;
		}
	}
}

GameObjectListItem*	GameObjectList::begin() const
{
	return m_begin;
}

