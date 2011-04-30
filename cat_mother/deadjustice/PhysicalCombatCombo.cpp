#include "PhysicalCombatCombo.h"
#include "config.h"

//-----------------------------------------------------------------------------

PhysicalCombatCombo::PhysicalCombatCombo() :
	m_sequence( util::Allocator< lang::String >(__FILE__,__LINE__) ),
	m_emptyString(""),
	m_currentMove(0)
{			
}

void PhysicalCombatCombo::addMove( const lang::String& move )
{ 
	m_sequence.add( move ); 
}	

const lang::String& PhysicalCombatCombo::advanceAndGetNext()						
{ 
	m_currentMove++; 
	return m_currentMove < m_sequence.size() ? m_sequence[m_currentMove] : m_emptyString; 
} 

void PhysicalCombatCombo::setMoveIndex( int i )
{ 
	m_currentMove = i; 
}

int PhysicalCombatCombo::moves() const
{ 
	return m_sequence.size(); 
}

const lang::String& PhysicalCombatCombo::move( int i ) const
{ 
	return m_sequence[i]; 
}

