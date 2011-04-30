#include "Timer.h"
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

Timer::Timer( float dt ) :
	m_dt( dt ),
	m_time( 0.f ),
	m_frameTime( 0.f )
{
}

void Timer::beginUpdate( float frameTime )
{
	m_frameTime += frameTime;
}

bool Timer::update()
{
	if ( m_frameTime >= m_dt )
	{
		m_frameTime -= m_dt;
		m_time += m_dt;
		return true;
	}
	else
	{
		return false;
	}
}

float Timer::dt() const
{
	return m_dt;
}

float Timer::time() const
{
	return m_time;
}
