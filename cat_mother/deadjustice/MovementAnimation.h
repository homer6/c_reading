#ifndef _MOVEMENTANIMATION_H
#define _MOVEMENTANIMATION_H


#include "AnimationParams.h"
#include "GameScriptable.h"
#include "ScriptMethod.h"
#include <lang/String.h>
#include <math/Vector3.h>
#include <util/Vector.h>
#include <util/Hashtable.h>


namespace io {
	class InputStreamArchive;
	class InputStream;}

namespace snd {
	class SoundManager; }

namespace ps {
	class ParticleSystemManager; }


class GameCharacter;
class Blender;


/**
 * @author Toni Aittoniemi
 */
class MovementAnimation :
	public GameScriptable
{
public:
	/** Enumerated curve types. */
	enum CurveType
	{
		/** Flat line. */
		CURVE_FLAT,
		/** Bell curve peaking halfway between start and end (both inclusive). */
		CURVE_BELL,
		/** Half of a bell curve peaking in end and zero in start (both inclusive). */
		CURVE_POS_OPENING_HALF_BELL,
		/** Half of a bell curve peaking in start and zero in end (both inclusive). */
		CURVE_NEG_OPENING_HALF_BELL,
		/** Triangle peak. */
		CURVE_TRIANGLE,
		/** Ramp up. */
		CURVE_RAMP_UP,
		/** Ramp down. */
		CURVE_RAMP_DOWN
	};

	/** Enumerated animation selection & blend control parameter types. */
	enum ControlParamType
	{
		/** No control, evaluate curve at (0). */
		CONTROL_NONE,
		/** Control is a pointer to Vector3, value is index[0] of the vector. */
		CONTROL_VECTOR2_X,
		/** Control is a pointer to Vector3, value is index[1] of the vector. */
		CONTROL_VECTOR2_Y,
		/** Control is a pointer to Vector3, value is the angle between the vector and forward in XZ plane. */
		CONTROL_VECTOR3_DIRECTIONXZ,
		/** Control is a pointer to Vector3, value is the angle between the vector and forward in YZ plane. */
		CONTROL_VECTOR3_DIRECTIONYZ,
		/** Control is a pointer to Vector3, value is the largest angle between the vector and forward in XZ or YZ plane. */		
		CONTROL_VECTOR3_DIRECTIONXZYZ,
		/** Control is a pointer to Vector3, value is the length of the vector. */
		CONTROL_VECTOR3_LENGTH,
		/** Control is a pointer to Vector3, value is a triangular interpolated value in range 1..0 starting from 1 in the center and 0 on the edges. 
			Min & Max values for this type of controller are different, they specify width & height of the diamond in radians. 
			Always use a flat blend curve with ANGLE_DIAMONDXZYZ control.*/
		CONTROL_VECTOR3_ANGLE_DIAMONDXZYZ,
		/** Control is a pointer to Vector3, value is a triangular interpolated value in range 1..0 starting from 1 in the center and 0 on the edges. 
			Min & Max values for this type of controller are different, they specify width & height of the diamond in radians. 
			Always use a flat blend curve with ANGLE_DIAMONDXZYZ control.*/
		CONTROL_VECTOR2_DIAMONDXZYZ
	};

	/** Contstruct a blank MovementAnimation. */
	MovementAnimation( script::VM* vm, io::InputStreamArchive* arch, snd::SoundManager* soundMgr, ps::ParticleSystemManager* particleMgr, GameCharacter* character, Blender* blender );

	/** Destroy MovementAnimation. */
	~MovementAnimation();

	/** Activate. */
	void activate( bool addToBlenderInstantly );

	/** Deactivate. */
	void deactivate();

	/** Update time. */
	void update( float dt );

	/** enable internal forced looping. */
	void enableLoop( bool enable );
	
	/** 
	 * Operate the Blender. 
	 */
	void outputToBlender( );

	/** Increase anim count by n. */
	void addAnims( int n );
	
	/** Increase layer count by n. */
	void addLayers( int n );

	/**
	 * Set anim parameters 
	 *
	 * @param index index to set
	 * @param name source name
	 * @param animLengthSeconds animation length in seconds ( should be reasonably accurate )
	 * @param blendDelay animation blend-in time in seconds
	 * @param layer layer to set animation on
	 * @param exitFrames pointer to list of exit frames, can be null if no exit frames should be used
	 * @param exitFrameCount number of exit frames
	 * @return true on success / false on error
	 */
	int  setAnimation( int index, const lang::String& name, float animLengthSeconds, float blendDelay, int layer, int* exitFrames, int exitFrameCount );
	
	/** 
	 * Add a blend control parameter with vectors as limiters
	 *
	 * @param name name of this control
	 * @param animation target animation ( target animation must have layer set! )
     * @param type type of control parameter
	 * @param src pointer to Vector3 to be used 
	 * @param curve curve type
	 * @param start curve start
	 * @param end curve end
	 * @return true on success / false on error
	 */ 
	int  addBlendControlParam( const lang::String& name, int animation, ControlParamType type, math::Vector3* src, CurveType curve, float height, const math::Vector3& start, const math::Vector3& end );
	
	/** 
	 * Set layer control parameters 
	 *
	 * @param layer layer index
	 * @param name name of this layer
	 * @param type type of control parameter
	 * @param src pointer to Vector3 to be used 
	 * @param minimum minimum value of control to activate this layer 
	 * @param maximum maximum value of control to activate this layer
	 * @param addAll set true to add all blends on this layer to the blender simultaneously, false to add just the active blends
	 * @return true on success / false on error
	 */
	int  setLayerParams( int layer, const lang::String& name, ControlParamType type, math::Vector3* src, const math::Vector3& minimum, const math::Vector3& maximum, bool addAll );
	
	/** 
	 * Set speed control parameter, controls the speed of animation by an external parameter
	 *
	 * @param layer layer index
	 * @param type type of control parameter
	 * @param src pointer to Vector3 to be used 
	 * @param inputmin minimum range of input
	 * @param inputmax maximum range of input
	 * @param outputmin minimum output
	 * @param outputmax maximum output
	 * @return true on success / false on error
	 */	 
	int  setSpeedControlParam( int layer, ControlParamType type, math::Vector3* src, const math::Vector3& inputmin, const math::Vector3& inputmax, const math::Vector3& outputmin, const math::Vector3& outputmax );

	/** Returns animation count. */
	int  anims() const;

	/** Returns layer count. */
	int  layers() const;

	/** Returns const of anims on layer n. */
	int  animsOnLayer( int n ) const;

private:
	/** AnimParams stores animation data. */
	class AnimParams
	{
	public:
		lang::String		name;
		float				animLengthSeconds;
		float				blendDelay;
		int					layer;
		util::Vector<int>	exitFrames;

		AnimParams();
	};
	
	/** BlendControlParams controls the visibility of an animation on output by external control. */
	class BlendControlParams
	{
	public:
		lang::String		name;
		int					anim;
		math::Vector3*		control;
		ControlParamType	controlType;
		math::Vector3		start;
		math::Vector3		end;
		CurveType			curve;
		float				curveHeight;
		int					blenderID;
		Blender*			blender;

		BlendControlParams();
		BlendControlParams( Blender* blender, const lang::String& name, int anim, ControlParamType type, math::Vector3* control, CurveType curve, float height, const math::Vector3& start, const math::Vector3& end );
	
		float	evaluateCurve( float min, float max, float pos );
		bool	inBlender() const;
	};

	/** Layer is used to stack sets of blendcontrollers. */
	class Layer
	{
	public:
		lang::String						name;
		util::Vector<BlendControlParams>	blendControllers;
		bool								addAllBlendsOnLayer;
		
		// Layer switching control ( required )
		math::Vector3*				switchControl;
		ControlParamType			switchControlType;
		math::Vector3				switchControlMinimum;
		math::Vector3				switchControlMaximum;

		// Layer speed control ( optional )
		math::Vector3*				speedControl;
		ControlParamType			speedControlType;
		math::Vector3				speedControlInputMin;
		math::Vector3				speedControlInputMax;
		math::Vector3				speedControlOutputMin;
		math::Vector3				speedControlOutputMax;	

		Layer();
	};

	// string to enum type conversion
	util::Hashtable<lang::String, ControlParamType>	m_controlParamTypeTable;
	util::Hashtable<lang::String, CurveType>		m_curveTypeTable;

	/** Last layer used for layer stickiness. */
	int							m_lastLayer;

	/** Base time, assumed to run from 0.f -> 1.f, updated by layer speed control, 
	    on output it is multiplied by AnimParams::animLengthSeconds. */
	float						m_mainTime;
	
	/** Loop internally. */
	bool						m_loop;

	/** Delta time of last update. */
	float						m_dt;

	/** Speed of main time. */
	float						m_mainSpeed;

	/** List of animations. */
	util::Vector<AnimParams>	m_anims;

	/** List of layers. */
	util::Vector<Layer>			m_layers;

	/** Points to Spawning Object. */
	GameCharacter*				m_character;

	/** Points to Blender Object. */
	Blender*					m_blender;

	/** Selects appropriate layer (sticky = last layer is preferred). */
	int				selectLayerSticky() const;

	/** Updates anim in blender, adding it to the blender if necessary. */
	void			updateAnimInBlender( MovementAnimation::BlendControlParams& blend, int anim, float animweight );

	/** Return true if control is between limits & translates control limits to float. */
	static bool		isBetween( ControlParamType type, math::Vector3* control, const math::Vector3& min, const math::Vector3& max, float* minimum = 0, float* maximum = 0, float* val = 0 );

	/** Returns absolute angle of vec and Vector3(0, 0, 1) in XZ-plane. */
	static float	getAbsoluteAngleXZ( const math::Vector3& vec );

	/** Returns absolute angle of vec and Vector3(0, 0, 1) in YZ-plane. */
	static float	getAbsoluteAngleYZ( const math::Vector3& vec );

	/** Returns max angle between vec & Vector3(0, 0, 1) in XZ or YZ plane. */
	static float	getMaxAngleXZYZ( const math::Vector3& vec);

	// scripting
	int										m_methodBase;
	static  ScriptMethod<MovementAnimation>	sm_methods[];

	int		methodCall( script::VM* vm, int i );	
	int		script_addLayer( script::VM* vm, const char* funcName );
	int		script_addSubAnim( script::VM* vm, const char* funcName );
	int		script_addBlendController( script::VM* vm, const char* funcName );
	int		script_enableForcedLooping( script::VM* vm, const char* funcName );

	MovementAnimation( const MovementAnimation& );
	MovementAnimation& operator=( const MovementAnimation& );
};


#endif // _MOVEMENTANIMATION_H
