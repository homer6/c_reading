#ifndef _CONTROLSET_H
#define _CONTROLSET_H


#include "GameAction.h"
#include <lang/String.h>
#include <util/Vector.h>


namespace id {
	class InputDriver;}

namespace util {
	class ExProperties;}


/**
 * @author Toni Aittoniemi
 */
class ControlSet 
{
public:
	ControlSet( id::InputDriver* inputDriver, util::ExProperties* cfg );
	ControlSet();
	~ControlSet();

	void				initializeByDevice( int index );
	void				addAction( const GameAction& val );
	GameAction&			getAction( int index );
	GameAction&			getConfigurableAction( int index );
	void				setName( const lang::String& name );

	int					actions() const;
	int					configurableActions() const;
	int					configurableActionIndexIs( int index ) const;
	const GameAction&	getAction( int index ) const;
	const lang::String&	name() const;
	const bool			hasJoystickControls() const;

private:
	P(id::InputDriver)			m_inputDriver;
	P(util::ExProperties)		m_cfg;
	lang::String				m_name;
	util::Vector<GameAction>	m_actions;
		
};


#endif //_CONTROLSET_H
