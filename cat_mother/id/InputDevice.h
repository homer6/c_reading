#ifndef _ID_INPUTDEVICE_H
#define _ID_INPUTDEVICE_H


namespace id
{


/**
 * Input device wrapper.
 * @author Toni Aittoniemi, Jani Kajala (jani.kajala@helsinki.fi)
 */
class InputDevice
{
public:
	/** Masks for different types of event codes. */
	enum EventMasks
	{
		EVENT_AXIS_MASK			= 0x000000ff,
		EVENT_AXIS_BITOFFSET	= 0,
		EVENT_BUTTON_MASK		= 0x0000ff00,
		EVENT_BUTTON_BITOFFSET	= 8,
		EVENT_KEY_MASK			= 0x00ff0000,
		EVENT_KEY_BITOFFSET		= 16,
		EVENT_HAT_MASK			= 0x0f000000,
		EVENT_HAT_BITOFFSET		= 24,
		EVENT_RELAXIS_MASK		= 0xf0000000,
		EVENT_RELAXIS_BITOFFSET	= 28,
	};

	struct Event
	{
		/** Event code. */
		int		code;

		/** Event value. */
		float	value;
	};

	/** Types of controllers. */
	enum DeviceType
	{
		TYPE_UNSUPPORTED,
		TYPE_KEYBOARD,
		TYPE_MOUSE,
		TYPE_JOYSTICK,
		TYPE_GAMEPAD,
		TYPE_FLIGHT,
	};

	/** Increments reference count by one. */
	virtual void			addReference() = 0;

	/** Decrements reference count by one and releases the object if no more references left. */
	virtual void			release() = 0;

	/** Polls input device, stores events in buffer. */
	virtual void			poll() = 0;
	
	/** Empties event buffer. */
	virtual void			flushEvents() = 0;
	
	/** Signals the device that app has lost focus. */
	virtual void			focusLost() = 0;
	
	/** Retrieves input device type. */
	virtual DeviceType		deviceType() const = 0;

	/** Retrieves input device ascii name. */
	virtual const char*		name() const = 0;

	/** Sets deadzone (range[0..1]) for an event. */
	virtual void			setDeadZone( int eventCode, float amount ) = 0;
	
	/** Enumerates device type & objects (results in re-mapping of action codes). */
	virtual void			enumerateDeviceObjects() = 0;

	/** Return true if device objects have changed. */
	virtual bool			objectsDirty() const = 0;

	/** Returns deadzone for an event. */
	virtual float			deadZone( int eventCode ) const = 0;

	/** Retrieves count of events in buffer. */
	virtual int				events() const = 0;

	/** Retrieves a single event. */
	virtual void			getEvent( int index, Event* data ) = 0;
	
	/** Retrieves count of different event codes the input device can produce. */
	virtual int				eventCodeCount() const = 0;

	/** Retrieves ascii description of an event code. */
	virtual const char*		getEventCodeName( int code ) = 0;
	
	/** Retrieves the code of an event by index. */
	virtual unsigned int	getEventCode( int index ) const = 0;

	/** Returns number of axes in device. */
	virtual int				axes() const = 0;

	/** Returns number of buttons in device. */
	virtual int				buttons() const = 0;

protected:
	InputDevice() {}
	virtual ~InputDevice() {}

private:
	InputDevice( const InputDevice& );
	InputDevice& operator=( const InputDevice& );
};


} // id


#endif // _ID_INPUTDEVICE_H
