#ifndef _DX8INPUTDEVICE_H 
#define _DX8INPUTDEVICE_H 


#include "Array.h"
#include "DrvObject.h"
#include <id/InputDevice.h>
#include <lang/Object.h>


/**
 * @author Toni Aittoniemi
 */
class Dx8InputDevice :
	public id::InputDevice,
	public DrvObject
{
public:
	
	Dx8InputDevice( const GUID& guidInstance, IDirectInputDevice8* didevice, DeviceType type, char* name, HWND windowhandle );
	~Dx8InputDevice();

	void			addReference();

	void			release();

	void			poll();
	void			focusLost();
	void			flushEvents();
	void			getEvent( int index, Event* data );
	const char*		getEventCodeName( int code );
	void			setDeadZone( int eventCode, float amount );
	void			enumerateDeviceObjects();
	bool			objectsDirty() const;
	float			deadZone( int eventCoed ) const;
	int				eventCodeCount() const;
	unsigned int	getEventCode( int index ) const;
	int				events() const;
	const char*		name() const;
	int				mappedAxes() const;
	int				axes() const;
	int				buttons() const;
	DeviceType		deviceType() const;
	const GUID*		guid() const;

	static DeviceType	convertDi8DevType( unsigned int devcategory );

private:
	struct ObjectDataFormat
	{
		DIOBJECTDATAFORMAT	format;
		GUID*				guid;
		char				name[MAX_PATH];
		int					range[2];
		int					isdoubledto[2];
	};
	struct CodeMap
	{
		unsigned int		code;
		int					sign;
		int					dataOffset;
		char				name[MAX_PATH];
	};

	struct DeadZoneEntry
	{
		int					code;
		float				amount;
		DeadZoneEntry() : code(0), amount(0.f) {}
		DeadZoneEntry( int c, float a ) : code(c), amount(a) {}
	};

	long						m_refs;

	DIDEVCAPS					m_diDevCaps;
	DeviceType					m_type;
	char						m_name[MAX_PATH];
	GUID						m_guid;

	IDirectInputDevice8*		m_device;
	Array<ObjectDataFormat>		m_dataformat;
	int							m_dataformatalignment;
	Array<CodeMap>				m_codes;

	unsigned long				m_buffersize;
	Array<DIDEVICEOBJECTDATA>	m_currentevents;
	unsigned long				m_eventcount;
	Array<unsigned int>			m_relaxislastpos;
	bool						m_lastbufferoverflowed;
	Array<bool>					m_relaxisFirstIteration;

	Array<DeadZoneEntry>		m_deadZones;

	static BOOL CALLBACK DIEnumDeviceObjectsCallback( LPCDIDEVICEOBJECTINSTANCE doi, LPVOID ref);

	int findDeadZoneEntry( const int& eventcode ) const;

	Dx8InputDevice( const Dx8InputDevice& );
	Dx8InputDevice& operator=( const Dx8InputDevice& );
};

#endif // _DX8INPUTDEVICE_H 
