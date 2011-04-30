#ifndef _GAMENOISE_H
#define _GAMENOISE_H


#include "GameObject.h"


/** 
 * AI detects 'noises'. Not related to sounds user hears.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GameNoise :
	public GameObject
{
public:
	GameNoise( script::VM* vm, io::InputStreamArchive* arch );
	
	/** Initializes noise with all parameters. */
	void			init( GameObject* source, float noiseLevel, 
						float fadeDistanceStart, float fadeDistanceEnd,
						float fadeOutTimeStart, float fadeOutTimeEnd );

	/** Called before putting noise to cache. */
	void			deinit();

	/** Updates noise state. */
	void			update( float dt );

	/** Sets noise level. */
	void			setNoiseLevel( float v );

	/** Sets noise distance attenuation. */
	void			setFadeDistance( float start, float end );

	/** Sets noise time attenuation. */
	void			setFadeOutTime( float start, float end );

	/** Returns relative noise volume [0,1] at specified position. */
	float			getLevelAt( const math::Vector3& hearingPos ) const;

	/** Returns noise level at the center of noise. */
	float			noiseLevel() const												{return m_noiseLevel;}

	/** Returns source of the noise. */
	GameObject*		noiseSource() const												{return m_source;}

	/** Returns true if noise has ended. */
	bool			ended() const													{return m_time >= m_fadeOutTimeEnd;}

private:
	GameObject*		m_source;
	float			m_noiseLevel;
	float			m_fadeDistanceStart;
	float			m_fadeDistanceEnd;
	float			m_fadeOutTimeStart;
	float			m_fadeOutTimeEnd;
	float			m_time;
	float			m_fadeDistanceEndSquared;

	GameNoise( const GameNoise& );
	GameNoise& operator=( const GameNoise& );

	// scripting
	int									m_methodBase;
	static ScriptMethod<GameNoise>		sm_methods[];

	int		methodCall( script::VM* vm, int i );
	int		script_getSource( script::VM* vm, const char* funcName );	
};


#endif // _GAMENOISE_H
