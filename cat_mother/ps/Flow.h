#ifndef _PS_FLOW_H
#define _PS_FLOW_H


#include <ps/PathParticleSystem.h>
#include <lang/String.h>


namespace io {
	class InputStream;
	class InputStreamArchive;}


namespace ps
{


/** 
 * Flow particle system. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Flow :
	public ps::PathParticleSystem
{
public:
	/** Start distance of the randomized paths around central path. */
	float			startRadius;

	/** End distance of the randomized paths around central path. */
	float			endRadius;

	/** Name of the path source mesh if any. */
	lang::String	pathSource;

	/** Name of the path target mesh if any. */
	lang::String	pathTarget;

	///
	Flow();

	/** Loads flow parameters from (.flo) text file. Doesnt set any paths. */
	explicit Flow( const lang::String& filename );

	/** Loads flow parameters from (.flo) input stream. Doesnt set any paths. */
	explicit Flow( io::InputStream* in, io::InputStreamArchive* zip );

	/** Copy by value. */
	Flow( const Flow& other );

private:
	void	load( io::InputStream* in, io::InputStreamArchive* zip );

	Flow& operator=( const Flow& );
};


} // ps


#endif // _PS_FLOW_H
