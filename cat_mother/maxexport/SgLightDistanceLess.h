#ifndef _SGLIGHTDISTANCELESS_H
#define _SGLIGHTDISTANCELESS_H


/**
 * Compares nodes by tempDistance member.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SgLightDistanceLess
{
public:
	bool operator()( SgLight* a, SgLight* b ) const
	{
		return a->tempDistance < b->tempDistance;
	}
};


#endif // _SGLIGHTDISTANCELESS_H
