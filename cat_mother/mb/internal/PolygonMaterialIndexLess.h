#include <mb/Polygon.h>


namespace mb
{


/**
 * Compares polygon material indices.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class PolygonMaterialIndexLess
{
public:
	bool operator()( const Polygon* a, const Polygon* b )
	{
		return a->material() < b->material();
	}
};


} // mb
