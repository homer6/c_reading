#include "StdAfx.h"
#include "Dx8InputDriver.h"
#include "error.h"
#include "toString.h"
#include <id/Errors.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

extern HANDLE g_module;

//-----------------------------------------------------------------------------

// TODO : BUG : Positive axis controller message is sometimes lost, why ?? 

Dx8InputDriver::Dx8InputDriver() :
	m_refs(0),
	m_di(0)
{
	m_friendlyDeviceNames.setSize(10);
		
	for ( int i = 0; i < 10; ++i ) 
		m_friendlyDeviceNames[i] = new char[256];
}

Dx8InputDriver::~Dx8InputDriver()
{
	destroy();

	for ( int i = 0; i < m_friendlyDeviceNames.size(); ++i )
		delete[] m_friendlyDeviceNames[i];
}
	
int	Dx8InputDriver::create()
{
	/* Store window handle */
	m_windowhandle = GetActiveWindow();
	if ( !m_windowhandle )
		return id::ERROR_NOACTIVEWINDOW;
	
	/* Create DirectInput Object */
	HRESULT errval = DirectInput8Create((HINSTANCE)g_module, DIRECTINPUT_VERSION, (REFIID)IID_IDirectInput8, (void**)&m_di, NULL);

	if (errval != DI_OK)
	{
		error( "DirectInput8Create error: %s", toString(errval) );
		return id::ERROR_GENERIC;
	}

	refreshAttachedInputDevices();

	return id::ERROR_NONE;
}

void Dx8InputDriver::destroy()
{
	m_devices.clear();
	assert( DrvObject::objects() == 0 );

	if ( m_di ) 
		m_di->Release();
	
	m_di = 0;
}

void Dx8InputDriver::addReference()
{
	InterlockedIncrement( &m_refs );
}

void Dx8InputDriver::release()
{
	if ( 0 == InterlockedDecrement( &m_refs ) )
		delete this;
}

void Dx8InputDriver::refreshAttachedInputDevices() 
{
	if ( attachedInputDevicesDirty() )
	{
		m_devices.clear();

		m_enumdevicesindex = 0;

		/* Enumerate devices */
		HRESULT errval = m_di->EnumDevices( DI8DEVCLASS_ALL, DICreateAttachedInputDevicesCallback, this, DIEDFL_ATTACHEDONLY );

		if (errval != DI_OK)
			error( "DirectInput8 EnumDevices error: %s", toString(errval) );
	}
}

void Dx8InputDriver::focusLost()
{
	for ( int i = 0 ; i < m_devices.size() ; ++i ) 
		m_devices[i]->focusLost();
}

int Dx8InputDriver::attachedInputDevices() const
{
	return m_devices.size();
}

id::InputDevice* Dx8InputDriver::getAttachedInputDevice( int i ) const
{
	if ( ( i >= 0) && ( i < m_devices.size() ) )
		return m_devices[i];
	else 
		return 0;
}

bool Dx8InputDriver::attachedInputDevicesDirty() const
{
	m_attachedDirty = false;

	/* List DIDEVICEINSTANCE for attached devices */
	Array<DIDEVICEINSTANCE> attachedDevices;
	HRESULT errval = m_di->EnumDevices( DI8DEVCLASS_ALL, DIListAttachedInputDevicesCallback, &attachedDevices, DIEDFL_ATTACHEDONLY );
	if (errval != DI_OK)
		error( "DirectInput8 EnumDevices error: %s", toString(errval) );

	// check that no devices have been added
	for ( int i = 0 ; i < attachedDevices.size() ; ++i )
	{
		DIDEVICEINSTANCE* desc = &attachedDevices[i];
		unsigned int devcategory = (desc->dwDevType & 0xff);
		Dx8InputDevice::DeviceType type = Dx8InputDevice::convertDi8DevType( devcategory );

		if ( Dx8InputDevice::TYPE_UNSUPPORTED != type )
		{
			bool found = false;
			for ( int i = 0 ; i < m_devices.size() ; ++i )
			{
				P(Dx8InputDevice) device = m_devices[i];

				if ( !memcmp( device->guid(), &desc->guidInstance, sizeof(GUID) ) )
				{
					found = true;
					break;
				}
			}

			if ( !m_attachedDirty )
				m_attachedDirty = !found;
		}
	}

	// check that no devices have been removed
	for ( int i = 0 ; i < m_devices.size() ; ++i )
	{
		P(Dx8InputDevice) device = m_devices[i];

		bool found = false;
		for ( int i = 0 ; i < attachedDevices.size() ; ++i )
		{
			DIDEVICEINSTANCE* desc = &attachedDevices[i];
			if ( !memcmp( device->guid(), &desc->guidInstance, sizeof(GUID) ) )
			{
				found = true;
				break;
			}
		}

		if ( !m_attachedDirty )
			m_attachedDirty = !found;
	}

	return m_attachedDirty;
}

Array<char*>& Dx8InputDriver::getFriendlyDeviceNames() 
{
	return m_friendlyDeviceNames;
}

int& Dx8InputDriver::getEnumRunningIndex() 
{
	return m_enumdevicesindex;
}

BOOL CALLBACK Dx8InputDriver::DIListAttachedInputDevicesCallback( LPCDIDEVICEINSTANCE desc, LPVOID ref )
{
	Array<DIDEVICEINSTANCE>* attachedDevices = reinterpret_cast<Array<DIDEVICEINSTANCE>*>( ref );
	attachedDevices->add( *desc );
	return DIENUM_CONTINUE;
}

BOOL CALLBACK Dx8InputDriver::DICreateAttachedInputDevicesCallback( LPCDIDEVICEINSTANCE desc, LPVOID ref )
{
	Dx8InputDriver* inputDrv = reinterpret_cast<Dx8InputDriver*>( ref );
	IDirectInput8* di = inputDrv->m_di;
	unsigned int devcategory = (desc->dwDevType & 0xff);
	Dx8InputDevice::DeviceType type = Dx8InputDevice::convertDi8DevType( devcategory );
	static int deviceIndex = 0;

	if ( Dx8InputDevice::TYPE_UNSUPPORTED != type )
	{
		/* Create Input Driver */
		IDirectInputDevice8* dxdevice;
		HRESULT errval = di->CreateDevice(desc->guidInstance, &dxdevice, NULL);
		
		if (errval != DI_OK)
		{
			error( "CreateDevice() error: %s", toString(errval) );
			return DIENUM_CONTINUE;
		}

		/* Create Input Driver Interface */

		int& enumindex = inputDrv->getEnumRunningIndex();
		Array<char*>& friendlyDeviceNames = inputDrv->getFriendlyDeviceNames();

		if ( ++enumindex > (friendlyDeviceNames.size() - 1) )
			friendlyDeviceNames.add( new char[256] );

		/** Use input device product name as friendly device name. */
		char indexString[16]; memset( indexString, 0, 16 );
		itoa( enumindex, indexString, 10 );
		strcpy( friendlyDeviceNames[enumindex], (char*)&desc->tszProductName );

		/* Append enumeration index to the name. */
		/*strcat( friendlyDeviceNames[enumindex], "(" );
		strcat( friendlyDeviceNames[enumindex], indexString );
		strcat( friendlyDeviceNames[enumindex], ")" );*/
		
		P(Dx8InputDevice) device = new Dx8InputDevice( desc->guidInstance, dxdevice, type, friendlyDeviceNames[enumindex], inputDrv->m_windowhandle );
		inputDrv->m_devices.add(device);

		dxdevice->Release();
		dxdevice = 0;
	}

	return DIENUM_CONTINUE;
}

