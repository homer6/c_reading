#include "ParticleSystemManager.h"
#include "SpriteParticleSystem.h"
#include <sg/Node.h>
#include <io/InputStream.h>
#include <io/InputStreamArchive.h>
#include <util/Vector.h>
#include <util/Hashtable.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace sg;
using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

namespace ps
{


class ParticleSystemManager::ParticleSystemManagerImpl :
	public Object
{
public:
	struct ActiveParticleSystem
	{
	public:
		P(ParticleSystem)	obj;
		float				inactive;

		ActiveParticleSystem( ParticleSystem* o=0 ) : obj(o), inactive(0) {}
	};

	ParticleSystemManagerImpl( InputStreamArchive* arch ) :
		m_scene(0), m_arch(arch),
		m_prototypes( Allocator< HashtablePair< String, P(ParticleSystem) > >(__FILE__) ),
		m_freed( Allocator< HashtablePair< String, P(Vector< P(ParticleSystem) >) > >(__FILE__) ),
		m_active( Allocator<ActiveParticleSystem>(__FILE__) )
	{
	}

	~ParticleSystemManagerImpl()
	{
	}

	void setScene( Node* scene )
	{
		m_scene = scene;
	}

	void update( float dt )
	{
		// remove inactive
		for ( int i = 0 ; i < m_active.size() ; ++i )
		{
			ActiveParticleSystem& active = m_active[i];
			P(ParticleSystem) obj = active.obj;
			if ( obj->time() + active.inactive > obj->systemLifeTime() )
			{
				obj->unlink();
				m_freed[ obj->name() ]->add( obj );
				m_active.remove( i );
				--i;
			}
		}

		// update
		for ( int i = 0 ; i < m_active.size() ; ++i )
		{
			ActiveParticleSystem& active = m_active[i];
			P(ParticleSystem) obj = active.obj;
			if ( obj->renderedInLastFrame() )
			{
				obj->update( dt );
				active.inactive = 0.f;
			}
			else
			{
				active.inactive += dt;
			}
		}
	}

	void load( const String& name )
	{
		m_prototypes[name] = new SpriteParticleSystem( name, m_arch );
		m_freed[name] = new Vector< P(ParticleSystem) >( Allocator<P(ParticleSystem)>(__FILE__) );
	}

	ParticleSystem* play( const String& name, Node* refobj )
	{
		assert( m_scene );

		// init type
		P( Vector< P(ParticleSystem) > ) freed = m_freed[name];
		if ( !freed )
		{
			load( name );
			freed = m_freed[name];
		}

		// create instance
		P(ParticleSystem) obj = 0;
		if ( freed->isEmpty() )
		{
			// create new
			obj = static_cast<ParticleSystem*>( m_prototypes[name]->clone() );
			obj->setName( name );
		}
		else
		{
			// get old from cache
			obj = freed->lastElement();
			freed->setSize( freed->size()-1 );
		}

		// link to anchor
		if ( refobj )
			obj->linkTo( refobj );
		else
			obj->linkTo( m_scene );

		// activate
		obj->reset();
		ActiveParticleSystem active( obj );
		m_active.add( active );

		// return ptr
		return obj;
	}

	void removeActive()
	{
		for ( int i = 0 ; i < m_active.size() ; ++i )
			m_active[i].obj->unlink();
		m_active.clear();
	}

	void clear()
	{
		removeActive();
		m_freed.clear();
		m_prototypes.clear();
	}

	int getActiveCount( const String& name, Node* refobj ) const
	{
		if ( !refobj )
			refobj = m_scene;

		int count = 0;
		for ( int i = 0 ; i < m_active.size() ; ++i )
		{
			const ActiveParticleSystem& active = m_active[i];
			if ( active.obj->name() == name && refobj == active.obj->parent() )
				++count;
		}
		return count;
	}

	int particles() const
	{
		int count = 0;
		for ( int i = 0 ; i < m_active.size() ; ++i )
			count += m_active[i].obj->particles();
		return count;
	}

private:
	P(Node)												m_scene;
	P(InputStreamArchive)								m_arch;
	Hashtable< String, P(ParticleSystem) >				m_prototypes;
	Hashtable< String, P(Vector< P(ParticleSystem) >) >	m_freed;
	Vector<ActiveParticleSystem>						m_active;

	ParticleSystemManagerImpl( const ParticleSystemManagerImpl& );
	ParticleSystemManagerImpl& operator=( const ParticleSystemManagerImpl& );
};

//-----------------------------------------------------------------------------

ParticleSystemManager::ParticleSystemManager( InputStreamArchive* arch )
{
	m_this = new ParticleSystemManagerImpl( arch );
}

ParticleSystemManager::~ParticleSystemManager()
{
}

void ParticleSystemManager::update( float dt )
{
	m_this->update( dt );
}

ParticleSystem* ParticleSystemManager::play( const String& name, Node* refobj )
{
	return m_this->play( name, refobj );
}

void ParticleSystemManager::clear()
{
	m_this->clear();
}

void ParticleSystemManager::removeActive()
{
	m_this->removeActive();
}

void ParticleSystemManager::setScene( Node* scene )
{
	m_this->setScene( scene );
}

int ParticleSystemManager::particles() const
{
	return m_this->particles();
}

int ParticleSystemManager::getActiveCount( const String& name, Node* refobj ) const
{
	return m_this->getActiveCount( name, refobj );
}


} // ps
