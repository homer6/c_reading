#ifndef _REQUIRE_H
#define _REQUIRE_H

void internalError( const char* fname, int line, const char* expr );

/** 
 * An assertion that is left to release build. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
#define require( EXPR ) if (EXPR) {} else {internalError(__FILE__,__LINE__,#EXPR);}

#endif // _REQUIRE_H
