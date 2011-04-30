#include "StdAfx.h"
#include "Dx9Effect.h"
#include "Dx9Primitive.h"
#include "Dx9GraphicsDriver.h"
#include "Dx9GraphicsDevice.h"
#include "Dx9Material.h"
#include "Dx9Texture.h"
#include "Dx9CubeTexture.h"
#include "error.h"
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

#undef new
#ifdef _DEBUG
#define DRIVER_NEW(X) new(__FILE__ "/" #X,__LINE__) X
#else
#define DRIVER_NEW(X) new X
#endif

//-----------------------------------------------------------------------------

using namespace gd;

//-----------------------------------------------------------------------------

Dx9GraphicsDriver::Dx9GraphicsDriver() :
	m_refs(0)
{
	message( "----------------------------------------------------------------" );
	message( "gd_dx9 Version %i", VERSION );
	message( "----------------------------------------------------------------" );
}

Dx9GraphicsDriver::~Dx9GraphicsDriver()
{
	destroy();
}

void Dx9GraphicsDriver::addReference()
{
	InterlockedIncrement( &m_refs );
}

void Dx9GraphicsDriver::release()
{
	if ( 0 == InterlockedDecrement( &m_refs ) )
		delete this;
}

void Dx9GraphicsDriver::destroy()
{
	assert( DrvObject::objects() == 0 );

	int objs = DrvObject::objects();
	if ( objs > 0 )
		error( "Driver dependent objects still exist (x%i) at device driver deinit", objs );

	DrvObject::deleteAll();
}

GraphicsDevice* Dx9GraphicsDriver::createGraphicsDevice()
{
	return DRIVER_NEW(Dx9GraphicsDevice);
}

Material* Dx9GraphicsDriver::createMaterial()
{
	return DRIVER_NEW(Dx9Material);
}

Primitive* Dx9GraphicsDriver::createPrimitive()
{
	return DRIVER_NEW(Dx9Primitive);
}

Effect* Dx9GraphicsDriver::createEffect()
{
	return DRIVER_NEW(Dx9Effect);
}

Texture* Dx9GraphicsDriver::createTexture()
{
	return DRIVER_NEW(Dx9Texture);
}

CubeTexture* Dx9GraphicsDriver::createCubeTexture()
{
	return DRIVER_NEW(Dx9CubeTexture);
}
