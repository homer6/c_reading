#include "StdAfx.h"
#include "DrvObject.h"
#include "config.h"

//-----------------------------------------------------------------------------

#define synchronized( OBJ ) CriticalSectionLock lock( &(OBJ).mutex )

//-----------------------------------------------------------------------------

class DrvObjectList
{
public:
	CRITICAL_SECTION mutex;

	DrvObjectList();
	~DrvObjectList();

	void		insert( DrvObject* item );
	void		remove( DrvObject* item );
	DrvObject*	begin();

private:
	DrvObject*	m_begin;

	DrvObjectList( const DrvObjectList& );
	DrvObjectList& operator=( const DrvObjectList& );
};

class CriticalSectionLock
{
public:
	CriticalSectionLock( CRITICAL_SECTION* cs )			: m_cs(cs) {EnterCriticalSection(m_cs);}
	~CriticalSectionLock()								{LeaveCriticalSection(m_cs);}

private:
	CRITICAL_SECTION* m_cs;
};

//-----------------------------------------------------------------------------

DrvObjectList::DrvObjectList()
{
	m_begin = 0;
	InitializeCriticalSection( &mutex );
}

DrvObjectList::~DrvObjectList()
{
	DeleteCriticalSection( &mutex );
}

void DrvObjectList::insert( DrvObject* item )
{
	synchronized( *this );
	
	item->m_next = m_begin;
	m_begin = item;
}

void DrvObjectList::remove( DrvObject* item )
{
	synchronized( *this );

	if ( m_begin == item )
	{
		m_begin = item->m_next;
		item->m_next = 0;
	}
	else
	{
		DrvObject* list = m_begin;
		while ( list && list->m_next != item )
			list = list->m_next;

		if ( list )
		{
			list->m_next = item->m_next;
			item->m_next = 0;
		}
	}
}

DrvObject* DrvObjectList::begin()
{
	synchronized( *this );
	return m_begin;
}

//-----------------------------------------------------------------------------

static DrvObjectList	s_objects;

//-----------------------------------------------------------------------------

DrvObject::DrvObject()
{
	s_objects.insert( this );
}

DrvObject::~DrvObject()
{
	s_objects.remove( this );
}

int DrvObject::objects()
{
	synchronized( s_objects );

	int count = 0;

	DrvObject* item = s_objects.begin();
	while ( item )
	{
		++count;
		item = item->m_next;
	}

	return count;
}

void DrvObject::deleteAll()
{
	DrvObject* item = s_objects.begin();
	while ( item )
	{
		delete item;
		item = s_objects.begin();
	}
}
