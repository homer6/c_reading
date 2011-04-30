#include <lang/Object.h>
#include <tester/Test.h>
#include <assert.h>

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

static int test()
{
	Object a;
	P(Object) b = new Object;
	P(Object) c = new Object;
	Object d;
	return 0;
}

//-----------------------------------------------------------------------------

TEST_REG(test);
