#ifndef _GAMECONTROLLER_H
#define _GAMECONTROLLER_H


#include "GameScriptable.h"
#include "ControlSet.h"
#include "CollisionInfo.h"
#include <id/InputDriver.h>
#include <id/InputDevice.h>
#include <util/Vector.h>
#include <math/Vector3.h>
#include <math/Vector2.h>


namespace util {
	class ExProperties; }

class GameCamera;
class GamePointObject;
class GameCharacter;
class ArcBallCamera;


/** 
 * Maps user input to game actions. 
 * @author Toni Aittoniemi, Jani Kajala (jani.kajala@helsinki.fi)
 */
class GameController :
	public GameScriptable
{
public:
	enum Constants
	{
		/** Controller prop version. */
		VERSION = 11
	};

	GameController( script::VM* vm, io::InputStreamArchive* arch, id::InputDriver* inputDriver, util::ExProperties* gameconfig );
	~GameController();

	void			update( float dt );
	void			focusLost();
	void			flushDevices();
					
	void			resetDevices();
	void			setDevicesToPoll();
	ControlSet&		getControlSet( int index );
	void			configureAction( int index );
	void			selectActiveControlSet( int index );
					
	void			loadConfiguration();
	void			saveConfiguration();
					
	void			controlArcBallCamera( GameCharacter* character, GameCamera* camera, float dt, float targetHeight, float rotationSpeed, float dollySpeed, float dollyMin, float dollyMax, float tiltMax );
	void			controlCutScene( GameCamera* camera, float dt );
	void			controlCharacter( GameCharacter* character, GameCamera* camera, float dt );
	void			resetInputState( GameCharacter* player );
	void			resetCrosshairPos();
	
	int				controlSets() const;
	bool			configuringInput() const;
	int				activeControlSet() const;
	bool			controllersDirty() const;
	math::Vector3	crosshairPos() const;
	lang::String	activeCameraName() const;
	
	/** Returns result of last aim collision check. */
	const CollisionInfo&	aimCollisionInfo() const;

private:
	int										m_methodBase;
	static ScriptMethod<GameController>		sm_methods[];

	P(id::InputDriver)						m_inputDriver;
	P(util::ExProperties)					m_cfg;
	util::Vector<int>						m_devicesToPoll;
	P(util::ExProperties)					m_controllerCfg;
											
	int										m_activeControlSet;
	util::Vector<ControlSet>				m_controlSets;			// TODO : Make to a hash table of <string, ControlSet>
	bool									m_configuringInput;
	int										m_configuringAction;
	bool									m_controllersDirty;
	bool									m_setHasJoystickControls;
	bool									m_keyboardRunToggle;
	
	// is true if input state needs to be reset
	bool									m_needToResetInputState;
											
	// crosshair			
	/** Ratio of relative axis coordinates to crosshair position. */			
	float									m_relativeAxisSensitivity;
	/** Set to true when crosshair needs to be reset. */
	bool									m_needToResetCrosshairPos;
	/** Crosshair position on screen, range ( -1..1, -1..1, 0 ) */
	math::Vector3							m_crosshairPos;
	/** Crosshair position target, range ( -1..1, -1..1, 0 ) */
	math::Vector3							m_crosshairShouldBeAtPos;

	CollisionInfo							m_aimCollisionInfo;

	// Crosshair averaging 

	/** Storage for crosshair positions on last iterations. */
	util::Vector<math::Vector2>				m_crosshairAverageBuffer;

	// Used for raytracing crosshair target position
	P(GamePointObject)						m_collisionTester;

	// Key stroke buffer		
	util::Vector<id::InputDevice::Event>	m_keyStrokes;

	void					processCrosshairInputEvents( GameCamera* camera, int devindex, const id::InputDevice::Event& event );
	void					getTurningStrengthFromCrosshairPos( GameCamera* camera, float* turnLeft, float* turnRight, float* pitch, math::Vector2* normalizedCrosshair );
	void					updateCrosshairPos( GameCamera* camera );
	void					applyCrosshairPos( GameCharacter* character, GameCamera* camera, const math::Vector3& crosshairPosDelta, float dt );
	void					addCrosshairAveragePos( const math::Vector2& val );
	math::Vector2			getAverageCrosshairPos();

	void					addSetAndAction( const lang::String& set, const lang::String& action, const lang::String& device, unsigned int code );
	GameAction::ActionType	getActionType( int deviceindex, int eventCode );
	int						getInputDevice( const lang::String& device ) const;
	float					exponentialize( float val, float exponentiality ) const;			// readjusts a number in range 0..1 to 2^(x * amount) in range 0..1, only accepts positive exponentials
	float					exponentializeOneOverX( float val, float exponentiality ) const;	// readjusta a number in range 0..1 to exp( 1 / (-0.125 - x * amount) ) in range 0..1 , only accepts negative exponential 

	int						methodCall( script::VM* vm, int i );
	int						script_hasJoystickControls( script::VM* vm, const char* funcName );

	GameController( const GameController& );
	GameController& operator=( const GameController& );
};


#endif // _GAMECONTROLLER_H
