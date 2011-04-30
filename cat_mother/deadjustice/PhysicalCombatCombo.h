#ifndef _PHYSICALCOMBATCOMBO_H
#define _PHYSICALCOMBATCOMBO_H

#include <lang/Object.h>
#include <lang/String.h>
#include <util/Vector.h>


/**
 * @author Toni Aittoniemi
 */
class PhysicalCombatCombo :
	public lang::Object
{
public:
	/** Create a new empty combo */
	PhysicalCombatCombo();

	/** Add move to combo */
	void				addMove( const lang::String& move );

	/** Advance current move and return next move name */
	const lang::String& advanceAndGetNext();
	
	/** Set current move  */
	void				setMoveIndex( int i );

	/** Returns number of moves */
	int					moves() const;

	/** Returns move name by index */
	const lang::String& move( int i ) const;

private:
	util::Vector<lang::String>	m_sequence;
	lang::String				m_emptyString;
	int							m_currentMove;
	
	PhysicalCombatCombo( const PhysicalCombatCombo& other );
	PhysicalCombatCombo& operator=( const PhysicalCombatCombo& other );
};

#endif