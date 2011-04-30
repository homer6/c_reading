#ifndef _DX8INPUTDRIVER_H
#define _DX8INPUTDRIVER_H


#include "Array.h"
#include "Dx8InputDevice.h"
#include <id/InputDriver.h>
#include <lang/Object.h>
#include <dinput.h>


/**
 * @author Toni Aittoniemi
 */
class Dx8InputDriver :
	public id::InputDriver
{
public:
	Dx8InputDriver();
	~Dx8InputDriver();

	void						addReference();
	void						release();
	int							create();
	void						destroy();
	void						refreshAttachedInputDevices();
	void						focusLost();
	int							attachedInputDevices() const;	
	id::InputDevice*			getAttachedInputDevice( int i ) const;
	bool						attachedInputDevicesDirty() const;

	Array<char*>&				getFriendlyDeviceNames();
	int&						getEnumRunningIndex();

private:
	long						m_refs;
	IDirectInput8*				m_di;
	Array<P(Dx8InputDevice)>	m_devices;
	HWND						m_windowhandle;
	mutable bool				m_attachedDirty;

	int							m_enumdevicesindex;
	Array<char*>				m_friendlyDeviceNames;

	static BOOL CALLBACK	DIListAttachedInputDevicesCallback( LPCDIDEVICEINSTANCE desc, LPVOID ref );
	static BOOL CALLBACK	DICreateAttachedInputDevicesCallback( LPCDIDEVICEINSTANCE di, LPVOID ref );
	
	Dx8InputDriver( const Dx8InputDriver& );
	Dx8InputDriver& operator=( const Dx8InputDriver& );

};


#endif // _DX8INPUTDRIVER_H
