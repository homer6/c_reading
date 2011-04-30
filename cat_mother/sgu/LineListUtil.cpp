#include "LineListUtil.h"
#include <sg/LineList.h>
#include <pix/Color.h>
#include <math/Matrix4x4.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace pix;
using namespace math;

//-----------------------------------------------------------------------------

namespace sgu
{


void LineListUtil::addLineBox( LineList* lines, 
	const Matrix4x4& tm, const Vector3& dim, const Color& color )
{
	Matrix3x3 rot = tm.rotation();
	Vector3 c = tm.translation();
	Vector3 x = rot.getColumn(0) * dim[0];
	Vector3 y = rot.getColumn(1) * dim[1];
	Vector3 z = rot.getColumn(2) * dim[2];

	Vector3 boxLines[] =
	{
		c-x-y-z, c+x-y-z,
		c-x-y+z, c+x-y+z,
		c-x+y+z, c+x+y+z,
		c-x+y-z, c+x+y-z,
		
		c-z-y-x, c+z-y-x,
		c-z-y+x, c+z-y+x,
		c-z+y+x, c+z+y+x,
		c-z+y-x, c+z+y-x,

		c-y-x-z, c+y-x-z,
		c-y+x-z, c+y+x-z,
		c-y+x+z, c+y+x+z,
		c-y-x+z, c+y-x+z,
	};

	int boxLineCount = sizeof(boxLines)/sizeof(boxLines[0])/2;
	for ( int i = 0 ; i < boxLineCount ; ++i )
	{
		if ( lines->lines() < lines->maxLines() )
			lines->addLine( boxLines[i*2+0], boxLines[i*2+1], color );
	}
}


} // sgu
