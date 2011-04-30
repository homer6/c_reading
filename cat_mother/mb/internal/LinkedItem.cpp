#include "LinkedItem.h"
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace mb
{


LinkedItem::LinkedItem()
{
	m_next		= 0;
	m_previous	= 0;
}

void LinkedItem::append( LinkedItem* item )
{
	assert( !item->m_next && !item->m_previous );

	item->m_previous	= this;
	item->m_next		= m_next;

	if ( m_next )
		m_next->m_previous = item;
	m_next = item;
}

void LinkedItem::unlink()
{
	if ( m_previous )
		m_previous->m_next = m_next;
	if ( m_next ) 
		m_next->m_previous = m_previous;
	m_previous = m_next = 0;
}

LinkedItem* LinkedItem::next() const
{
	return m_next;
}

LinkedItem* LinkedItem::previous() const
{
	return m_previous;
}


} // mb
