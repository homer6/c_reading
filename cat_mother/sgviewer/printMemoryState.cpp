#include "StdAfx.h"
#include <lang/Array.h>
#include <lang/Debug.h>
#include <mem/Group.h>
#include <algorithm>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

class MemoryGroup
{
public:
	int			bytesInUse;
	int			blocksInUse;
	int			bytesTotal;
	int			blocksTotal;
	const char*	name;

	MemoryGroup() :
		m_group(0), 
		bytesInUse(0), 
		blocksInUse(0), 
		bytesTotal(0),
		blocksTotal(0),
		name("")
	{
	}

	explicit MemoryGroup( void* group ) :
		m_group( mem_Group_copy(group) ), 
		bytesInUse( mem_Group_bytesInUse(group) ), 
		blocksInUse( mem_Group_blocksInUse(group) ), 
		bytesTotal( mem_Group_bytesTotal(group) ), 
		blocksTotal( mem_Group_blocksTotal(group) ), 
		name( mem_Group_name(group) )
	{
	}

	MemoryGroup( const MemoryGroup& other ) :
		m_group( mem_Group_copy(other.m_group) ), 
		bytesInUse( other.bytesInUse ), 
		blocksInUse( other.blocksInUse ),
		bytesTotal( other.bytesTotal ), 
		blocksTotal( other.blocksTotal ),
		name( other.name )
	{
	}

	~MemoryGroup()
	{
		mem_Group_release(m_group);
	}

	MemoryGroup& operator=( const MemoryGroup& other )
	{
		void* group = mem_Group_copy( other.m_group );
		mem_Group_release( m_group );
		m_group = group;
		bytesInUse = other.bytesInUse; 
		blocksInUse = other.blocksInUse;
		bytesTotal = other.bytesTotal;
		blocksTotal = other.blocksTotal;
		name = other.name;
		return *this;
	}

	/** Bigger blocks come first. */
	bool operator<( const MemoryGroup& other ) const
	{
		return bytesInUse > other.bytesInUse;
	}

private:
	void*	m_group;
};

//-----------------------------------------------------------------------------

void printMemoryState()
{
	const String commonPrefix = "f:/projects/";

	// count groups
	int n = 0;
	void* group;
	for ( group = mem_Group_first() ; group ; group = mem_Group_next(group) )
		++n;

	// reserve array for groups
	Array<MemoryGroup> stats;
	stats.setSize( n ); 
	stats.clear();

	// list and sort groups
	int bytes = 0;
	int blocks = 0;
	for ( group = mem_Group_first() ; group ; group = mem_Group_next(group) )
	{
		MemoryGroup mg( group );
		bytes += mg.bytesInUse;
		blocks += mg.blocksInUse;
		stats.add( mg );
	}
	std::sort( stats.begin(), stats.end() );

	// print groups
	Debug::println( "Memory state: Total {0,#} bytes in {1,#} blocks", bytes, blocks );
	for ( int i = 0 ; i < stats.size() ; ++i )
	{
		const MemoryGroup& mg = stats[i];
		String name = String(mg.name).toLowerCase().replace( '\\', '/' );
		
		if ( name.startsWith(commonPrefix) )
			name = name.substring( commonPrefix.length() );

		Debug::println( "  {0}: uses {1,#} bytes in {2,#} blocks, total {3,#} bytes in {4,#} blocks", name, mg.bytesInUse, mg.blocksInUse, mg.bytesTotal, mg.blocksTotal );
	}
}
