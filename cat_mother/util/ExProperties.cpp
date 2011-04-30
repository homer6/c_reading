#include "ExProperties.h"
#include <lang/Float.h>
#include <lang/Integer.h>
#include <lang/Character.h>
#include <lang/NumberFormatException.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace util
{

void ExProperties::setInteger( const lang::String& name, int x ) 
{
	setIntegers( name, &x, 1 );
}

void ExProperties::setFloat( const lang::String& name, float x ) 
{
	setFloats( name, &x, 1 );
}

void ExProperties::setBoolean( const lang::String& name, bool x ) 
{
	setBooleans( name, &x, 1 );
}

void ExProperties::setIntegers( const lang::String& name, 
	const int* buffer, int count ) 
{
	String str = "";
	for ( int i = 0 ; i < count ; ++i )
	{
		str = str + String::valueOf( buffer[i] );
		if ( i+1 < count )
			str = str + " ";
	}
	(*this)[name] = str;
}

void ExProperties::setFloats( const lang::String& name, 
	const float* buffer, int count ) 
{
	String str = "";
	for ( int i = 0 ; i < count ; ++i )
	{
		str = str + String::valueOf( buffer[i] );
		if ( i+1 < count )
			str = str + " ";
	}
	(*this)[name] = str;
}

void ExProperties::setBooleans( const lang::String& name, 
	const bool* buffer, int count ) 
{
	String str = "";
	for ( int i = 0 ; i < count ; ++i )
	{
		str = str + (buffer[i] ? "true" : "false");
		if ( i+1 < count )
			str = str + " ";
	}
	(*this)[name] = str;
}

int ExProperties::getInteger( const lang::String& name ) const
{
	int x = 0;
	getIntegers( name, &x, 1 );
	return x;
}

float ExProperties::getFloat( const lang::String& name ) const
{
	float x = 0.f;
	getFloats( name, &x, 1 );
	return x;
}

bool ExProperties::getBoolean( const lang::String& name ) const
{
	bool x = false;
	getBooleans( name, &x, 1 );
	return x;
}

void ExProperties::getIntegers( const lang::String& name,
	int* buffer, int count ) const
{
	assert( count > 0 );

	try
	{
		String	prop = (*this)[name];
		int		prev = 0;
		int		read = 0;

		for ( int i = 0 ; i <= prop.length() ; ++i )
		{
			if ( i == prop.length() ||
				Character::isWhitespace( prop.charAt(i) ) )
			{
				String substr = prop.substring(prev,i);
				buffer[read++] = Integer::parseInt( substr );
				if ( read == count )
					break;
				prev = i;
			}
		}

		if ( read != count )
			throw NumberFormatException( Format("Failed to parse {0,#} integers from {1}", count, name) );
	}
	catch ( NumberFormatException& )
	{
		throw NumberFormatException( Format("Failed to parse {0,#} integers from {1}", count, name) );
	}
}

void ExProperties::getFloats( const lang::String& name,
	float* buffer, int count ) const
{
	assert( count > 0 );

	try
	{
		String	prop = (*this)[name];
		int		prev = 0;
		int		read = 0;

		for ( int i = 0 ; i <= prop.length() ; ++i )
		{
			if ( i == prop.length() ||
				Character::isWhitespace( prop.charAt(i) ) )
			{
				String substr = prop.substring(prev,i);
				buffer[read++] = Float::parseFloat( substr );
				if ( read == count )
					break;
				prev = i;
			}
		}

		if ( read != count )
			throw NumberFormatException( Format("Failed to parse {0,#} floats from {1}", count, name) );
	}
	catch ( NumberFormatException& )
	{
		throw NumberFormatException( Format("Failed to parse {0,#} floats from {1}", count, name) );
	}
}

void ExProperties::getBooleans( const lang::String& name,
	bool* buffer, int count ) const
{
	assert( count > 0 );

	try
	{
		String	prop = (*this)[name].toLowerCase();
		int		prev = 0;
		int		read = 0;

		for ( int i = 0 ; i <= prop.length() ; ++i )
		{
			if ( i == prop.length() ||
				Character::isWhitespace( prop.charAt(i) ) )
			{
				String substr = prop.substring(prev,i);

				bool value = false;
				if ( substr == "true" )
					value = true;
				else if ( substr == "false" )
					value = false;
				else if ( substr.length() > 0 && (substr.charAt(0) == '1' || substr.charAt(0) == '0') )
					value = (substr.charAt(0) == '1');
				else
					throw NumberFormatException( Format("Failed to parse {0,#} booleans from {1}", count, name) );
					
				buffer[read++] = value;
				if ( read == count )
					break;
				prev = i;
			}
		}

		if ( read != count )
			throw NumberFormatException( Format("Failed to parse {0,#} booleans from {1}", count, name) );
	}
	catch ( NumberFormatException& )
	{
		throw NumberFormatException( Format("Failed to parse {0,#} booleans from {1}", count, name) );
	}
}

void ExProperties::setStrings( const lang::String& name, 
	const String* buffer, int count ) 
{
	String str = "";
	for ( int i = 0 ; i < count ; ++i )
	{
		str = str + buffer[i];
		if ( i+1 < count )
			str = str + " ";
	}
	(*this)[name] = str;
}

void ExProperties::getStrings( const lang::String& name,
	String* buffer, int count ) const
{
	assert( count > 0 );

	try
	{
		String	prop = (*this)[name];
		int		prev = 0;
		int		read = 0;

		for ( int i = 0 ; i <= prop.length() ; ++i )
		{
			if ( i == prop.length() ||
				Character::isWhitespace( prop.charAt(i) ) )
			{
				String substr = prop.substring(prev,i).trim();
				buffer[read++] = substr;
				if ( read == count )
					break;
				prev = i;
			}
		}

		if ( read != count )
			throw Exception( Format("Failed to parse {0,#} strings from {1}", count, name) );
	}
	catch ( Exception& )
	{
		throw Exception( Format("Failed to parse {0,#} strings from {1}", count, name) );
	}
}


} // util
