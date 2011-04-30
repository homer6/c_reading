#include "Character.h"
#include <ctype.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace lang
{


bool Character::isDigit( Char32 cp )
{
	if ( cp < Char32(0x80) )
		return 0 != isdigit( (char)cp );
	else
		return false;
}

bool Character::isLetter( Char32 cp )
{
	if ( cp < Char32(0x80) )
		return 0 != isalpha( (char)cp );
	else
		return false;
}

bool Character::isLetterOrDigit( Char32 cp )
{
	return isLetter(cp) || isDigit(cp);
}

bool Character::isLowerCase( Char32 cp )
{
	if ( cp < Char32(0x80) )
		return 0 != islower( (char)cp );
	else
		return false;
}

bool Character::isUpperCase( Char32 cp )
{
	if ( cp < Char32(0x80) )
		return 0 != isupper( (char)cp );
	else
		return false;
}

bool Character::isWhitespace( Char32 cp )
{
	if ( cp < Char32(0x80) )
		return 0 != isspace( (char)cp );
	else
		return false;
}


} // lang
