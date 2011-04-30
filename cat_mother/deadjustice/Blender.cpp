#include "Blender.h"
#include "BlendData.h"
#include <lang/Float.h>
#include <lang/String.h>
#include <lang/Integer.h>
#include <lang/Exception.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

Blender::Blender() :
	m_maxSlewRate( 2.f ),
	m_idgenerator( 0 ),
	m_blends( Allocator<P(BlendData)>(__FILE__) ),
	m_blendCache( Allocator<P(BlendData)>(__FILE__) )
{
}

Blender::~Blender()
{
	for ( int i = 0; i < m_blends.size(); ++i )
		m_blends[i] = 0;

	for ( int i = 0; i < m_blendCache.size(); ++i )
		m_blendCache[i] = 0;
}

void Blender::update( float dt ) 
{
	// Update blends
	int i;
	for ( i = 0; i < m_blends.size(); ++i )
	{
		BlendData* blend = m_blends[i];

		blend->anim.blendTime += dt;

		switch ( blend->state )
		{
		case BlendData::BLEND_FADEIN:
			// Fade-in
			assert( blend->weightDeltaInSecond >= 0.f );
			blend->anim.weight += dt * blend->weightDeltaInSecond;
						
			if ( blend->anim.blendTime >= blend->anim.blendDelay )
			{
				blend->state = BlendData::BLEND_STATIC;
			}
			break;
		case BlendData::BLEND_STATIC:
			// Apply slew rate 
			if ( blend->targetWeight > blend->anim.weight )
			{
				float offset = blend->targetWeight - blend->anim.weight;

				if ( offset > dt * m_maxSlewRate )
					blend->anim.weight += dt * m_maxSlewRate;
				else
					blend->anim.weight += offset;
			}
			else if ( blend->targetWeight < blend->anim.weight )
			{
				float offset = blend->anim.weight - blend->targetWeight;

				if ( offset > dt * m_maxSlewRate )
					blend->anim.weight -= dt * m_maxSlewRate;
				else
					blend->anim.weight -= offset;
			}
			break;
		case BlendData::BLEND_FADEOUT:
			// Timed fade-out, blender assumes responsibility of advancing time of fading-out anims
			if ( blend->anim.blendTime <= blend->anim.blendDelay )
			{
				blend->anim.weight += dt * blend->weightDeltaInSecond;
				blend->anim.time += dt * blend->anim.speed;
			}
			else
			{
				blend->anim.weight = 0.f;
			}
			break;
		case BlendData::BLEND_INACTIVE:
			throw Exception( Format( "BLEND_INACTIVE state in blender on anim {0}!", blend->anim.name ) );
			break;
		default:
			throw Exception( Format( "Invalid blend state in Blender! (Should never occur)" ) );
			break;
		}
	}

	// Clear finished fadeouts;
	i = 0;
	while ( i < m_blends.size() )
	{
		bool removed = false;

		if ( m_blends[i]->state == BlendData::BLEND_FADEOUT && m_blends[i]->anim.weight <= 0.f )
		{
			m_blendCache.add(m_blends[i]);
			m_blends[i]->state = BlendData::BLEND_INACTIVE;
			m_blends.remove(i);
			removed = true;
		}

		if ( !removed )
		{
			i++;
		}
	}
}

int Blender::addBlend( const AnimationParams& animationparams, float targetWeight ) 
{
	P(BlendData) blendData = 0;

	if ( ++m_idgenerator >= Integer::MAX_VALUE )
		m_idgenerator = 0;
	
	if ( m_blendCache.size() > 0 )
	{
		int index = m_blendCache.size() - 1;
		blendData = m_blendCache[index];
		m_blendCache.setSize( index );
	}
	else
	{
		blendData =  new BlendData;
	}

	blendData->init( m_idgenerator, animationparams, BlendData::BLEND_FADEIN, targetWeight );
	m_blends.add( blendData );

	return m_idgenerator;
}

int Blender::addBlendInstantly( const AnimationParams& animationparams ) 
{
	P(BlendData) blendData = 0;

	if ( ++m_idgenerator >= Integer::MAX_VALUE )
		m_idgenerator = 0;
	
	if ( m_blendCache.size() > 0 )
	{
		int index = m_blendCache.size() - 1;
		blendData = m_blendCache[index];
		m_blendCache.setSize( index );
	}
	else
	{
		blendData = new BlendData;
	}

	blendData->init( m_idgenerator,  animationparams, BlendData::BLEND_STATIC, animationparams.weight );
	m_blends.add( blendData );

	return m_idgenerator;
}

int Blender::setAnimWeight( int id, float weight ) 
{
	BlendData* blend = getBlend(id);

	if ( !blend ) 
		return false;

//	float blendTime		= blend->anim.blendTime;
//	float blendDelay	= blend->anim.blendDelay;
//	float blendWeight	= blend->anim.weight;

	blend->targetWeight	= weight;

	if ( blend->state == BlendData::BLEND_FADEIN )
	{
//		float remainingTime = (blendDelay - blendTime);
//		blend->weightDeltaInSecond = (blend->targetWeight - blendWeight) / remainingTime;
	}

	return true;
}

int Blender::setAnimTime( int id, float time ) 
{
	BlendData* blend = getBlend(id);

	if ( !blend ) 
		return false;

	blend->anim.time = time;

	return true;
}

void Blender::setMaxSlewRate( float rate ) 
{
	if ( rate < 0.f ) 
		throw Exception( Format("Blender::setMaxSlewRate(); Slew rate must be > 0!") );

	m_maxSlewRate = rate;
}

void Blender::fadeoutBlend( int id ) 
{
	BlendData* blend = getBlend(id);

	if ( blend && blend->state != BlendData::BLEND_FADEOUT )
	{
		blend->state = BlendData::BLEND_FADEOUT;
		blend->weightDeltaInSecond = (1.f / blend->anim.blendDelay) * -blend->anim.weight;
		blend->anim.blendTime = 0.f;
	}
}

void Blender::fadeoutAllBlends() 
{
	for ( int i = 0; i < m_blends.size(); ++i )
		if ( m_blends[i]->state != BlendData::BLEND_FADEOUT )
		{
			m_blends[i]->state = BlendData::BLEND_FADEOUT;
			m_blends[i]->weightDeltaInSecond = (1.f / m_blends[i]->anim.blendDelay) * -m_blends[i]->anim.weight;
			m_blends[i]->anim.blendTime = 0.f;
		}
}

void Blender::removeAllBlends()
{
	for ( int i = 0; i < m_blends.size(); ++i )
	{
		m_blends[i]->state = BlendData::BLEND_INACTIVE;
		m_blendCache.add( m_blends[i] );
	}
	m_blends.setSize(0);
}

int Blender::getResult( int maxAnims, AnimationParams* buffer, float normalizelimit ) 
{
	int noAnims = maxAnims < m_blends.size() ? maxAnims : m_blends.size();
	int i;
	int j;
	float weightsum = 0.f;
	int aboveZero = 0;

	for ( i = 0, j = 0; i < noAnims; ++i )
	{
		if ( m_blends[i]->anim.weight > Float::MIN_VALUE )
		{
			buffer[j]	= m_blends[i]->anim;
			weightsum  += m_blends[i]->anim.weight;
			aboveZero	= j + 1;
			j++;
		}
	}

	if ( weightsum == 0.f )		// Exit to avoid divide by zero
	{
		return 0;	
	}

	for ( i = 0; i < aboveZero; ++i )
	{
		if ( weightsum > normalizelimit )
		{
			buffer[i].weight /= weightsum;
		}
	}

	return aboveZero;
}

int Blender::getAnims( int maxAnims, lang::String* names, float* times, float* weights ) const
{
	int n = 0;
	for ( int i = 0; i < m_blends.size() ; ++i )
	{
		if ( m_blends[i]->anim.weight > Float::MIN_VALUE )
		{
			if ( n < maxAnims )
			{
				names[n]	= m_blends[i]->anim.name;
				times[n]	= m_blends[i]->anim.time;
				weights[n]	= m_blends[i]->anim.weight;
				++n;
			}
		}
	}
	return n;
}

int	Blender::blends() const
{
	return m_blends.size();
}

void Blender::getBlends( BlendData* blends ) const
{
	for ( int i = 0; i < m_blends.size(); ++i )
	{
		blends[i] = *m_blends[i];
	}
}

String Blender::stateString( bool normalizeWeights ) const 
{
	const int N = 16;
	String names[N];
	float times[N];
	float weights[N];
	int n = getAnims( N, names, times, weights );

	float sumw = 0.f;
	for ( int i = 0; i < n ; ++i )
		sumw += weights[i];
	if ( normalizeWeights && sumw > Float::MIN_VALUE )
		sumw = 1.f / sumw;
	else
		sumw = 1.f;

	String out = Format( "n = {0}; ", m_blends.size() ).format();
	for ( int i = 0; i < n ; ++i )
		out = Format("{2}{0}(w={1,#.##},t={3,#.00}), ", names[i], weights[i]*sumw, out, times[i]).format();

	return out;
}

BlendData*	Blender::getBlend( int id ) const 
{
	for ( int i = 0; i < m_blends.size(); ++i )
		if ( m_blends[i]->id == id )
			return m_blends[i].ptr();

	return 0;
}

const AnimationParams& Blender::getAnim( int id ) const 
{
	for ( int i = 0; i < m_blends.size(); ++i )
		if ( m_blends[i]->id == id )
			return m_blends[i]->anim;

	throw Exception( Format("Blender::getAnim(); Animation ID {0} not found!", id ) );
	return *new AnimationParams;
}
