#include <win/FrameWindow.h>
#include <id/InputDriver.h>
#include <id/InputDevice.h>
#include <sg/Context.h>
#include <sg/Font.h>
#include <io/FileInputStream.h>
#include <lang/Array.h>
#include <lang/Exception.h>
#include <lang/DynamicLinkLibrary.h>
#include <stdio.h>
#include "config.h"


using namespace id;
using namespace io;
using namespace win;
using namespace lang;
using namespace sg;


void run( FrameWindow* wnd, InputDriver* inputDrv )
{
	int err = inputDrv->create();
	if ( err )
		throw Exception( Format("InputDriver create failed") );

	P(Context) renderer = new Context("gd_dx9");
	renderer->open();

	/* Load Font */
	P(FileInputStream)	textfile = new FileInputStream( "font_Arial.txt" );
	P(FileInputStream)	graphicsfile = new FileInputStream( "font_Arial.tga" );
	P(Font)				font = new Font( graphicsfile, textfile );
	float				axl[64][16];
	int					axlcount[16];
	int					buttons[64][16];
	Array<String>		keys(0);
	bool				escape = false;

	/* Reset control values */
	for ( int i = 0; i < 16; ++i )
	{
		for ( int j = 0; j < 64; ++j )
		{
			axl[j][i] = 0.f;
			buttons[j][i] = 0;
		}
		axlcount[i] = 0;
	}

	while ( wnd->flushWindowMessages() && (!escape ) )
	{	
		/* Get attached controllers */
		inputDrv->refreshAttachedInputDevices();
		int count = inputDrv->attachedInputDevices();

		/* Start rendering */
		renderer->beginScene();
		renderer->clear();

		float x = 4.f;
		float y = 4.f;

		/* Poll controllers */

		for ( int i = count - 1 ; i >= 0 ; --i )
		{
			InputDevice* controller = inputDrv->getAttachedInputDevice( i );

			/* Check if device caps dirty */
			if ( controller->objectsDirty() )
			{
				if ( i != 1 )
				{
					i = i;
				}
				controller->enumerateDeviceObjects();
				axlcount[i] = 0;
			}

			controller->poll();
			int events = controller->events();
		
			if ( events > 0 ) 
			{	
				for ( int j = 0 ; j < events ; ++j )
				{
					struct InputDevice::Event event;
					controller->getEvent( j, &event );
					
					if ( event.code & InputDevice::EVENT_AXIS_MASK )
					{
						int axis		= event.code - 1;
						axl[axis][i]	= event.value;
						if (axis + 1 > axlcount[i]) axlcount[i] = axis + 1;
					}
					if ( event.code & InputDevice::EVENT_RELAXIS_MASK )
					{
						int axis		= (event.code >> InputDevice::EVENT_RELAXIS_BITOFFSET) - 1;
						axl[axis][i]    = event.value;
						if (axis + 1 > axlcount[i]) axlcount[i] = axis + 1;
					}
					if ( event.code & InputDevice::EVENT_BUTTON_MASK )
					{
						int button = ( ( event.code & InputDevice::EVENT_BUTTON_MASK ) >> InputDevice::EVENT_BUTTON_BITOFFSET ) - 1;
					
						buttons[button][i] = (int)event.value;
					}
					if ( event.code & InputDevice::EVENT_KEY_MASK )
					{
						keys.add( controller->getEventCodeName( event.code ) + Format(" {0}", event.value).format() );
						if ( event.code == 0x010000 ) 
							escape = true;
					}
				}
			}

			if (controller->deviceType() == InputDevice::TYPE_GAMEPAD || controller->deviceType() == InputDevice::TYPE_JOYSTICK || controller->deviceType() == InputDevice::TYPE_FLIGHT)
			{
				font->drawText( x, y, Format("Directional controller {0} index {1}", controller->name(), i ).format(), 0, &y );
				for ( int ix = 0; ix < axlcount[i] ; ++ix )
					font->drawText( x, y, Format("Axis {0,#.###} {1,#.###}", ix, axl[ix][i] ).format(), 0, &y );
				font->drawText( x, y, Format("Buttons 0-9   {0}{1}{2}{3}{4}{5}{6}{7}{8}{9}", buttons[0][i], buttons[1][i], buttons[2][i], buttons[3][i], buttons[4][i], buttons[5][i], buttons[6][i], buttons[7][i], buttons[8][i], buttons[9][i] ).format(), 0, &y );
				font->drawText( x, y, Format("Buttons 10-15 {0}{1}{2}{3}{4}{5}", buttons[10][i], buttons[11][i], buttons[12][i], buttons[13][i], buttons[14][i], buttons[15][i] ).format(), 0, &y );
			}
			if (controller->deviceType() == InputDevice::TYPE_MOUSE)
			{
				font->drawText( x, y, Format("Mouse {0} index {1}", controller->name(), i ).format(), 0, &y );
				for ( int ix = 0; ix < axlcount[i] ; ++ix )
					font->drawText( x, y, Format("Axis {0} {1}", ix, axl[ix][i] ).format(), 0, &y );
				font->drawText( x, y, Format("Buttons {0}{1}{2}{3}{4}{5}{6}{7}", buttons[0][i], buttons[1][i], buttons[2][i], buttons[3][i], buttons[4][i], buttons[5][i], buttons[6][i], buttons[7][i] ).format(), 0, &y );
			}
			if (controller->deviceType() == InputDevice::TYPE_KEYBOARD)
			{
				font->drawText( x, y, Format("Keyboard {0} index {1}", controller->name(), i).format(), 0, &y );
				for ( int j = 0; j < keys.size(); ++j )
					font->drawText( x, y, keys[j], 0, &y );	
			}
		}
		
		if ( y > 1024 ) keys.clear();
		renderer->endScene();
		renderer->present();
	}
}

int WINAPI WinMain( HINSTANCE inst, HINSTANCE, LPSTR cmdLine, int cmdShow )
{
	P(FrameWindow)	wnd = 0;

	try
	{
		wnd = new FrameWindow;
		wnd->create( "id_test", "input test", 640, 1024, false, inst );

		DynamicLinkLibrary dll( "id_dx8" );
		createInputDriverFunc createFunc = (createInputDriverFunc)dll.getProcAddress( "createInputDriver" );
		P(InputDriver) inputDrv = createFunc();
		run( wnd, inputDrv );
		inputDrv = 0;
	}
	catch ( Throwable& e )
	{
		// show error message
		char msgText[256];
		char msgTitle[256];
		sprintf( msgTitle, "%s - Error", "input test" );
		e.getMessage().format().getBytes( msgText, sizeof(msgText), "ASCII-7" );
		MessageBox( 0, msgText, msgTitle, MB_OK|MB_ICONERROR );
	}
	return 0;
}
