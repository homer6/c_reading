#include "MovementAnimation.h"
#include "BellCurve.h"
#include "Blender.h"
#include "GameCharacter.h"
#include "ScriptUtil.h"
#include <sg/Node.h>
#include <sgu/NodeGroupSet.h>
#include <anim/VectorInterpolator.h>
#include <lang/Debug.h>
#include <lang/Format.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/String.h>
#include <script/VM.h>
#include <script/ScriptException.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace anim;
using namespace lang;
using namespace math;
using namespace sg;
using namespace sgu;
using namespace script;
using namespace util;

//-----------------------------------------------------------------------------

ScriptMethod<MovementAnimation> MovementAnimation::sm_methods[] =
{
	//ScriptMethod<GameCharacter>( "funcName", script_funcName ),
	ScriptMethod<MovementAnimation>( "addLayer", script_addLayer ),
	ScriptMethod<MovementAnimation>( "addSubAnim", script_addSubAnim ),
	ScriptMethod<MovementAnimation>( "addBlendController", script_addBlendController ),
	ScriptMethod<MovementAnimation>( "enableForcedLooping", script_enableForcedLooping ),
};

//-----------------------------------------------------------------------------

MovementAnimation::MovementAnimation( script::VM* vm, io::InputStreamArchive* arch, snd::SoundManager* soundMgr, 
									 ps::ParticleSystemManager* particleMgr, GameCharacter* character, Blender* blender ) :
	GameScriptable( vm, arch, soundMgr, particleMgr ),
	m_controlParamTypeTable( Allocator<HashtablePair<String, MovementAnimation::ControlParamType> >(__FILE__) ),
	m_curveTypeTable( Allocator<HashtablePair<String, MovementAnimation::CurveType> >(__FILE__) ),
	m_lastLayer( 0 ),
	m_mainTime( 0.f ),
	m_loop( true ),
	m_dt( 0.f ),
	m_mainSpeed( 1.f ),
	m_anims( Allocator<MovementAnimation::AnimParams>(__FILE__) ),
	m_layers( Allocator<MovementAnimation::Layer>(__FILE__) ),
	m_character( character ),
	m_blender( blender ),
	m_methodBase( -1 )
{
	m_methodBase = ScriptUtil<MovementAnimation,GameScriptable>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );

	// prepare movementanimation parameter conversion tables
	m_controlParamTypeTable["NONE"] = CONTROL_NONE;
	m_controlParamTypeTable["VECTOR2_X"] = CONTROL_VECTOR2_X;
	m_controlParamTypeTable["VECTOR2_Y"] = CONTROL_VECTOR2_Y;
	m_controlParamTypeTable["VECTOR3_DIRECTIONXZ"] = CONTROL_VECTOR3_DIRECTIONXZ;
	m_controlParamTypeTable["VECTOR3_DIRECTIONYZ"] = CONTROL_VECTOR3_DIRECTIONYZ;
	m_controlParamTypeTable["VECTOR3_DIRECTIONXZYZ"] = CONTROL_VECTOR3_DIRECTIONXZYZ;
	m_controlParamTypeTable["VECTOR3_LENGTH"] = CONTROL_VECTOR3_LENGTH;
	m_controlParamTypeTable["VECTOR3_ANGLE_DIAMONDXZYZ"] = CONTROL_VECTOR3_ANGLE_DIAMONDXZYZ;
	m_controlParamTypeTable["VECTOR2_DIAMONDXZYZ"] = CONTROL_VECTOR2_DIAMONDXZYZ;

	m_curveTypeTable["FLAT"] = CURVE_FLAT;
	m_curveTypeTable["BELL"] = CURVE_BELL;
	m_curveTypeTable["POS_OPENING_HALF_BELL"] = CURVE_POS_OPENING_HALF_BELL;
	m_curveTypeTable["NEG_OPENING_HALF_BELL"] = CURVE_NEG_OPENING_HALF_BELL;
	m_curveTypeTable["TRIANGLE"] = CURVE_TRIANGLE;	
	m_curveTypeTable["RAMP_UP"] = CURVE_RAMP_UP;	
	m_curveTypeTable["RAMP_DOWN"] = CURVE_RAMP_DOWN;	
}

MovementAnimation::~MovementAnimation()
{
}

float MovementAnimation::getAbsoluteAngleXZ( const Vector3& vec )
{	
	Vector3 planevec = Vector3( vec.x, 0, vec.z );

	if ( planevec.length() > Float::MIN_VALUE )
	{
		planevec = planevec.normalize();

		float turn = planevec.dot( Vector3(0.f, 0.f, 1.f) );
		bool onright = ( planevec.dot( Vector3(1.f, 0.f, 0.f) ) > 0.f );
		
		float angle = Math::acos( turn );
		if ( !onright ) 
			angle = -angle;
			
		return( angle );
	}
	else
		return 0.f;
}

float MovementAnimation::getAbsoluteAngleYZ( const Vector3& vec )
{
	Vector3 planevec = Vector3( 0, vec.y, vec.z );

	if ( planevec.length() > Float::MIN_VALUE )
	{
		planevec = planevec.normalize();

		float turn = planevec.dot( Vector3(0.f, 0.f, 1.f) );
		bool ontop = ( planevec.dot( Vector3(0.f, 1.f, 0.f) ) > 0.f );
		
		float angle = Math::acos( turn );
		if ( ontop ) 
			angle = -angle;
			
		return( angle );
	}
	else
		return 0.f;
}

float MovementAnimation::getMaxAngleXZYZ( const math::Vector3& vec) 
{
	float xz = getAbsoluteAngleXZ( vec );
	float yz = getAbsoluteAngleYZ( vec );

	if ( Math::abs( xz ) > Math::abs( yz ) )
		return xz;
	else
		return yz;
}

bool MovementAnimation::isBetween( ControlParamType type, math::Vector3* control, const math::Vector3& mini, const math::Vector3& maxi, float* minimum, float* maximum, float* val )
{
	assert( control->finite() );

	bool isbetween = false;
	float tempmin, tempmax, tempval;
	
	if ( !minimum )
		minimum = &tempmin;
	if ( !maximum )
		maximum = &tempmax;
	if ( !val )
		val = &tempval;

	switch ( type )
	{
	case CONTROL_NONE:
		*minimum = 0.f;
		*maximum = 0.f;
		*val = 0.f;
		isbetween = true;			
		break;
	case CONTROL_VECTOR2_X:
		*minimum = mini.x;
		*maximum = maxi.x;
		*val = control->x;
		if ( *minimum <= *val && *val < *maximum ) 
			isbetween = true;
		break;
	case CONTROL_VECTOR2_Y:
		*minimum = mini.y;
		*maximum = maxi.y;
		*val = control->y;
		if ( *minimum <= *val && *val < *maximum ) 
			isbetween = true;
		break;
	case CONTROL_VECTOR3_DIRECTIONXZ:
		*minimum = getAbsoluteAngleXZ( mini );
		*maximum = getAbsoluteAngleXZ( maxi );
		*val = getAbsoluteAngleXZ( *control );
		if ( ( *minimum < *maximum ) && 
			 ( *minimum <= *val && *val < *maximum ) )
			isbetween = true;
		else
		if ( ( *minimum > *maximum ) &&
			 ( *val >= *minimum || *maximum > *val ) )
			isbetween = true;

		break;
	case CONTROL_VECTOR3_DIRECTIONYZ:
		*minimum = getAbsoluteAngleYZ( mini );
		*maximum = getAbsoluteAngleYZ( maxi );
		*val = getAbsoluteAngleYZ( *control );
		if ( ( *minimum < *maximum ) && 
			 ( *minimum <= *val && *val < *maximum ) )
			isbetween = true;
		else
		if ( ( *minimum > *maximum ) &&
			 ( *val >= *minimum || *maximum > *val ) )
			isbetween = true;

		break;
	case CONTROL_VECTOR3_DIRECTIONXZYZ:
		*minimum = getAbsoluteAngleXZ( mini );
		*maximum = getAbsoluteAngleXZ( maxi );
		*val = getMaxAngleXZYZ( *control );
		if ( ( *minimum < *maximum ) && 
			 ( *minimum <= *val && *val < *maximum ) )
			isbetween = true;
		else
		if ( ( *minimum > *maximum ) &&
			 ( *val >= *minimum || *maximum > *val ) )
			isbetween = true;

		break;
	case CONTROL_VECTOR3_LENGTH:
		*minimum = mini.length();
		*maximum = maxi.length();
		*val = control->length();
		if ( *minimum <= *val && *val < *maximum ) 
			isbetween = true;

		break;
	case CONTROL_VECTOR3_ANGLE_DIAMONDXZYZ:
		{
			Vector3 direction		= *control;
			float horizontalLimit	= getAbsoluteAngleXZ( mini );
			float verticalLimit		= getAbsoluteAngleYZ( maxi );
			float horizontalAngle	= getAbsoluteAngleXZ( direction );
			float verticalAngle		= getAbsoluteAngleYZ( direction );
			if ( Math::abs( horizontalAngle ) > horizontalLimit || Math::abs( verticalAngle ) > verticalLimit )
			{
				isbetween = false;
				*val = 0.f;
			}
			else
			{
				isbetween = true;

				float verticalOffset	= Math::abs( verticalAngle ) / verticalLimit;
				float horizontalOffset	= Math::abs( horizontalAngle ) / horizontalLimit;

				*val = ( 1.f - horizontalOffset ) * ( 1.f - verticalOffset );
			}
		}
		*minimum = 0.f;
		*maximum = 1.f;
		break;
	case CONTROL_VECTOR2_DIAMONDXZYZ:
		{
			Vector3 direction		= *control;
			if ( Math::abs( direction.x ) > mini.x || Math::abs( direction.y ) > maxi.y )
			{
				isbetween = false;
				*val = 0.f;
			}
			else
			{
				isbetween = true;

				float verticalOffset	= Math::abs( direction.y ) / maxi.y;
				float horizontalOffset	= Math::abs( direction.x ) / mini.x;

				*val = ( 1.f - horizontalOffset ) * ( 1.f - verticalOffset );
			}
		}
		*minimum = 0.f;
		*maximum = 1.f;
		break;
	}

	return isbetween;
}

int MovementAnimation::selectLayerSticky() const 
{
	int lastLayer = -1;
	int newLayer = -1;

	for ( int l = 0 ; l < m_layers.size() ; ++l )
	{
		if ( isBetween( m_layers[l].switchControlType, m_layers[l].switchControl, m_layers[l].switchControlMinimum, m_layers[l].switchControlMaximum ) )
		{
			if ( l == m_lastLayer )
			{
				lastLayer = m_lastLayer;
			}
			else if ( newLayer == -1 )
			{
				newLayer = l;
			}
		}
	}

	if ( lastLayer != -1 )
		return lastLayer;
	else
		return newLayer;

}

void MovementAnimation::updateAnimInBlender( MovementAnimation::BlendControlParams& blend, int anim, float animweight )
{
	// BEGIN DEBUG

//	if ( blend.inBlender() )
//	{
//		BlendData* data = m_blender->getBlend( blend.blenderID );
//		assert( data->state != BlendData::BLEND_FADEOUT );
//	}
	// END DEBUG

	// Update anim time & weight
	if ( blend.inBlender() )
	{	
		m_blender->setAnimTime( blend.blenderID, m_anims[anim].animLengthSeconds * m_mainTime ); // NOTE : will return false like setAnimWeight on removed blend

		if ( !m_blender->setAnimWeight( blend.blenderID, animweight ) )
		{
			// Add anim to blend if blender has removed the old one
			AnimationParams params;
			params.name			= m_anims[anim].name;
			params.time			= m_anims[anim].animLengthSeconds * m_mainTime;
			params.weight		= 0.f;
			params.blendTime	= 0.f;
			params.blendDelay	= m_anims[anim].blendDelay;
			blend.blenderID = m_blender->addBlend( params, animweight );
		}
/*		else
		{ 
			BlendData* data = m_blender->getBlend( blend.blenderID );
			assert ( data );

			// if anim is fading out, add a new one since the fading out anim will eventually clear itself
			if ( data->state == BlendData::BLEND_FADEOUT ) 
			{
				AnimationParams params;
				params.name			= m_anims[anim].name;
				params.time			= m_anims[anim].animLengthSeconds * m_mainTime;
				params.weight		= 0.f;
				params.blendTime	= 0.f;
				params.blendDelay	= m_anims[anim].blendDelay;

				blend.blenderID = m_blender->addBlend( params, animweight );	
			}
		}*/
	}
	else
	// Add to blender
	{ 
		AnimationParams params;
		params.name			= m_anims[anim].name;
		params.time			= m_anims[anim].animLengthSeconds * m_mainTime;
		params.weight		= 0.f;
		params.blendTime	= 0.f;
		params.blendDelay	= m_anims[anim].blendDelay;

		blend.blenderID = m_blender->addBlend( params, animweight );					
	}
}

void MovementAnimation::outputToBlender() 
{
	assert( m_blender );

	int layer = selectLayerSticky();

	if ( layer != -1 )
	{
		m_lastLayer = layer;

		// Fade out blends on other layers
		for ( int l = 0; l < layers(); ++l )
			if ( l != layer )
				for ( int i = 0; i < m_layers[l].blendControllers.size(); ++i )
				{
					BlendControlParams& blend = m_layers[l].blendControllers[i];
					
					if ( blend.inBlender() )
					{
						m_blender->fadeoutBlend( blend.blenderID );
						blend.blenderID = Blender::INVALIDID;
//						if ( !m_blender->setAnimTime( blend.blenderID, m_anims[blend.anim].animLengthSeconds * m_mainTime ) ) 
//							blend.blenderID = Blender::INVALIDID;
					}
				}

		int anim = -1;
		float animweight = 0.f;
		float highestweight = 0.f;
		Layer& thislayer = m_layers[layer];

		// Check blend controllers on this layer
		for ( int b = 0; b < thislayer.blendControllers.size(); ++b )
		{
			BlendControlParams& blend = thislayer.blendControllers[b];

			anim = blend.anim;
			float minimum, maximum, val;
			
			// Check if blend control is valid for current control input
			if ( isBetween(	blend.controlType, blend.control, blend.start, blend.end, &minimum, &maximum, &val ) )
			{
				if ( blend.controlType != CONTROL_VECTOR3_ANGLE_DIAMONDXZYZ && blend.controlType != CONTROL_VECTOR2_DIAMONDXZYZ )
				{
					// Linear interpolation
					if ( minimum < maximum )
					{
						animweight = blend.evaluateCurve( minimum, maximum, val );
					}
					else
					{
						// Angle interpolation
						if ( blend.controlType == CONTROL_VECTOR3_DIRECTIONXZ ||
							 blend.controlType == CONTROL_VECTOR3_DIRECTIONYZ ||
							 blend.controlType == CONTROL_VECTOR3_DIRECTIONXZYZ )
						{
							float numin = -Math::PI + minimum;
							float numax = Math::PI + maximum;
							float nuval = val > 0.f ? Math::PI - val : -Math::PI - val;
							animweight = blend.evaluateCurve( numin, numax, nuval );
						}
						else animweight = 1.f;
					}
				}
				else
				{
					// Diamond interpolation
					animweight = val * blend.curveHeight; 
				}

				// Update anim in blend
				updateAnimInBlender( blend, anim, animweight );
				
				// Use highest weighing anim's speed as main speed
				if ( animweight > highestweight )
				{
					highestweight = animweight;
					m_mainSpeed = 1.f / m_anims[anim].animLengthSeconds;
				}
			}
			else
			{	
				if ( thislayer.addAllBlendsOnLayer )
				{
					updateAnimInBlender( blend, anim, 0 );
				}
				else
				{
					// Remove unused blends from blender
					if ( blend.inBlender() )
					{
						m_blender->fadeoutBlend( blend.blenderID );
						blend.blenderID = Blender::INVALIDID;
						
						// Keep time running until animation has faded out
			//			if ( !m_blender->setAnimTime( blend.blenderID, m_anims[blend.anim].animLengthSeconds * m_mainTime ) ) 
			//				blend.blenderID = Blender::INVALIDID;
					}
				}

			}
		}
		// Done checking blend controllers
	}
	else
	{
		// No suitable layer found
		Debug::printlnError( "MovementAnimation: Unable to find any layer for the current value of layer control!" );
		
		// Fade out blends on all layers
		for ( int l = 0; l < layers(); ++l )
			for ( int i = 0; i < m_layers[l].blendControllers.size(); ++i )
			{
				BlendControlParams& blend = m_layers[l].blendControllers[i];
					
				if ( blend.inBlender() )
				{
					m_blender->fadeoutBlend( blend.blenderID );
					blend.blenderID = Blender::INVALIDID;
				}
			}
		return;
	}
}

void MovementAnimation::activate( bool addToBlenderInstantly )
{
	for ( int l = 0; l < layers(); ++l )
		for ( int i = 0; i < m_layers[l].blendControllers.size(); ++i )
			if ( addToBlenderInstantly )
			{
				AnimationParams anim;
				anim.name		= m_anims[ m_layers[l].blendControllers[i].anim ].name;
				anim.blendDelay = m_anims[ m_layers[l].blendControllers[i].anim ].blendDelay;
				anim.time		= m_anims[ m_layers[l].blendControllers[i].anim ].animLengthSeconds * m_mainTime;
				anim.weight		= 1;
				m_layers[l].blendControllers[i].blenderID = m_blender->addBlendInstantly( anim );
			}
			else
			{
				m_layers[l].blendControllers[i].blenderID = Blender::INVALIDID;
			}

	if ( addToBlenderInstantly )
		m_mainTime = 0.f;

	m_lastLayer = -1;
}

void MovementAnimation::deactivate()
{
	for ( int l = 0; l < layers(); ++l )
		for ( int i = 0; i < m_layers[l].blendControllers.size(); ++i )
		{
			BlendControlParams& blend = m_layers[l].blendControllers[i];
				
			m_blender->fadeoutBlend( blend.blenderID );
			blend.blenderID = Blender::INVALIDID;
		}
}

void MovementAnimation::enableLoop( bool enable ) 
{
	m_loop = enable;
}

void MovementAnimation::update( float dt ) 
{
	m_dt = dt;
	m_mainTime += dt * m_mainSpeed;

	if ( m_loop && m_mainTime > 1.f )
		m_mainTime -= 1.f;
}
	
void MovementAnimation::addAnims( int n ) 
{
	m_anims.setSize( m_anims.size() + n );
}
	
void MovementAnimation::addLayers( int n ) 
{
	m_layers.setSize( m_layers.size() + n );
}

int MovementAnimation::setAnimation( int index, const lang::String& name, float animLengthSeconds, float blendDelay, int layer, int* exitFrames, int exitFrameCount )
{
	assert( index >= 0 && index < m_anims.size() );
	
	if ( index < 0 || index >= m_anims.size() )
		return false;

	m_anims[index].name = name;
	m_anims[index].animLengthSeconds = animLengthSeconds;  // TODO : Calculate automatically. (keys - 1) / 30
	m_anims[index].blendDelay = blendDelay;
	m_anims[index].layer = layer;
	m_anims[index].exitFrames.setSize( exitFrameCount );
	for ( int i = 0; i < exitFrameCount; ++i)
		m_anims[index].exitFrames[i] = exitFrames[i];

	return true;
}

int MovementAnimation::addBlendControlParam( const lang::String& name, int animation, ControlParamType type, math::Vector3* src, CurveType curve, float height, const math::Vector3& start, const math::Vector3& end ) 
{
	assert( animation >= 0 && animation < m_anims.size() );

	if ( animation < 0 || animation >= m_anims.size() )
		return false;

	int layer = m_anims[animation].layer;

	assert( layer >= 0 && layer < m_layers.size() );

	if ( layer < 0 || layer >= m_layers.size() )
		return false;

	m_layers[layer].blendControllers.add( BlendControlParams( m_blender, name, animation, type, src, curve, height, start, end ) );

	return true;
}

int MovementAnimation::setLayerParams( int layer, const lang::String& name, ControlParamType type, math::Vector3* src, const math::Vector3& minimum, const math::Vector3& maximum, bool addAll )
{
	assert( layer >= 0 && layer < m_layers.size() );

	if ( layer < 0 || layer >= m_layers.size() )
		return false;
	
	m_layers[layer].name = name;
	m_layers[layer].addAllBlendsOnLayer = addAll;
	m_layers[layer].switchControl = src;
	m_layers[layer].switchControlType = type;
	m_layers[layer].switchControlMinimum = minimum;
	m_layers[layer].switchControlMaximum = maximum;
	
	return true;
}
	
int MovementAnimation::setSpeedControlParam( int layer, ControlParamType type, math::Vector3* src, const math::Vector3& inputmin, const math::Vector3& inputmax, const math::Vector3& outputmin, const math::Vector3& outputmax ) 
{
	assert( layer >= 0 && layer < m_layers.size() );

	if ( layer < 0 || layer >= m_layers.size() )
		return false;

	m_layers[layer].speedControl = src;
	m_layers[layer].speedControlType = type;
	m_layers[layer].speedControlInputMin = inputmin;
	m_layers[layer].speedControlInputMax = inputmax;
	m_layers[layer].speedControlOutputMin = outputmin;
	m_layers[layer].speedControlOutputMax = outputmax;

	return false;
}

int MovementAnimation::anims() const 
{
	return m_anims.size();
}

int MovementAnimation::layers() const 
{
	return m_layers.size();
}

int MovementAnimation::animsOnLayer( int n ) const 
{
	return m_layers[n].blendControllers.size();
}

// scripting
int MovementAnimation::methodCall( script::VM* vm, int i ) 
{
	return ScriptUtil<MovementAnimation,GameScriptable>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

int	MovementAnimation::script_addLayer( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_STRING, VM::TYPE_STRING, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects name, controlparam type and source as string + min & max values as number + \"ADDALLBLENDS\"/\"NORMAL\" switch.", funcName) );

	String name = vm->toString(1);
	String controltype = vm->toString(2);
	String controlsource = vm->toString(3);

	assert( m_character );

	if ( !m_controlParamTypeTable.containsKey(controltype) )
		throw ScriptException( Format( "{0} : Invalid control parameter type.", funcName ) );
		
	if ( !m_character->m_controlPtrTable.containsKey(controlsource) )
		throw ScriptException( Format( "{0} : Invalid control parameter source.", funcName ) );

	addLayers(1);
	int currentlayer = layers()-1;

	MovementAnimation::ControlParamType enumcontroltype = m_controlParamTypeTable[controltype];
	Vector3* control = m_character->m_controlPtrTable[controlsource];
	Vector3 minimum, maximum;

	switch ( enumcontroltype )
	{
	case MovementAnimation::CONTROL_NONE:
		minimum = Vector3(0,0,0);
		maximum = Vector3(0,0,0);
		break;
	case MovementAnimation::CONTROL_VECTOR2_X:
		minimum = Vector3(vm->toNumber(4), 0, 0);
		maximum = Vector3(vm->toNumber(5), 0, 0);
		break;
	case MovementAnimation::CONTROL_VECTOR2_Y:
		minimum = Vector3(0, vm->toNumber(4), 0);
		maximum = Vector3(0, vm->toNumber(5), 0);
		break;
	case MovementAnimation::CONTROL_VECTOR3_DIRECTIONXZYZ:
	case MovementAnimation::CONTROL_VECTOR3_DIRECTIONXZ:
		minimum = Vector3(0,0,1);
		minimum.rotate(Vector3(0,1,0), vm->toNumber(4));
		maximum = Vector3(0,0,1);
		maximum.rotate(Vector3(0,1,0), vm->toNumber(5));
		break;
	case MovementAnimation::CONTROL_VECTOR3_DIRECTIONYZ:
		minimum = Vector3(0,0,1);
		minimum.rotate(Vector3(1,0,0), vm->toNumber(4));
		maximum = Vector3(0,0,1);
		maximum.rotate(Vector3(1,0,0), vm->toNumber(5));
		break;
	case MovementAnimation::CONTROL_VECTOR3_LENGTH:
		minimum = Vector3(0,0,1) * vm->toNumber(4);
		maximum = Vector3(0,0,1) * vm->toNumber(5);
		break;
	case MovementAnimation::CONTROL_VECTOR3_ANGLE_DIAMONDXZYZ:
		minimum = Vector3(0,0,1);
		minimum.rotate(Vector3(0,1,0), vm->toNumber(4));
		maximum = Vector3(0,0,1);
		maximum.rotate(Vector3(1,0,0), vm->toNumber(5));
		break;
	case MovementAnimation::CONTROL_VECTOR2_DIAMONDXZYZ:
		minimum = Vector3(vm->toNumber(4), 0, 0);
		maximum = Vector3(0, vm->toNumber(5), 0);
		break;
	}

	String addAll = vm->toString(6);

	this->setLayerParams( currentlayer, name, enumcontroltype, control, minimum, maximum, addAll == "ADDALLBLENDS" ? true : false ); 
	
	vm->pushNumber( (float)currentlayer );
	return 1;
}

int MovementAnimation::script_addSubAnim( script::VM* vm, const char* funcName )
{
	int argc = vm->top();
	if ( argc < 3 ) 
		throw ScriptException( Format("{0} expects source anim name, blend delay, target layer and 0..* number of exitframes ", funcName) );

	if ( !vm->isString(1) )
		throw ScriptException( Format("{0} parameter 1 must be string", funcName) );

	if ( !vm->isNumber(2) )
		throw ScriptException( Format("{0} parameter 2 must be number", funcName) );

	if ( !vm->isNumber(3) )
		throw ScriptException( Format("{0} parameter 3 must be number", funcName) );

	String anim = vm->toString(1);
	float animblenddelay = vm->toNumber(2);
	int layer = (int)vm->toNumber(3);

	assert( m_character );

	if ( !m_character->m_anims->hasGroup(anim) )
		throw ScriptException( Format( "{0}: Invalid animation name {1}!", funcName, anim ) );

	// Find anim length
	Node* root = m_character->m_anims->getGroup( anim );
	float animlength = 0.f;
	float nodelength;

	for ( Node* node = root ; node ; node = node->nextInHierarchy() )
	{
		Interpolator* posInterpl = dynamic_cast<Interpolator*>(node->positionController());
		if ( posInterpl )
		{
			nodelength = posInterpl->getKeyTime( posInterpl->keys() - 1 );
			if ( nodelength > animlength )
				animlength = nodelength;
		}
		Interpolator* rotInterpl = dynamic_cast<Interpolator*>(node->rotationController());
		if ( rotInterpl )
		{
			nodelength = rotInterpl->getKeyTime( rotInterpl->keys() - 1 );
			if ( nodelength > animlength )
				animlength = nodelength;
		}
		Interpolator* sclInterpl = dynamic_cast<Interpolator*>(node->scaleController());
		if ( sclInterpl )
		{
			nodelength = sclInterpl->getKeyTime( sclInterpl->keys() - 1 );
			if ( nodelength > animlength )
				animlength = nodelength;
		}
	}

	if ( animlength == 0.f )
//		throw ScriptException( Format( "script_addMovementAnimationSubAnim() : Anim '{0}' length could not be retrieved. Does the animation have Interpolation Controllers?", anim ) );
		animlength = 1.f;

	addAnims( 1 );
	int currentanim = anims() - 1;
	
	Vector<int> exitframes( Allocator<int>(__FILE__,__LINE__) );
	exitframes.setSize( argc - 3 );
	for ( int i = 4; i <= argc; ++i )
	{
		if ( !vm->isNumber(i) )
			throw Exception( Format("{0} parameter {1} must be number", funcName, i ) );
		
		exitframes.add( (int)vm->toNumber( i ) );
	}

	setAnimation( currentanim, anim, animlength, animblenddelay, layer, exitframes.begin(), exitframes.size() );
	
	vm->pushNumber( (float)currentanim );
	return 1;
}
int MovementAnimation::script_addBlendController( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_STRING, VM::TYPE_STRING, VM::TYPE_STRING, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects target name, anim index, controlparam type, source and curve type as string + curve height, min & max values as number ", funcName) );
	
	int anim = (int)vm->toNumber(1);
	String controltype = vm->toString(2);
	String controlsource = vm->toString(3);
	String controlcurve = vm->toString(4);
	float  curveheight = vm->toNumber(5);
	
	assert( m_character );

	if ( !m_controlParamTypeTable.containsKey(controltype) )
		throw ScriptException( Format( "script_addMovementAnimationLayer() : Invalid control parameter type." ) );
		
	if ( !m_character->m_controlPtrTable.containsKey(controlsource) )
		throw ScriptException( Format( "script_addMovementAnimationLayer() : Invalid control parameter source." ) );

	if ( !m_curveTypeTable.containsKey(controlcurve) )
		throw ScriptException( Format( "script_addMovementAnimationLayer() : Invalid control curve." ) );		

	MovementAnimation::ControlParamType enumcontroltype = m_controlParamTypeTable[controltype];
	MovementAnimation::CurveType enumcurve = m_curveTypeTable[controlcurve];
	Vector3* control = m_character->m_controlPtrTable[controlsource];
	Vector3 minimum, maximum;

	switch ( enumcontroltype )
	{
	case MovementAnimation::CONTROL_NONE:
		minimum = Vector3(0,0,0);
		maximum = Vector3(0,0,0);
		break;
	case MovementAnimation::CONTROL_VECTOR2_X:
		minimum = Vector3(vm->toNumber(6), 0, 0);
		maximum = Vector3(vm->toNumber(7), 0, 0);
		break;
	case MovementAnimation::CONTROL_VECTOR2_Y:
		minimum = Vector3(0, vm->toNumber(6), 0);
		maximum = Vector3(0, vm->toNumber(7), 0);
		break;
	case MovementAnimation::CONTROL_VECTOR3_DIRECTIONXZYZ:
	case MovementAnimation::CONTROL_VECTOR3_DIRECTIONXZ:
		minimum = Vector3(0,0,1).rotate(Vector3(0,1,0), Math::toRadians( vm->toNumber(6) < 0.f ? 360.f + vm->toNumber(6) : vm->toNumber(6) ));
		maximum = Vector3(0,0,1).rotate(Vector3(0,1,0), Math::toRadians( vm->toNumber(7) < 0.f ? 360.f + vm->toNumber(7) : vm->toNumber(7) ));
		break;
	case MovementAnimation::CONTROL_VECTOR3_DIRECTIONYZ:
		minimum = Vector3(0,0,1).rotate(Vector3(1,0,0), Math::toRadians( vm->toNumber(6) < 0.f ? 360.f + vm->toNumber(6) : vm->toNumber(6) ));
		maximum = Vector3(0,0,1).rotate(Vector3(1,0,0), Math::toRadians( vm->toNumber(7) < 0.f ? 360.f + vm->toNumber(7) : vm->toNumber(7) ));
		break;
	case MovementAnimation::CONTROL_VECTOR3_LENGTH:
		minimum = Vector3(0,0,1) * vm->toNumber(6);
		maximum = Vector3(0,0,1) * vm->toNumber(7);
		break;
	case MovementAnimation::CONTROL_VECTOR3_ANGLE_DIAMONDXZYZ:
		minimum = Vector3(0,0,1).rotate(Vector3(0,1,0), Math::toRadians( vm->toNumber(6) < 0.f ? 360.f + vm->toNumber(6) : vm->toNumber(6) ));
		maximum = Vector3(0,0,1).rotate(Vector3(1,0,0), Math::toRadians( vm->toNumber(7) < 0.f ? 360.f + vm->toNumber(7) : vm->toNumber(7) ));		
		break;
	case MovementAnimation::CONTROL_VECTOR2_DIAMONDXZYZ:
		minimum = Vector3(vm->toNumber(6), 0, 0);
		maximum = Vector3(0, vm->toNumber(7), 0);
		break;
	}

	addBlendControlParam( "BlendController", anim, enumcontroltype, control, enumcurve, curveheight, minimum, maximum );
	
	return 0;
}

int MovementAnimation::script_enableForcedLooping( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 1 )
		throw ScriptException( Format("{0} expects boolean (1/nil) toggling forced looping on/off", funcName) );

	m_loop = !vm->isNil(1);
	return 0;
}

//-----------------------------------------------------------------------------

MovementAnimation::AnimParams::AnimParams() :
	name(""),
	animLengthSeconds(0.f),
	blendDelay(0.f),
	layer(0),
	exitFrames( Allocator<int>(__FILE__) )
{
}

//-----------------------------------------------------------------------------

MovementAnimation::BlendControlParams::BlendControlParams() :
	name(""),
	anim(0),
	control(0),
	controlType(MovementAnimation::CONTROL_NONE),
	start(Vector3(0,0,0)),
	end(Vector3(0,0,0)),
	curve(MovementAnimation::CURVE_FLAT),
	curveHeight(1),
	blenderID( Blender::INVALIDID ),
	blender(0)
{
}

MovementAnimation::BlendControlParams::BlendControlParams( Blender* _blender, const String& _name, int _anim, ControlParamType _type, Vector3* _control, CurveType _curve, float _height, const Vector3& _start, const Vector3& _end ) :
	name(_name),
	anim(_anim),
	control(_control),
	controlType(_type),
	start(_start),
	end(_end),
	curve(_curve),
	curveHeight(_height),
	blenderID( Blender::INVALIDID ),
	blender( _blender )
{
}

float MovementAnimation::BlendControlParams::evaluateCurve( float min, float max, float pos )
{
	switch ( curve )
	{
	case CURVE_BELL:
		return curveHeight * BellCurve::evaluateFull( min, max, pos );
		break;
	case CURVE_POS_OPENING_HALF_BELL:
		return curveHeight * BellCurve::evaluateNegHalf( min, max, pos );
		break;
	case CURVE_NEG_OPENING_HALF_BELL:
		return curveHeight * BellCurve::evaluatePosHalf( min, max, pos );
		break;
	case CURVE_TRIANGLE:
		{
			float halfwidth = ( max - min ) / 2.f;
			float offset = pos - min;

			if ( offset <= halfwidth )
				return curveHeight * ( offset / halfwidth ); 
			else
				return curveHeight * ( (max - pos) / halfwidth );
		}
		break;
	case CURVE_RAMP_UP:
		return curveHeight * ( (pos - min) / ( max - min ) );
		break;
	case CURVE_RAMP_DOWN:
		return curveHeight * ( 1.f - ((pos - min) / ( max - min )) );
		break;
	default:
		return curveHeight;
	}
}

bool MovementAnimation::BlendControlParams::inBlender() const 
{
	if ( blenderID == Blender::INVALIDID ) 
		return false;
	
	BlendData* blenddata = blender->getBlend( blenderID );

	if ( !blenddata )
		return false;

	if ( blenddata->state == BlendData::BLEND_FADEOUT )
		return false;

	return true;
}


//-----------------------------------------------------------------------------

MovementAnimation::Layer::Layer() :
	name(""),
	blendControllers(Allocator<MovementAnimation::BlendControlParams>(__FILE__)),
	addAllBlendsOnLayer(false),
	switchControl(0),
	switchControlType(CONTROL_NONE),
	switchControlMinimum(Vector3(0,0,0)),
	switchControlMaximum(Vector3(0,0,0)),
	speedControl(0),
	speedControlType(CONTROL_NONE),
	speedControlInputMin(Vector3(0,0,0)),
	speedControlInputMax(Vector3(0,0,0)),
	speedControlOutputMin(Vector3(0,0,0)),
	speedControlOutputMax(Vector3(0,0,0))
{
}
