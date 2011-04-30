#ifndef _SGU_LINELISTUTIL_H
#define _SGU_LINELISTUTIL_H


namespace sg {
	class LineList;}

namespace pix {
	class Color;}

namespace math {
	class Vector3;
	class Matrix4x4;}


namespace sgu
{


/** 
 * Common line list primitive helper functions. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class LineListUtil
{
public:
	/** Adds wireframe box to line list. Requires that vertices are locked. */
	static void		addLineBox( sg::LineList* lines, 
						const math::Matrix4x4& tm, const math::Vector3& dim, const pix::Color& color );
};


} // sgu


#endif // _SGU_LINELISTUTIL_H
 