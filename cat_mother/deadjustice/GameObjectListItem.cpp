#include "GameObjectListItem.h"
#include "config.h"

//-----------------------------------------------------------------------------

GameObjectListItem::~GameObjectListItem()
{
	m_next = 0;
	m_obj = 0;
}
