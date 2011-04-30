#ifndef	_GAMESCRIPTABLE_H
#define _GAMESCRIPTABLE_H


#include "ScriptMethod.h"
#include "TextureAnimation.h"
#include <util/Vector.h>
#include <util/Random.h>
#include <script/Scriptable.h>


namespace io {
	class InputStreamArchive;
	class InputStream;}

namespace sg {
	class Camera;
	class Light;
	class Mesh;
	class Node;}

namespace snd {
	class SoundManager;}

namespace ps {
	class ParticleSystemManager;}


/** 
 * Base functionality for all scriptable objects in the game.
 *
 * Usually scripts are used by signaling specific events to script.
 * This makes the scripts structurally cleaner and also more performance
 * friendly as script functions do not need to be executed
 * in every update.
 * 
 * Workflow for adding C++ methods to be usable from script:
 * <ul>
 * <li>		Create a static array (sm_methods) of ScriptMethod objects to derived class T.
 *			ScriptMethod constructor takes function name (to be known to script) 
 *			and C++ member function address of type
 *			int (T::*FuncType)( vm, funcName ).
 *			Scriptable functions should be named using script_(funcName),
 *			where (funcName) is the name to be known to the script.
 * <li>		In derived class T constructor add array of scriptable methods 
 *			to the script object by calling ScriptUtil::addMethods.
 *			Store the returned int as method index base (m_methodBase).
 * <li>		Override methodCall( vm, i ) in derived class.
 *			Call ScriptUtil::methodCall to get correct member function called.
 * <li>		For trivial setX(x) style script_setX implementations use helper 
 *			functions setNumber and setNumbers provided by this class.
 * <li>		See class GameCamera and GamePlayer implementations as an example.
 * </ul>
 *
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GameScriptable :
	public script::Scriptable
{
public:
	/** 
	 * Creates game object with scripting support. 
	 * @param vm Script virtual machine.
	 * @param arch File archive to use.
	 * @param soundMgr Sound manager if the object can play sounds. 0 otherwise.
	 * @param soundMgr Particle system manager if the object can play particle effects. 0 otherwise.
	 */
	explicit GameScriptable( script::VM* vm, io::InputStreamArchive* arch, 
		snd::SoundManager* soundMgr, ps::ParticleSystemManager* particleMgr );

	///
	~GameScriptable();

	/** 
	 * Compiles a script file from the archive given in the ctor. 
	 * @exception ScriptException
	 */
	void				compile( const lang::String& scriptName );

	/** 
	 * Updates object state. Default implementation does nothing.
	 * @param dt Update interval in seconds.
	 * @exception ScriptException
	 */
	virtual void		update( float dt );

	/**
	 * When a (C++) method is called from a script, this function
	 * is executed and unique method identifier is passed as parameter.
	 * Derived classes must override this if they add new scriptable methods.
	 * @param vm Script virtual machine executing the method.
	 * @param i Unique identifier (index) of the called method.
	 * @return Number of arguments returned in the script stack.
	 */
	virtual int			methodCall( script::VM* vm, int i );

	/** 
	 * Returns object to be used in rendering. Call base class implementation
	 * when overriding this as getRenderObject may be used for other purposes also (as updating animations).
	 * @param camera Pointer to camera if any. If 0 then scene is not rendered but the object is requested for other purposes.
	 */
	virtual sg::Node*	getRenderObject( sg::Camera* camera );

	/** 
	 * Removes all pending timer events. 
	 * @return Number of events removed.
	 */
	int					removeTimerEvents();

	/** 
	 * Removes timer events that start with specified string. 
	 * @return Number of events removed.
	 */
	int					removeTimerEvents( const lang::String& str );

	/** Sets name of the object. */
	void				setName( const lang::String& name );

	/** Returns number of timer events that start with specified string. */
	int					getTimerEventCount( const lang::String& str ) const;

	/** Returns name of the object if any. */
	const lang::String&	name() const;

	/** Returns key light (if any) information of this object. */
	virtual	sg::Light*	keylight() const;

	/** Removes lights and cameras from the node hierarchy. */
	static void			removeLightsAndCameras( sg::Node* root );

	/** Sets hierarchy rendering passes. */
	static void			setRenderPasses( sg::Node* root, int solidPass, int transparentPass );

	/** Replaces legacy lightmap materials with lightmap shader. */
	static void			replaceLightmapMaterialsWithShader( sg::Node* scene, sg::Shader* shader );

protected:
	/** Returns index in range [begin,end) passed as a parameter to a scriptable function. */
	static int		getIndex( script::VM* vm, const char* funcName, 
						int begin, int end, int param=1 );

	/** 
	 * Helper function for implementing simple 'set(x)' type scriptable methods.
	 * @param vm Script virtual machine executing the method.
	 * @param funcName Name of the set function called by the script.
	 * @param type Name of the type to be set from the script.
	 * @param num [out] Receives the number (first parameter passed to setX).
	 * @return Number of return values in the stack (0).
	 * @exception ScriptException
	 */
	static int		getParam( script::VM* vm, const char* funcName, 
						const char* type, float* num );

	/** 
	 * Helper function for implementing simple 'set(x,y,z)' type scriptable methods.
	 * @param vm Script virtual machine executing the method.
	 * @param funcName Name of the set function called by the script.
	 * @param type Name of the types to be set from the script.
	 * @param nums [out] Receives the numbers (parameters passed to setX).
	 * @param n Number of numbers expected as parameters.
	 * @return Number of return values in the stack (0).
	 * @exception ScriptException
	 */
	static int		getParams( script::VM* vm, const char* funcName, 
						const char* type, float* nums, int n );

	/** Returns next pseudo-random float in range [0,1). */
	float			random();

	/** Returns current screen width. */
	float			screenWidth() const;

	/** Returns current screen height. */
	float			screenHeight() const;

	/** Returns current archive. */
	io::InputStreamArchive*		archive() const;

	/** Returns current sound manager. */
	snd::SoundManager*			soundManager() const;

	/** Returns current particle system manager. */
	ps::ParticleSystemManager*	particleSystemManager() const;

private:
	class TimerEvent
	{
	public:
		int			scriptFuncRef;
		float		time;
		bool		condition;

		TimerEvent()																: scriptFuncRef(-1), time(0.f), condition(false) {}
		bool operator<( const TimerEvent& other ) const								{return time < other.time;}
	};

	// scripting
	int										m_methodBase;
	static ScriptMethod<GameScriptable>		sm_methods[];

	lang::String						m_name;
	P(io::InputStreamArchive)			m_arch;
	P(snd::SoundManager)				m_soundMgr;
	P(ps::ParticleSystemManager)		m_particleMgr;
	util::Random						m_rng;
	util::Vector<P(TextureAnimation)>	m_texAnims;
	util::Vector<TimerEvent>			m_timerEvents;
	float								m_dt;

	/** Finds texture animation identified by the script function parameter (string). */
	TextureAnimation*	getTextureAnimation( script::VM* vm, const char* funcName, int stackIndex=1 );

	/** 
	 * Finds parent node for an effect (particle system or sound). 
	 * The first parameter (at stackIndex) is an effect name, 
	 * the second is optional bone/node name.
	 * @param stackIndex Index of the first string parameter.
	 */
	sg::Node*			getAnchorNode( script::VM* vm, const char* funcName, int stackIndex );

	int		script_addNonLinearTextureAnimation( script::VM* vm, const char* funcName );
	int		script_addTextureAnimation( script::VM* vm, const char* funcName );
	int		script_addTimerEvent( script::VM* vm, const char* funcName );
	int		script_addTimerWaitCondition( script::VM* vm, const char* funcName );
	int		script_fadeInSoundAt( script::VM* vm, const char* funcName );
	int		script_fadeOutSound( script::VM* vm, const char* funcName );
	int		script_getRandomInteger( script::VM* vm, const char* funcName );
	int		script_getTextureAnimationTime( script::VM* vm, const char* funcName );
	int		script_getTimerEventCount( script::VM* vm, const char* funcName );
	int		script_include( script::VM* vm, const char* funcName );
	int		script_loadSound( script::VM* vm, const char* funcName );
	int		script_name( script::VM* vm, const char* funcName );
	int		script_playParticleSystem( script::VM* vm, const char* funcName );
	int		script_playParticleSystemAt( script::VM* vm, const char* funcName );
	int		script_playDirectedParticleSystemAt( script::VM* vm, const char* funcName );
	int		script_playSound( script::VM* vm, const char* funcName );
	int		script_playSoundOffset( script::VM* vm, const char* funcName );
	int		script_playSoundAt( script::VM* vm, const char* funcName );
	int		script_playTextureAnimation( script::VM* vm, const char* funcName );
	int		script_random( script::VM* vm, const char* funcName );
	int		script_removeTimerEvents( script::VM* vm, const char* funcName );
	int		script_setName( script::VM* vm, const char* funcName );
	int		script_setSeed( script::VM* vm, const char* funcName );
	int		script_stopSound( script::VM* vm, const char* funcName );
	int		script_stopTextureAnimation( script::VM* vm, const char* funcName );

	GameScriptable( const GameScriptable& );
	GameScriptable& operator=( const GameScriptable& );
};


#endif // _GAMESCRIPTABLE_H
