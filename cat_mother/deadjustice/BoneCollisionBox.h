#ifndef _BONEBOUNDINGBOX_H
#define _BONEBOUNDINGBOX_H


#include <lang/Object.h>
#include <math/Vector3.h>


namespace sg {
	class Node;}

namespace lang {
	class String;}

namespace math {
	class Matrix4x4;}


/** 
 * Used for bone specific collision checking. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class BoneCollisionBox
{
public:
	BoneCollisionBox();

	/** Computes bounding box for a bone. */
	BoneCollisionBox( sg::Node* root, sg::Node* bone, float damageMultiplier );

	/** Merges point to this box. */
	void					mergePoint( const math::Vector3& point );

	/** Returns bone node. */
	sg::Node*				bone() const;

	/** Sets box dimensions. */
	void					setBox( const math::Vector3& boxMin, const math::Vector3& boxMax );

	/** Finds line-box intersection if any. */
	bool					findLineBoxIntersection( const math::Vector3& start, const math::Vector3& delta, float* t ) const;

	/** Returns bounding box maximum dimensions. */
	const math::Vector3&	boxMax() const;

	/** Returns bounding box minimum dimensions. */
	const math::Vector3&	boxMin() const;

	/** Returns bone damage multiplier. */
	float					damageMultiplier() const;

	/** Returns true if box has bone. */
	bool					hasBone() const;

private:
	P(sg::Node)		m_bone;
	math::Vector3	m_max;
	math::Vector3	m_min;
	float			m_damageMultiplier;
};


#endif // _BONEBOUNDINGBOX_H
