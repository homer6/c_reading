-- Finite State Machine test

true = 1
false = nil

function this.base( diz )
	trace( "Creating base" )
	local state = {}
	state.name = "base"
	state.enterCondition = function( this ) 
								return true 
							end
	state.enter = function( this ) 
						trace( "Entering base state" ) 
					end
	state.updateInterval = 0.5
	state.update = function( this ) 
						trace( "Updating base state" ) 
					end
	state.exitCondition = function( this ) 
							return false
						end
	state.exit = function( this ) 
					trace( "Exiting base state" ) 
				end

	return state;
end

function this.interrupt( diz )
	trace( "Creating interrupt" )
	local state = {}
	state.name = "interrupt"
	state.enterCondition = function( this ) if ( %diz.enterFlag == 1 ) then return true else return false end end
	state.enter = nil --function( this ) trace( "Entering interrupt state" ) end
	state.updateInterval = 0.2
	state.update = function( this ) trace( "Updating interrupt state" ) end
	state.exitCondition = function( this ) if ( %diz.baseExitFlag == 1 ) then return true else return false end end
	state.exit = function( this ) trace( "Exiting interrupt state" ) end

	return state;
end

function this.nonreachable( diz )
	trace( "Creating nonreachable" )
	local state = {}
	state.name = "nonreachable"
	state.enterCondition = function( this ) return false end
	state.enter = function( this ) trace( "Non reachable state" ) end
	state.updateInterval = 1
	state.update = function( this ) trace( "Non reachable state" ) end
	state.exitCondition = function( this ) return true end
	state.exit = function( this ) trace( "Non reachable state" ) end

	return state;
end

function this.init( this )
	this:removeStates()
	this:addState( this:base() )
	this:addState( this:interrupt() )
	this:addState( this:nonreachable() )

	this.enterFlag = 0
	this.baseExitFlag = 0

	local get_state_test = this:getState( "base" );
	trace( get_state_test.name.." state update interval = "..get_state_test.updateInterval );
end
