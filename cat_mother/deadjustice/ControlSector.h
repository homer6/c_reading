#ifndef _CONTROLSECTOR_H
#define _CONTROLSECTOR_H


namespace math {
	class Vector3;}


/** 
 * Used to limit movement control vector in certain directions,
 * so that for example character can walk/strafe to left but cannot
 * walk backwards and left.
 * Zero angle of sector is character forward direction
 * and angle increases clockwise.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ControlSector
{
public:
	ControlSector();
	ControlSector( float minAngle, float maxAngle, float controlLimit );

	/** Returns true if specified movement control direction is in the sector. */
	bool	isInSector( const math::Vector3& movementControlVector ) const;

	/** Returns movement vector length limit at this sector. */
	float	controlLimit() const;

private:
	float	m_minAngle;
	float	m_maxAngle;
	float	m_controlLimit;

	/** Remaps input angle to trigonometric circle convention. */
	static float	remapAngle( float angle );
};


#endif // _CONTROLSECTOR_H
