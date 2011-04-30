/*
 * Dummy test file for regression test framework.
 * Run 'collect' to get /projects/test_*.cpp recursively to the project.
 */
#include <tester/Test.h>

//-----------------------------------------------------------------------------

static int test()
{
	return 0;
}

//-----------------------------------------------------------------------------

TEST_REG(test);
