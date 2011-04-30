#ifndef _SCENEENVIRONMENT_H
#define _SCENEENVIRONMENT_H


namespace io {
	class ChunkOutputStream;}


/** 
 * Rendering environment of the scene. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SceneEnvironment
{
public:
	/** Ambient color. */
	float	ambient[3];

	/** Creates default rendering environment. */
	SceneEnvironment();

	/** Creates rendering environment from scene settings. */
	SceneEnvironment( Interface* ip, Interval animRange );

	/** Writes environment chunk to the stream. */
	void	write( io::ChunkOutputStream* out );
};


#endif // _SCENEENVIRONMENT_H
