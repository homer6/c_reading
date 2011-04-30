#include <mem/Group.h>
#include <script/VM.h>
#include <script/ClassTag.h>
#include <script/Scriptable.h>
#include <tester/Test.h>
#include <stdio.h>
#include <assert.h>

//-----------------------------------------------------------------------------

using namespace lang;
using namespace script;

//-----------------------------------------------------------------------------

class TestClass :
	public Scriptable
{
public:
	explicit TestClass( VM* vm ) :
		Scriptable( vm, ClassTag<TestClass>::getTag(vm) )
	{
		m_methodBase = addMethod( "hello1" );
		addMethod( "hello2" );
	}

	int methodCall( VM* vm, int i )
	{
		switch ( i-m_methodBase )
		{
		case 0:		return hello1( vm );
		case 1:		return hello2( vm );
		default:	return 0;
		}
	}

	int hello1( VM* vm )
	{
		int n = vm->top();
		printf( "Hello (1)\n" );
		return 0;
	}

	int hello2( VM* vm )
	{
		int n = vm->top();
		assert( 1 == n );
		float x = vm->toNumber( 1 );
		printf( "Hello (2): %g\n", x );
		return 0;
	}

private:
	int	m_methodBase;
};

//-----------------------------------------------------------------------------

static int test()
{
	P(VM) vm = new VM;
	P(TestClass) test = new TestClass( vm );
	test->compileFile( "test.lua" );
	assert( test->hasMethod("f") );
	assert( !test->hasMethod("fuu") );
	test->pushMethod( "f" );
	vm->pushNumber( 2 );
	test->call( 1, 0 );

	// test Table
	Table tab( vm );
	tab.setNumber( "myTest", 123.45f );
	tab.setString( "myTest2", "123.45f" );
	assert( tab.getNumber("myTest") == 123.45f );
	assert( tab.getString("myTest2") == "123.45f" );
	tab = Table();

	printf( "\nGroups:\n" );
	for ( void* group = mem_Group_first() ; group ; group = mem_Group_next(group) )
		printf( "  %s: %i bytes in %i blocks\n", mem_Group_name(group), mem_Group_bytesInUse(group), mem_Group_blocksInUse(group) );
	return 0;
}

//-----------------------------------------------------------------------------

TEST_REG( test );
