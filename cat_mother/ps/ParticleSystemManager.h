#ifndef _PS_PARTICLESYSTEMMANAGER_H
#define _PS_PARTICLESYSTEMMANAGER_H


#include <lang/Object.h>
#include <lang/String.h>


namespace io {
	class InputStreamArchive;}

namespace sg {
	class Node;}


namespace ps
{

class ParticleSystem;


/** 
 * Manages created particle systems. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ParticleSystemManager :
	public lang::Object
{
public:
	/** 
	 * Initializes a particle manager. 
	 * Loads particles from specified archive.
	 */
	explicit ParticleSystemManager( io::InputStreamArchive* arch );

	///
	~ParticleSystemManager();

	/** Sets parent scene. */
	void	setScene( sg::Node* scene );

	/** Updates all active particle systems by specified time delta (seconds). */
	void	update( float dt );

	/** Starts a new particle effect in reference object space. */
	ParticleSystem*	play( const lang::String& name, sg::Node* refobj );

	/** Removes all managed particle systems. */
	void	clear();

	/** Removes active managed particle systems. */
	void	removeActive();

	/** Returns number of active particle effects with specified name in reference object space. */
	int		getActiveCount( const lang::String& name, sg::Node* refobj ) const;

	/** Returns total number of active particles. */
	int		particles() const;

private:
	class ParticleSystemManagerImpl;
	P(ParticleSystemManagerImpl) m_this;

	ParticleSystemManager( const ParticleSystemManager& );
	ParticleSystemManager& operator=( const ParticleSystemManager& );
};


} // ps


#endif // _PS_PARTICLESYSTEMMANAGER_H
