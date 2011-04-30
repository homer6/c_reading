#include <max.h>
#include <assert.h>
#include "require.h"

#include <config_msvc.h>

#undef assert
#define assert( EXPR ) if (EXPR) {} else {internalError(__FILE__,__LINE__,#EXPR);}
