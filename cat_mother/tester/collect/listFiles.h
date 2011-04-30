#include <lang/String.h>
#include <util/Vector.h>


/**
 * Lists all files beginning with <prefix> and ending with <suffix>, recursively.
 */
void listFiles( const lang::String& path, 
	const lang::String& prefix, const lang::String& suffix, 
	util::Vector<lang::String>& files );
