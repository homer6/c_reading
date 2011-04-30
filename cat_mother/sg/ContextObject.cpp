#include "ContextObject.h"
#include <lang/Object.h>
#include <lang/String.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace sg
{


class ContextObjectList :
	public Object				// for synchronization
{
public:
	ContextObjectList();

	void			insert( ContextObject* item );
	void			remove( ContextObject* item );
	ContextObject*	begin();

private:
	ContextObject*	m_begin;

	ContextObjectList( const ContextObjectList& );
	ContextObjectList& operator=( const ContextObjectList& );
};

//-----------------------------------------------------------------------------

ContextObjectList::ContextObjectList() :
	Object( OBJECT_INITMUTEX )
{
	m_begin = 0;
}

void ContextObjectList::insert( ContextObject* item )
{
	synchronized( this );
	
	item->m_next = m_begin;
	m_begin = item;
}

void ContextObjectList::remove( ContextObject* item )
{
	synchronized( this );

	if ( m_begin == item )
	{
		m_begin = item->m_next;
		item->m_next = 0;
	}
	else
	{
		ContextObject* list = m_begin;
		while ( list && list->m_next != item )
			list = list->m_next;

		if ( list )
		{
			list->m_next = item->m_next;
			item->m_next = 0;
		}
	}
}

ContextObject* ContextObjectList::begin()
{
	synchronized( this );
	return m_begin;
}

//-----------------------------------------------------------------------------

static ContextObjectList	s_objects;

//-----------------------------------------------------------------------------

ContextObject::ContextObject()
{
	s_objects.insert( this );
}

ContextObject::~ContextObject()
{
	s_objects.remove( this );
}

void ContextObject::destroy()
{
}

void ContextObject::destroyAll()
{
	P(ContextObject) item = s_objects.begin();
	while ( item )
	{
		P(ContextObject) nextItem;
		{synchronized( s_objects );
		nextItem = item->m_next;}
		item->destroy();
		item = nextItem;
	}
}

void ContextObject::blendState( anim::Animatable** /*anims*/, 
	const float* /*times*/, const float* /*weights*/, int /*n*/ )
{
}


} // sg
