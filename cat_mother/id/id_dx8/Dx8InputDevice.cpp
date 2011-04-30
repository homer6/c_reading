#include "StdAfx.h"
#include "error.h"
#include "dikeytab.h"
#include "toString.h"
#include "Dx8InputDevice.h"
#include <math.h>
#include <stdio.h>
#include "config.h"

//-----------------------------------------------------------------------------

static bool s_axisdoubling = true;

//-----------------------------------------------------------------------------

Dx8InputDevice::Dx8InputDevice( const GUID& guidInstance, IDirectInputDevice8* didevice, DeviceType type, char* name, HWND windowhandle ) :
	m_refs(0),
	m_type(type),
	m_device(didevice),
	m_buffersize(50),
	m_lastbufferoverflowed(false)
{
	m_device->AddRef();

	/* Get Device Caps */
	m_diDevCaps.dwSize = sizeof(DIDEVCAPS);
	m_device->GetCapabilities( &m_diDevCaps );
	message( "Initialized input device with %i axes and %i buttons", m_diDevCaps.dwAxes, m_diDevCaps.dwButtons );

	/* Allocate buffer for device data */
	m_eventcount	= m_buffersize;
	m_currentevents.setSize(m_eventcount);

	/* Set product name with ' ' replaced by '_' */
	strcpy( m_name, name );
	char* c = 0;
	while ( 0 != ( c = strchr( m_name, ' ' ) ) ) 
		c[0] = '_';

	/* Store GUID */
	memcpy( &m_guid, &guidInstance, sizeof(GUID) );

	/* Enumerate device objects */
	enumerateDeviceObjects();

	/* Set Device Properties */	

	DIPROPDWORD prop;
	prop.diph.dwSize		= sizeof(DIPROPDWORD);
	prop.diph.dwHeaderSize	= sizeof(DIPROPHEADER);
	prop.diph.dwObj			= 0;
	prop.diph.dwHow			= DIPH_DEVICE;
	prop.dwData				= m_buffersize;
	
	HRESULT errval = m_device->SetProperty( DIPROP_BUFFERSIZE, (DIPROPHEADER*)&prop );

	if ( ( errval != DI_OK ) && ( errval != DI_PROPNOEFFECT ) )
	{
		error( "SetProperty() error: %s", toString(errval) );
		return;
	}

	/* Set cooperative level */

	if ( type == TYPE_KEYBOARD )
		errval = m_device->SetCooperativeLevel( windowhandle, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND );
	else
		errval = m_device->SetCooperativeLevel( windowhandle, DISCL_EXCLUSIVE | DISCL_FOREGROUND );
	
	if ( errval != DI_OK )
	{
		error( "SetCooperativeLevel() error: %s", toString(errval) );
		return;
	}

}

void Dx8InputDevice::enumerateDeviceObjects() 
{
	HRESULT errval;
	DeviceType type = m_type;

	errval = m_device->Unacquire();
	if ( errval != DI_OK )
		error( "Unacquire() error: %s", toString(errval) );

	/* Reset data */
	m_dataformat.setSize(0);
	m_codes.setSize(0);
	
	for ( int i = 0; i < (int)m_eventcount; ++i )
	{
		m_currentevents[i].dwOfs = 0;
		m_currentevents[i].dwData = 0;
		m_currentevents[i].dwTimeStamp = 0;
		m_currentevents[i].dwSequence = 0;
		m_currentevents[i].uAppData = 0;
	}

	/* Setup of directional types of controllers */
	if ( type == TYPE_GAMEPAD || type == TYPE_JOYSTICK || type == TYPE_FLIGHT || type == TYPE_MOUSE )
	{
		/* Don't double axes for mice */
		if ( type == TYPE_MOUSE)
			s_axisdoubling = false;
		else
			s_axisdoubling = true;

		/* Get Device Caps */
		m_diDevCaps.dwSize = sizeof(DIDEVCAPS);
		m_device->GetCapabilities( &m_diDevCaps );

		/* Enumerate device objects */

		errval = m_device->EnumObjects( DIEnumDeviceObjectsCallback, this, DIDFT_ABSAXIS | DIDFT_RELAXIS | DIDFT_BUTTON | DIDFT_POV );

		if ( errval != DI_OK )
		{
			error( "EnumObjects() error : %s!", toString(errval) );
		}

		/* Create array of DIOBJECTDATAFORMAT structures */

		Array<DIOBJECTDATAFORMAT>	objectdataformat(m_dataformat.size());

		for ( int i = 0 ; i < m_dataformat.size() ; ++i ) 
			objectdataformat[i] = m_dataformat[i].format;

		/* Set Data Format */

		DIDATAFORMAT devicedataformat;

		devicedataformat.dwSize		= sizeof(DIDATAFORMAT);
		devicedataformat.dwObjSize	= sizeof(DIOBJECTDATAFORMAT);
		devicedataformat.dwFlags	= 0;
		devicedataformat.dwNumObjs	= m_dataformat.size();
		devicedataformat.dwDataSize = 4 * m_dataformat.size();
		devicedataformat.rgodf		= objectdataformat.begin();

		errval = m_device->SetDataFormat(&devicedataformat);

		if ( errval != DI_OK )
		{
			error( "SetDataFormat() error : %s!", toString(errval) );
		}

		/* Get Axis Ranges */

		for ( int i = 0 ; i < m_dataformat.size() ; ++i )
		{
			if ( ( m_dataformat[i].format.dwType & DIDFT_RELAXIS ) || ( m_dataformat[i].format.dwType & DIDFT_ABSAXIS ) )
			{
				DIPROPRANGE range;
				range.diph.dwSize		= sizeof(DIPROPRANGE);
				range.diph.dwHeaderSize	= sizeof(DIPROPHEADER);
				range.diph.dwObj		= i * 4;
				range.diph.dwHow		= DIPH_BYOFFSET;
				
				errval = m_device->GetProperty( DIPROP_RANGE, (DIPROPHEADER*)&range); 

				if ( ( errval != DI_OK ) && ( errval != S_FALSE ) )
				{
					error( "GetProperty() error: %s", toString(errval) );
				}
				else
				{
					m_dataformat[i].range[0] = range.lMin;
					m_dataformat[i].range[1] = range.lMax;
				}
			}
		}
		/* Set spacing of data blocks in data format to 4 bytes */
		m_dataformatalignment = 2;

	/* End setup of directional type controllers */
	}
	/* Setup of keyboard type controllers */
	if ( type == TYPE_KEYBOARD )
	{
		/* Set data format */
		for ( int i = 0; i < 256; i++ )
		{
			ObjectDataFormat	objectdata;

			objectdata.guid		= 0;
			sprintf(objectdata.name, "KEY_%s", s_diKeyCodeToName[i]);
			objectdata.range[0]	= 0;
			objectdata.range[1]	= 128;
			objectdata.isdoubledto[0] = m_codes.size() + 1;
			objectdata.isdoubledto[1] = 0;

			m_dataformat.add(objectdata);
		
			CodeMap				code;

			code.dataOffset		= i;
			sprintf(code.name, "KEY_%s", s_diKeyCodeToName[i]);
			code.code = i << EVENT_KEY_BITOFFSET;
			code.sign = 1;

			m_codes.add(code);
		}
		
		errval = m_device->SetDataFormat(&c_dfDIKeyboard);

		if ( errval != DI_OK )
		{
			error( "SetDataFormat() error : %s!", toString(errval) );
		}

		/* Set spacing of data blocks in data format to 1 byte */
		m_dataformatalignment = 0;
	/* End setup of keyboard type controllers */
	}

	/* Init relative axis data */
	m_relaxislastpos.setSize( 16 );
	m_relaxisFirstIteration.setSize( 16 );
	for ( int i = 0; i < 16; ++i )
	{
		m_relaxislastpos[i] = 0;
		m_relaxisFirstIteration[i] = true;
	}

	/* Clear dead zones. */
	m_deadZones.setSize( 0 );
}

Dx8InputDevice::~Dx8InputDevice()
{
	HRESULT errval = m_device->Unacquire();
	if ( errval != DI_OK )
		error( "Unacquire() error: %s", toString(errval) );
	
	m_device->Release();

	for ( int i = 0 ; i < m_dataformat.size() ; ++i ) 
		delete m_dataformat[i].guid;

}

void Dx8InputDevice::addReference()
{
	InterlockedIncrement( &m_refs );
}

void Dx8InputDevice::release()
{
	if ( 0 == InterlockedDecrement( &m_refs ) )
		delete this;
}

void Dx8InputDevice::poll() 
{
	/* Acquire device */
	HRESULT errval = m_device->Acquire();

	if ( ( errval != DI_OK ) && ( errval != S_FALSE ) )
	{
		m_eventcount = 0;
		error( "device Acquire() error: %s", toString(errval) );
		return;
	}

	/* Set relative axis data last positions */ 
	if ( !m_lastbufferoverflowed )
	{
		for ( int i = 0; i < m_currentevents.size(); i++ )
		{	
			int formatindex = m_currentevents[i].dwOfs >> m_dataformatalignment;
				int code = m_codes[m_dataformat[formatindex].isdoubledto[0] - 1].code;
				if ( code & EVENT_RELAXIS_MASK )
				{
					int axis = code >> EVENT_RELAXIS_BITOFFSET;
					m_relaxislastpos[axis] = m_currentevents[i].dwData;
					m_relaxisFirstIteration[axis] = false;
				}
		}
	}

	/* Get Buffered Data */
	m_eventcount = m_buffersize;
	m_currentevents.setSize(m_buffersize);
	errval = m_device->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), m_currentevents.begin(), &m_eventcount, 0 );

	if ( ( errval != DI_OK ) && ( errval != DI_BUFFEROVERFLOW ) )
	{
		error( "GetDeviceData() error: %s", toString(errval) );
		return;
	}
	else 
	if ( errval == DI_BUFFEROVERFLOW )
	{
		error ( "device Buffer Overflow, some input lost" );
		m_lastbufferoverflowed = true;
	}
	else
	{
		m_lastbufferoverflowed = false;
	}

	/* Set event buffer size to actual event count */
	m_currentevents.setSize( m_eventcount );
	int itemsinbuffer = m_eventcount;

	/* Apply double events to doubled controls */
	for ( int i = 0; i < itemsinbuffer; i++ )
	{
		ObjectDataFormat& objdata = m_dataformat[m_currentevents[i].dwOfs >> m_dataformatalignment];
		float mid = float(objdata.range[1] + objdata.range[0]) / 2.f;
		if ( objdata.isdoubledto[1] )
		{
			DIDEVICEOBJECTDATA extradata( m_currentevents[i] );

			if ( float(m_currentevents[i].dwData) <= mid ) 
				extradata.dwData = (int)floor(mid + 1.f);
			else
				extradata.dwData = (int)floor(mid);

			m_currentevents.add( ++i, extradata );
			++itemsinbuffer;
			++m_eventcount;
		}
	}
}
			
void Dx8InputDevice::flushEvents() 
{
	/* poll input buffer and trash the results */	
	poll();
	m_eventcount = 0;
	m_currentevents.setSize( 0 );
}

void Dx8InputDevice::focusLost()
{
	/* Flush events */
	flushEvents();

	/* Unaquire device */
	m_device->Unacquire();
}

int Dx8InputDevice::events() const 
{
	return m_eventcount;
}

void Dx8InputDevice::getEvent( int index, Event* data ) 
{
	/* Translate event data : retrieve event code from format -> insert data */
	int formatindex = m_currentevents[index].dwOfs >> m_dataformatalignment;
	if (formatindex < m_dataformat.size() )
	{
		if ( m_dataformat[formatindex].isdoubledto[1])
		{
			float val = ((float)m_currentevents[index].dwData - (float)m_dataformat[formatindex].range[0]) / float( m_dataformat[formatindex].range[1] - m_dataformat[formatindex].range[0] );
			val *= 2;

			if ( val < 1.f )
			{
				data->code	= m_codes[m_dataformat[formatindex].isdoubledto[0] - 1].code;
				data->value = 1.f - val;
			}
			else
			{
				data->code	= m_codes[m_dataformat[formatindex].isdoubledto[1] - 1].code;
				data->value = val - 1.f;
			}
		}
		else
		{
			data->code	= m_codes[m_dataformat[formatindex].isdoubledto[0] - 1].code;
			if ( data->code & EVENT_AXIS_MASK )
			{
				data->value = ((float)m_currentevents[index].dwData - (float)m_dataformat[formatindex].range[0]) / float( m_dataformat[formatindex].range[1] - m_dataformat[formatindex].range[0] );
			}
			else if ( data->code & EVENT_BUTTON_MASK )
			{
				data->value = float( m_currentevents[index].dwData / ( m_dataformat[formatindex].range[1] - m_dataformat[formatindex].range[0] ) );
			}
			else if ( data->code & EVENT_KEY_MASK )
			{
				data->value = float( m_currentevents[index].dwData / ( m_dataformat[formatindex].range[1] - m_dataformat[formatindex].range[0] ) );
			}
			else if ( data->code & EVENT_RELAXIS_MASK )
			{
				int axis = data->code >> EVENT_RELAXIS_BITOFFSET;

				if ( m_relaxislastpos[axis] > m_currentevents[index].dwData )
					data->value = -float( m_relaxislastpos[axis] - m_currentevents[index].dwData ); 
				else
					data->value = float( m_currentevents[index].dwData - m_relaxislastpos[axis] );

				// when m_currentevent[index].dwData > 2^31 & m_relaxislastpos < 2^31 -> wrap around.
				if ( m_currentevents[index].dwData > 2147483647 && m_relaxislastpos[axis] < 2147483647 )
					data->value = -float( 0xffffffff - m_currentevents[index].dwData + m_relaxislastpos[axis] );
				// the other way round
				if ( m_currentevents[index].dwData < 2147483647 && m_relaxislastpos[axis] > 2147483647 )
					data->value = float( 0xffffffff - m_relaxislastpos[axis] + m_currentevents[index].dwData );

				// Remove first iteration error ( m_relaxislastpos is not yet valid )
				if ( m_relaxisFirstIteration[axis] )
					data->value = 0.f;
			}
		}

		// Adjust value for deadzone
		int match = findDeadZoneEntry( data->code );
		if ( match != -1 )
		{
			if ( data->value > m_deadZones[match].amount )
			{
				float scale = 1.f / (1.f - m_deadZones[match].amount);				
				data->value = ( data->value - m_deadZones[match].amount ) * scale;
			}
			else
			{
				data->value = 0.f;
			}
		}
	}
}

const char* Dx8InputDevice::getEventCodeName( int code ) 
{
	for ( int i = 0 ; i < m_codes.size() ; ++i )
		if ( m_codes[i].code == (unsigned int)code ) 
			return m_codes[i].name;

	assert( false ); // event code not found
	return 0;
}

bool Dx8InputDevice::objectsDirty() const 
{
	DIDEVCAPS oldcaps;
	DIDEVCAPS curcaps;
	curcaps.dwSize = sizeof(DIDEVCAPS);
	m_device->GetCapabilities( &curcaps );

	memcpy( &oldcaps, &m_diDevCaps, sizeof(DIDEVCAPS) );

	// DEBUG :Don't compare flags
	oldcaps.dwFlags = 0;
	curcaps.dwFlags = 0;

	if ( memcmp( &oldcaps, &curcaps, sizeof(DIDEVCAPS) ) != 0 )
		return true;
	else
		return false;
}

int	Dx8InputDevice::eventCodeCount() const
{
	return m_codes.size();
}

unsigned int Dx8InputDevice::getEventCode( int index ) const
{
	return m_codes[index].code;
}

const char*	Dx8InputDevice::name() const 
{
	return m_name;
}

int Dx8InputDevice::mappedAxes() const
{
	int counter = 0;
	for ( int i = 0 ; i < m_codes.size() ; ++i )
	{
		if ( m_codes[i].code & EVENT_AXIS_MASK || m_codes[i].code & EVENT_RELAXIS_MASK ) 
			counter++;
	}
	return counter;
}

Dx8InputDevice::DeviceType Dx8InputDevice::deviceType() const
{
	return m_type;
}

BOOL CALLBACK Dx8InputDevice::DIEnumDeviceObjectsCallback( LPCDIDEVICEOBJECTINSTANCE doi, LPVOID ref ) 
{
	Dx8InputDevice* device = reinterpret_cast<Dx8InputDevice*>(ref);

	struct ObjectDataFormat objectdata;
	struct CodeMap			codemap;
	
	/* Generate event code */
	codemap.code = 0;	
	unsigned int type = doi->dwType & 0xff;

	switch(type)
	{
	case DIDFT_RELAXIS:
		codemap.code = device->mappedAxes() + 1;

		/* Check for overflow */
		if ( codemap.code > 0xf )
			return DIENUM_CONTINUE;

		codemap.code <<= EVENT_RELAXIS_BITOFFSET;
		codemap.dataOffset = device->m_dataformat.size();
		sprintf( codemap.name, "%s", doi->tszName );
		codemap.sign = 1;

		device->m_codes.add(codemap);
			
		objectdata.isdoubledto[0] = device->m_codes.size();
		objectdata.isdoubledto[1] = 0;
		break;	
	case DIDFT_ABSAXIS:
//	case DIDFT_AXIS:
		if ( s_axisdoubling )
		{
			/* Set code for Negative value */
			codemap.code = device->mappedAxes() + 1;
			
			/* Check for overflow */
			if ( codemap.code > 0xff )
				return DIENUM_CONTINUE;

			codemap.code <<= EVENT_AXIS_BITOFFSET;		
			codemap.dataOffset = device->m_dataformat.size();
			sprintf( codemap.name, "%s_NEG", doi->tszName );
			codemap.sign = -1;

			device->m_codes.add(codemap);
			
			objectdata.isdoubledto[0] = device->m_codes.size();
			
			/* Set code for Positive value */
			codemap.code = device->mappedAxes() + 1;
			
			/* Check for overflow */
			if ( codemap.code > 0xff )
				return DIENUM_CONTINUE;

			codemap.code <<= EVENT_AXIS_BITOFFSET;		
			codemap.dataOffset = device->m_dataformat.size();
			sprintf( codemap.name, "%s_POS", doi->tszName );
			codemap.sign = 1;
			
			device->m_codes.add(codemap);

			objectdata.isdoubledto[1] = device->m_codes.size();
		}
		else
		{
			codemap.code = device->mappedAxes() + 1;
			
			/* Check for overflow */
			if ( codemap.code > 0xff )
				return DIENUM_CONTINUE;

			codemap.code <<= EVENT_AXIS_BITOFFSET;
			codemap.dataOffset = device->m_dataformat.size();
			sprintf( codemap.name, "%s", doi->tszName );
			codemap.sign = 1;

			device->m_codes.add(codemap);
			
			objectdata.isdoubledto[0] = device->m_codes.size();
			objectdata.isdoubledto[1] = 0;
		}
		break;	
	case DIDFT_PSHBUTTON:
	case DIDFT_TGLBUTTON:
	case DIDFT_BUTTON:
		codemap.code = DIDFT_GETINSTANCE(doi->dwType) + 1;
		
		/* Check for overflow */
		if ( codemap.code > 0xff )
			return DIENUM_CONTINUE;

		codemap.code <<= EVENT_BUTTON_BITOFFSET;
		codemap.dataOffset = device->m_dataformat.size();
		sprintf( codemap.name, "%s", doi->tszName );
		codemap.sign = 1;
		
		device->m_codes.add(codemap);

		objectdata.isdoubledto[0] = device->m_codes.size();
		objectdata.isdoubledto[1] = 0;

		objectdata.range[0] = 0;
		objectdata.range[1] = 128;
		break;
	case DIDFT_POV:
		codemap.code = DIDFT_GETINSTANCE(doi->dwType) + 1;
		
		/* Check for overflow */
		if ( codemap.code > 0xf )
			return DIENUM_CONTINUE;

		codemap.code <<= EVENT_HAT_BITOFFSET;					
		sprintf( codemap.name, "%s", doi->tszName );
		codemap.sign = 1;

		device->m_codes.add(codemap);
		
		objectdata.isdoubledto[0] = device->m_codes.size();
		objectdata.isdoubledto[1] = 0;

		objectdata.range[0] = 0;
		objectdata.range[1] = 0xffff;
		break;
	
	}

	/* Save name */

	memcpy( objectdata.name, doi->tszName, MAX_PATH );

	 /* Save GUID */

	objectdata.guid = new GUID(doi->guidType);

	/* Create DIOBJECTDATAFORMAT from DIDEVICEOBJECTINSTANCE */

	objectdata.format.dwOfs		= device->m_dataformat.size() * 4;
	objectdata.format.pguid		= objectdata.guid;
	objectdata.format.dwType	= doi->dwType;
	objectdata.format.dwFlags	= 0;

	/* Store objectdataformat */

	device->m_dataformat.add(objectdata);

	return DIENUM_CONTINUE;
}

const GUID* Dx8InputDevice::guid() const
{
	return &m_guid;
}

Dx8InputDevice::DeviceType Dx8InputDevice::convertDi8DevType( unsigned int devcategory )
{
	DeviceType type = TYPE_UNSUPPORTED;
	switch (devcategory)
	{
	case DI8DEVTYPE_1STPERSON:
		/* TODO */
		break;
	case DI8DEVTYPE_DEVICE:
		break;
	case DI8DEVTYPE_DEVICECTRL:
		break;
	case DI8DEVTYPE_DRIVING:
		break;
	case DI8DEVTYPE_FLIGHT:
		type = TYPE_FLIGHT;
		break;
	case DI8DEVTYPE_JOYSTICK:
		type = TYPE_JOYSTICK;
		break;
	case DI8DEVTYPE_GAMEPAD:
		type = TYPE_GAMEPAD;
		break;
	case DI8DEVTYPE_KEYBOARD:
		type = TYPE_KEYBOARD;
		break;
	case DI8DEVTYPE_MOUSE:
		type = TYPE_MOUSE;
		break;
	case DI8DEVTYPE_REMOTE:
		break;
	case DI8DEVTYPE_SCREENPOINTER:
		break;
	case DI8DEVTYPE_SUPPLEMENTAL:
		break;
	}
	return type;
}

int Dx8InputDevice::findDeadZoneEntry( const int& eventcode ) const 
{
	for ( int i = 0; i < m_deadZones.size(); ++i )
		if ( m_deadZones[i].code == eventcode )
			return i;

	return -1;
}

void Dx8InputDevice::setDeadZone( int eventCode, float amount ) 
{
	int match = findDeadZoneEntry( eventCode );
		
	if ( match == -1 )
	{
		// Create new entry
		m_deadZones.add( DeadZoneEntry( eventCode, amount ) );
	}
	else
	{
		// Modify existing entry
		m_deadZones[match].amount = amount;
	}
}

float Dx8InputDevice::deadZone( int eventCode ) const 
{
	int match = findDeadZoneEntry( eventCode );
		
	if ( match != -1 )
		return m_deadZones[match].amount;
	else
		return 0.f;
}

int Dx8InputDevice::axes() const
{
	return m_diDevCaps.dwAxes;
}

int Dx8InputDevice::buttons() const
{
	return m_diDevCaps.dwButtons;
}
