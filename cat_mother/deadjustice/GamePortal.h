#ifndef _GAMEPORTAL_H
#define _GAMEPORTAL_H


#include <lang/Object.h>
#include <math/Vector3.h>
#include <math/Vector4.h>


class GameCell;


/** 
 * Portals connect cells.
 * @see GameCell
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GamePortal :
	public lang::Object
{
public:
	/** Portal constants. */
	enum Constants
	{
		/** Number of corners in portal. */
		NUM_CORNERS = 4,
	};

	/** Constructs a physical portal leading to a Cell. */
	GamePortal( const math::Vector3* corners, GameCell* target );

	/** 
	 * Checks position & delta against portal plane.
	 * @param position Vector3
	 * @param delta Vector3
	 * @param distance [out] Receives distance to portal if intersection. (optional)
	 * @return true if position + delta led from one side to the other, false otherwise ( for example : position and delta lie in portal plane ) 
	 */
	bool isCrossing( const math::Vector3& position, const math::Vector3& delta ) const;

	/** Returns true if point is on positive side of the portal. */
	bool isOnPositiveSide( const math::Vector3& position ) const;

	/** Returns target cell. */
	GameCell*	target() const;

	/** Copies 4 corners to supplied pointer. */
	void		getCorners( math::Vector3* corners ) const;

	/** Returns normal of the portal plane. */
	const math::Vector3&	normal() const											{return m_normal;}

	/** Returns portal plane. */
	const math::Vector4&	plane() const											{return m_plane;}

private:
	math::Vector3	m_corners[NUM_CORNERS];
	math::Vector4	m_plane;
	math::Vector3	m_normal;
	GameCell*		m_target;

	GamePortal();
	GamePortal( const GamePortal& other );
	GamePortal& operator=( const GamePortal& other );
};


#endif // _GAMEPORTAL_H
