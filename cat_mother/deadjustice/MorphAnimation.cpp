#include "MorphAnimation.h"
#include <lang/Debug.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

MorphAnimation::MorphAnimation( const String& baseMeshName, const String& animName ) :
	m_morphers( Allocator<P(Morpher)>(__FILE__) ),
	m_baseMeshName( baseMeshName ),
	m_name( animName ),
	m_time( -1.f ),
	m_endTime( 0.f ),
	m_blendTarget( 0 ),
	m_blendSource( 0 ),
	m_blendTime( 0.f ),
	m_blendDelay( 0.f ),
	m_removeAfterEnd( false )
{
}

MorphAnimation::MorphAnimation( const MorphAnimation& other ) :
	m_morphers( other.m_morphers ),
	m_baseMeshName( other.m_baseMeshName ),
	m_name( other.m_name ),
	m_time( other.m_time ),
	m_endTime( 0.f ),
	m_blendTarget( 0 ),
	m_blendSource( 0 ),
	m_blendTime( 0.f ),
	m_blendDelay( 0.f ),
	m_removeAfterEnd( false )
{
}

MorphAnimation::~MorphAnimation()
{
	stop();
}

void MorphAnimation::apply()
{
	if ( active() && !isBlendTarget() )
	{
		for ( int k = 0 ; k < m_morphers.size() ; ++k )
		{
			Morpher* morpher = m_morphers[k];
			
			if ( m_blendTarget )
			{
				// blending between two morph animations
				float u = m_blendTime/m_blendDelay;
				anim::Animatable* morphers[2] = {morpher, m_blendTarget->m_morphers[k]};
				float times[2] = {m_time, m_blendTarget->m_time};
				float weights[2] = {1.f-u, u};
				morpher->blendState( morphers, times, weights, 2 );
			}
			else 
			{
				// playing single morph animation
				morpher->setState( m_time );
			}

			//Debug::println( "MorphAnimation: {0} -------------------------------------------", m_name );
			morpher->apply( true );
		}
	}
}

void MorphAnimation::update( float dt )
{
	if ( active() )
	{
		m_time += dt;

		if ( m_blendTarget )
		{
			m_blendTime += dt;
			if ( m_blendTime >= m_blendDelay )
			{
				assert( m_blendTarget->active() );
				stop();
			}
		}
	}
}

void MorphAnimation::start( float startTime )
{
	stop();
	m_time = startTime;
}

void MorphAnimation::stop()
{
	assert( valid() );

	m_time = -1.f;
	
	if ( m_blendTarget )
	{
		assert( !m_blendSource );
		assert( m_blendTarget->m_blendSource == this );

		m_blendTarget->m_blendSource = 0;
		m_blendTarget = 0;
	}

	if ( m_blendSource )
	{
		assert( !m_blendTarget );
		assert( m_blendSource->m_blendTarget == this );

		m_blendSource->m_blendTarget = 0;
		m_blendSource = 0;
	}
}

bool MorphAnimation::active() const
{
	return m_time >= 0.f;
}

void MorphAnimation::blendTo( MorphAnimation* dst, float blendDelay, float dstStartTime )
{
	assert( valid() );

	dst->stop();
	dst->m_time = dstStartTime;
	dst->m_blendSource = this;

	m_blendSource = 0;
	m_blendTarget = dst;
	m_blendTime = 0.f;
	m_blendDelay = blendDelay;

	assert( valid() );
}

float MorphAnimation::time() const
{
	assert( active() );
	return m_time;
}

float MorphAnimation::endTime() const
{
	assert( active() );
	return m_endTime;
}

void MorphAnimation::addMorpher( sg::Morpher* morpher )
{
	if ( morpher )
	{
		float endTime = morpher->endTime();
		if ( endTime > m_endTime )
			m_endTime = endTime;
	}

	m_morphers.add( morpher );
}

bool MorphAnimation::isBlendTarget() const
{
	return m_blendSource != 0;
}

bool MorphAnimation::isBlendSource() const
{
	return m_blendTarget != 0;
}

float MorphAnimation::blendPhase() const
{
	assert( isBlendSource() );
	return m_blendTime / m_blendDelay;
}

MorphAnimation* MorphAnimation::blendTarget() const
{
	assert( isBlendSource() );
	return m_blendTarget;
}

bool MorphAnimation::valid() const
{
	assert( !m_blendSource || !m_blendTarget );
	assert( !m_blendTarget || m_blendTarget->m_blendSource == this );
	assert( !m_blendSource || m_blendSource->m_blendTarget == this );
	
	return
		( !m_blendSource || !m_blendTarget ) &&
		( !m_blendTarget || m_blendTarget->m_blendSource == this ) &&
		( !m_blendSource || m_blendSource->m_blendTarget == this );
}

void MorphAnimation::enableRemoveAfterEnd()
{
	m_removeAfterEnd = true;
}

bool MorphAnimation::isRemovedAfterEnd() const
{
	return m_removeAfterEnd;
}

