#include "StdAfx.h"
#include "GmLineList.h"

//-----------------------------------------------------------------------------

using namespace math;
using namespace util;
using namespace lang;

//-----------------------------------------------------------------------------

GmLineList::GmLineList() :
	points( Allocator<Vector3>(__FILE__) )
{
}

bool GmLineList::operator==( const GmLineList& other ) const
{
	if ( points.size() != other.points.size() )
		return false;
	for ( int i = 0 ; i < points.size() ; ++i )
		if ( points[i] != other.points[i] )
			return false;
	return true;
}

bool GmLineList::operator!=( const GmLineList& other ) const
{
	if ( points.size() != other.points.size() )
		return true;
	for ( int i = 0 ; i < points.size() ; ++i )
		if ( points[i] != other.points[i] )
			return true;
	return false;
}
