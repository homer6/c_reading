#ifndef _ID_INPUTDRIVER_H
#define _ID_INPUTDRIVER_H


namespace id
{


class InputDevice;


/**
 * Used for enumerating attached input devices.
 * @author Toni Aittoniemi, Jani Kajala (jani.kajala@helsinki.fi)
 */
class InputDriver
{
public:
	enum Constants
	{
		VERSION = 4
	};

	/** Increments reference count by one. */
	virtual void			addReference() = 0;

	/** Decrements reference count by one and releases the object if no more references left. */
	virtual void			release() = 0;

	/** Creates driver object */
	virtual int				create() = 0;

	/** Destroys driver object */
	virtual void			destroy() = 0;

	/** 
	 * Updates list of attached input devices. 
	 * Warning: Very expensive on Windows 2000.
	 */
	virtual void			refreshAttachedInputDevices() = 0;

	/** Tells drives that application has lost focus */
	virtual void			focusLost() = 0;

	/** Returns number of attached input devices. */
	virtual int				attachedInputDevices() const = 0;	

	/** 
	 * Returns ith attached input device.
	 * @see attachedInputDevices
	 */
	virtual InputDevice*	getAttachedInputDevice( int i ) const = 0;

	/** 
	 * Returns true if list of attached devices is not up-to-date anymore. 
	 * Warning: Very expensive on Windows 2000.
	 */
	virtual bool			attachedInputDevicesDirty() const = 0;

protected:
	InputDriver() {}
	virtual ~InputDriver() {}

private:
	InputDriver( const InputDriver& );
	InputDriver& operator=( const InputDriver& );
};


typedef InputDriver* (*createInputDriverFunc)();
typedef int	(*getInputDriverVersionFunc)();


} // id


#endif // _ID_INPUTDRIVER_H
