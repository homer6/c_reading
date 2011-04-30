#include "CryptUtil.h"
#include "internal/random.h"
#include <lang/Array.h>
#include <io/File.h>
#include "internal/config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;

//-----------------------------------------------------------------------------

namespace crypt
{


static String cryptString( String inputStr )
{
	return /*String("CM_") + */inputStr;
	// cannot be used -- messes up extension based image loader...
	/*inputStr = inputStr.toLowerCase();
	Array<Char,256> outputStr( inputStr.length()+1 );
	int i = 0;
	for ( ; i < inputStr.length() ; ++i )
	{
		Char c = inputStr.charAt(i);
		if ( c >= 'a' && c <= 'z' )
		{
			c += 13;
			if ( c > 'z' )
				c -= 'z' - 'a';
		}
		outputStr[i] = c;
	}
	outputStr[i] = 0;
	String str = outputStr.begin();
	return str;*/
}

//-----------------------------------------------------------------------------

String CryptUtil::cryptFileName( const String& inputName )
{
	int index = inputName.lastIndexOf('.');
	if ( index >= 0 )
	{
		String str = File(inputName).getParent();
		if ( str.length() > 0 )
			str = str + File::separator;
		return str + cryptString( File(inputName).getName() );
	}
	return inputName;
}

void CryptUtil::cryptBuffer( const uint8_t* in, uint8_t* out, long c, long offset )
{
	for ( long i = 0 ; i < c ; ++i )
	{
		uint8_t c = in[i];
		uint8_t x = ~((c-73) ^ s_cryptRandoms[offset % CRYPT_RANDOM_COUNT]);
		uint8_t x0 = (~x ^ s_cryptRandoms[offset % CRYPT_RANDOM_COUNT]) + 73;
		assert( x0 == c );
		out[i] = x;
		++offset;
	}
}

void CryptUtil::decryptBuffer( const uint8_t* in, uint8_t* out, long c, long offset )
{
	for ( long i = 0 ; i < c ; ++i )
	{
		uint8_t x = in[i];
		uint8_t c = (~x ^ s_cryptRandoms[offset % CRYPT_RANDOM_COUNT]) + 73;
		out[i] = c;
		++offset;
	}
}


} // crypt
