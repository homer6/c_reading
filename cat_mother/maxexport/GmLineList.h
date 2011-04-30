#ifndef _GMLINELIST_H
#define _GMLINELIST_H


#include <lang/Object.h>
#include <util/Vector.h>
#include <math/Vector3.h>


/**
 * List of lines.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GmLineList :
	public lang::Object
{
public:
	util::Vector<math::Vector3>	points;

	GmLineList();

	bool	operator==( const GmLineList& other ) const;
	bool	operator!=( const GmLineList& other ) const;
};


#endif // _GMLINELIST_H
