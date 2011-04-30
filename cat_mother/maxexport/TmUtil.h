#include <math/Matrix4x4.h>


/** 
 * Utilities for processing 3DS Max transformations. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class TmUtil
{
public:
	/** 
	 * Returns left-handed model-to-parent transformation. 
	 * @param node Source node for transformation.
	 * @param t Time of transformation.
	 */
	static math::Matrix4x4	getModelToParentLH( INode* node, TimeValue t );

	/** Returns transform in left-handed coordinate system. */
	static math::Matrix4x4	toLH( const Matrix3& tm );

	/** 
	 * Returns object pivot transformation.
	 * Transform vertices with (pivot*convm) to get left-handed coordinates
	 * which have melted pivot transform. Transforming those
	 * with getModelToParentLH() will result coordinates in parent space.
	 */
	static Matrix3		getPivotTransform( INode* node );

	/** Returns true if the matrix has negative scaling. */
	static bool			hasNegativeParity( const Matrix3& m );

	/** Prints matrix to debug output using specified margin. */
	static void			println( const Matrix3& tm, int margin=0 );
};
