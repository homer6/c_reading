#include "Profile.h"
#include <lang/Object.h>
#include <util/Vector.h>
#include <assert.h>
#include <string.h>

#if defined(_DEBUG) && defined(WIN32) && defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <crtdbg.h>
#endif

#include "config.h"

//-----------------------------------------------------------------------------

using namespace util;

//----------------------------------------------------------------------------

namespace dev
{


/**
 * Profiling info implementation.
 */
class Profile::BlockInfoImpl : 
	public Profile::BlockInfo
{
public:
	BlockInfoImpl*		m_next;
	int					m_refs;
	char				m_name[64];
	TimeStamp			m_ticks;
	int					m_count;

	BlockInfoImpl()																
	{
		m_next = 0;
	}

	void create( const char* name )
	{
		m_refs	= 1;
		strncpy( m_name, name, sizeof(m_name)-1 );
		m_name[sizeof(m_name)-1] = 0;
		m_ticks = TimeStamp(0,0);
		m_count = 1;
	}

	double			time() const													{return m_ticks.seconds();}
	int				count() const													{return m_count;}
	const char*		name() const													{return m_name;}

private:
	BlockInfoImpl( const BlockInfoImpl& );
	BlockInfoImpl& operator=( const BlockInfoImpl& );
};

//----------------------------------------------------------------------------

static class Profile::ProfileStaticData :
	public lang::Object						// for synchronization
{
public:
	ProfileStaticData() :
		Object( OBJECT_INITMUTEX ),
		m_free( Allocator<BlockInfoImpl*>(__FILE__,__LINE__) ),
		m_blockInfoAlloc(__FILE__,__LINE__)
	{
		m_active = 0;

		// enable exit-time leak check
		#if defined(_DEBUG) && defined(_MSC_VER) && defined(WIN32)
		_CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF );
		#endif
	}

	~ProfileStaticData()
	{
		reset();

		synchronized( this );
		while ( m_free.size() > 0 )
		{
			m_blockInfoAlloc.deallocate( m_free[m_free.size()-1], 1 );
			m_free.setSize( m_free.size()-1 );
		}
	}

	BlockInfoImpl* enterBlock( const char* name )
	{
		synchronized( this );

		// try to find existing block
		BlockInfoImpl* info;
		if ( m_active )
		{
			for ( info = m_active ; info ; info = info->m_next )
			{
				if ( !strcmp(info->m_name,name) )
				{
					info->m_count++;
					info->m_refs++;

					assert( info->m_refs > 0 );
					return info;
				}
			}
		}

		// create block
		if ( 0 == m_free.size() )
		{
			info = m_blockInfoAlloc.allocate(1);
		}
		else
		{
			info = m_free[m_free.size()-1];
			m_free.setSize( m_free.size()-1 );
		}
		info->create( name );

		// link to list of active blocks
		info->m_next = m_active;
		m_active = info;

		assert( 1 == info->m_refs );
		return info;
	}

	void leaveBlock( BlockInfoImpl* info, TimeStamp ticks )
	{
		synchronized( this );

		info->m_ticks += ticks; 
		info->m_refs--;

		assert( info->m_refs >= 0 );
	}

	void reset()
	{
		synchronized( this );

		// discard all info blocks
		if ( m_active )
		{
			BlockInfoImpl* nextInfo;
			for ( BlockInfoImpl* info = m_active ; info ; info = nextInfo )
			{
				// reset() can't be called while profiling is in progress
				assert( 0 == info->m_refs );

				nextInfo = info->m_next;
				info->m_next = 0;
				m_free.add( info );
			}
			m_active = 0;
		}
	}

	int	count()
	{
		synchronized( this );

		int count = 0;
		for ( BlockInfoImpl* info = m_active ; info ; info = info->m_next )
			++count;

		return count;
	}

	Profile::BlockInfo* first()
	{
		return m_active ? static_cast<BlockInfoImpl*>(m_active) : 0;
	}

	Profile::BlockInfo* getNext( const Profile::BlockInfo* next )
	{
		assert( next );

		const BlockInfoImpl* nextImpl = static_cast<const BlockInfoImpl*>(next);
		if ( nextImpl )
			return static_cast<BlockInfoImpl*>( nextImpl->m_next );
		else
			return 0;
	}

private:
	BlockInfoImpl*				m_active;	// List of active profiling blocks.
	Vector<BlockInfoImpl*>		m_free;
	Allocator<BlockInfoImpl>	m_blockInfoAlloc;

} s_data;

//----------------------------------------------------------------------------

bool Profile::sm_enabled = true;

//-----------------------------------------------------------------------------

Profile::Profile( const char* name ) :
	m_info( sm_enabled ? s_data.enterBlock(name) : 0 ),
	m_ticks()
{
}

Profile::~Profile()
{
	if ( m_info )
	{
		TimeStamp ticks;
		ticks -= m_ticks;
		s_data.leaveBlock( m_info, ticks );
	}
}

void Profile::reset()
{
	s_data.reset();
}

int	Profile::count()
{
	return s_data.count();
}

Profile::BlockInfo* Profile::get( int index )
{
	assert( index >= 0 && index < count() );

	synchronized( s_data );

	BlockInfo* it = s_data.first();
	for ( int i = 0 ; i != index && it ; ++i )
	{
		it = s_data.getNext(it);
	}

	assert( it );
	return it;
}

void Profile::setEnabled( bool enabled )
{
	sm_enabled = enabled;
}


} // dev
