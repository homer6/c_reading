#include <lang/Object.h>
#include <tester/Test.h>
#include <assert.h>

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace testref
{


class A : public Object
{
};

class B : public A
{
};

class C : public A
{
};


} using namespace testref;

//-----------------------------------------------------------------------------

static int test()
{
	P(B) b = new B;
	P(A) a = b;
	assert( a.ptr() == b );
	assert( !(a.ptr() != b) );
	if ( a.ptr() == b )
		a = new C;
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
