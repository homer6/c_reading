#include "Morpher.h"
#include <sg/Model.h>
#include <sg/VertexFormat.h>
#include <sg/MorphTarget.h>
#include <sg/VertexAndIndexLock.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Debug.h>
#include <lang/String.h>
#include <util/Vector.h>
#include <math/Vector3.h>
#include <anim/VectorInterpolator.h>
#include <stdint.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;
using namespace math;
using namespace anim;

//-----------------------------------------------------------------------------

namespace sg
{


class Morpher::MorpherImpl :
	public Object
{
public:
	enum { MAX_HINTS = 2 };
		
	class Channel
	{
	public:
		float					weight;
		P(VectorInterpolator)	weightAnim;
		P(MorphTarget)			target;

		Channel() :
			weight(0),
			weightAnim(0),
			target(0)
		{
		}
	};

	MorpherImpl() :
		m_channels( Allocator<Channel>(__FILE__) ),
		m_weightsDirty(true)
	{
	}

	~MorpherImpl() 
	{
	}

	void blendState( Animatable** anims,
		const float* times, const float* weights, int n )
	{
		int channels = m_channels.size();
		for ( int i = 0 ; i < channels ; ++i )
		{
			Channel& chn = m_channels[i];
			float weight = 0.f;

			for ( int k = 0 ; k < n ; ++k )
			{
				Morpher* other = dynamic_cast<Morpher*>(anims[k]);
				assert( other );
				assert( channels == other->m_this->m_channels.size() );

				if ( other && other->m_this->m_channels.size() == channels )
				{
					Channel& chn2 = other->m_this->m_channels[i];
					assert( chn2.target->name() == chn.target->name() );

					float weight2 = chn2.weight;
					if ( chn2.weightAnim )
					{
						if ( k < MAX_HINTS )
							m_hints[k] = chn2.weightAnim->getValue( times[k], &weight2, 1, m_hints[k] );
						else
							chn2.weightAnim->getValue( times[k], &weight2, 1, 0 );
					}

					weight += weight2 * weights[k];
				}
			}

			if ( Math::abs(chn.weight-weight) > 1e-3f )
			{
				chn.weight = weight;
				m_weightsDirty = true;
			}
		}
	}

	void draw( Shader* shader ) 
	{
		if ( !m_model || m_weightsDirty )
		{
			apply( true );
			m_weightsDirty = false;
		}

		assert( m_model );
		m_model->setShader( shader );
		m_model->draw();
	}

	void load()
	{
		if ( m_base )
		{
			apply( true );
			m_model->load();
		}
	}

	void unload()
	{
		if ( m_model )
			m_model->unload();
	}

	void destroy()
	{
		m_base = 0;
		m_model = 0;
		m_channels.clear();
		m_channels.trimToSize();
	}

	void apply( bool reset )
	{
		assert( m_base );

		if ( !m_model )
		{
			m_model = new Model( m_base->vertices(), m_base->indices(), m_base->vertexFormat(), Model::USAGE_STATIC );
			m_model->setShader( m_base->shader() );

			VertexAndIndexLock<Model> lkbase( m_base, Model::LOCK_READ );
			VertexAndIndexLock<Model> lktgt( m_model, Model::LOCK_WRITE );
			
			m_model->copyVertices( 0, m_base, 0, m_model->vertices() );
			m_model->copyIndices( 0, m_base, 0, m_model->indices() );
		}

		{
			VertexAndIndexLock<Model> lkbase( m_base, Model::LOCK_READ );
			VertexAndIndexLock<Model> lktgt( m_model, Model::LOCK_READWRITE );

			if ( reset )
			{
				int verts = m_model->vertices();
				m_model->copyVertices( 0, m_base, 0, verts );
			}

			for ( int k = 0 ; k < m_channels.size() ; ++k )
			{
				Channel& chn = m_channels[k];
				if ( chn.weight > Float::MIN_VALUE )
				{
					//Debug::println( "  Channel {0} weight = {1}", k, chn.weight );
					chn.target->apply( m_model, chn.weight );
				}
			}
		}

		m_model->setBoundSphere( m_base->boundSphere() );
		m_model->setBoundBox( m_base->boundBox() );
	}

	bool updateVisibility( const math::Matrix4x4& modelToCamera, 
		const ViewFrustum& viewFrustum ) 
	{
		return m_base->updateVisibility( modelToCamera, viewFrustum );
	}

	void setBase( Model* model ) 
	{
		m_base = model;
	}

	void setOutput( Model* model ) 
	{
		m_model = model;
	}

	void addTarget( MorphTarget* target )
	{
		Channel chn;
		chn.target = target;
		m_channels.add( chn );
	}

	void setTargetWeight( const String& name, float weight ) 
	{
		for ( int i = 0 ; i < m_channels.size() ; ++i )
		{
			if ( name == m_channels[i].target->name() )
			{
				m_channels[i].weight = weight;
				m_weightsDirty = true;
				return;
			}
		}
		assert( false );
	}

	void setTargetWeightController( const String& name, VectorInterpolator* anim ) 
	{
		for ( int i = 0 ; i < m_channels.size() ; ++i )
		{
			if ( name == m_channels[i].target->name() )
			{
				m_channels[i].weightAnim = anim;
				return;
			}
		}
		assert( false );
	}

	float boundSphere() const 
	{
		return m_base->boundSphere();
	}

	int usedBones() const 
	{
		return m_base->usedBones();
	}

	const int* usedBoneArray() const 
	{
		return m_base->usedBoneArray();
	}

	int targets() const
	{
		return m_channels.size();
	}

	Model* output() const
	{
		return m_model;
	}

	MorphTarget* getTarget( int i ) const
	{
		return m_channels[i].target;
	}

	VectorInterpolator* getTargetWeightController( int i ) const
	{
		return m_channels[i].weightAnim;
	}

	bool isValidBase( const Model* model ) const
	{
		for ( int i = 0 ; i < m_channels.size() ; ++i )
			if ( !m_channels[i].target->isValidBase(model) )
				return false;
		return true;
	}

	Model* base() const
	{
		return m_base;
	}

private:
	P(Model)		m_base;
	P(Model)		m_model;
	Vector<Channel>	m_channels;
	int				m_hints[MAX_HINTS];
	bool			m_weightsDirty;
};

//-----------------------------------------------------------------------------

Morpher::Morpher() 
{
	m_this = new MorpherImpl;
}

Morpher::~Morpher() 
{
}

Morpher::Morpher( const Morpher& other, int shareFlags ) :
	Primitive( other, shareFlags )
{
	if ( shareFlags & SHARE_GEOMETRY )
		m_this = other.m_this;
	else
		m_this = new MorpherImpl( *other.m_this );
}

Primitive* Morpher::clone( int shareFlags ) const
{
	return new Morpher( *this, shareFlags );
}

void Morpher::blendState( Animatable** anims, 
	const float* times, const float* weights, int n )
{
	m_this->blendState( anims, times, weights, n );
}

void Morpher::draw() 
{
	m_this->draw( shader() );
}

bool Morpher::updateVisibility( const math::Matrix4x4& modelToCamera, 
	const ViewFrustum& viewFrustum ) 
{
	return m_this->updateVisibility( modelToCamera, viewFrustum );
}

void Morpher::setBase( Model* model ) 
{
	m_this->setBase( model );
	setShader( model->shader() );
}

void Morpher::setOutput( Model* model ) 
{
	m_this->setOutput( model );
}

void Morpher::apply( bool reset )
{
	m_this->apply( reset );
}

void Morpher::addTarget( MorphTarget* target )
{
	m_this->addTarget( target );
}

void Morpher::setTargetWeight( const String& name, float weight ) 
{
	m_this->setTargetWeight( name, weight );
}

void Morpher::setTargetWeightController( const String& name, VectorInterpolator* anim ) 
{
	m_this->setTargetWeightController( name, anim );
}

float Morpher::boundSphere() const 
{
	return m_this->boundSphere();
}

int Morpher::usedBones() const 
{
	return m_this->usedBones();
}

const int* Morpher::usedBoneArray() const 
{
	return m_this->usedBoneArray();
}

int Morpher::targets() const
{
	return m_this->targets();
}

void Morpher::load()
{
	m_this->load();
}

void Morpher::unload()
{
	m_this->unload();
}

void Morpher::destroy()
{
	m_this = 0;
	Primitive::destroy();
}

Model* Morpher::base() const
{
	return m_this->base();
}

Model* Morpher::output() const
{
	return m_this->output();
}

MorphTarget* Morpher::getTarget( int i ) const
{
	return m_this->getTarget( i );
}

VectorInterpolator* Morpher::getTargetWeightController( int i ) const
{
	return m_this->getTargetWeightController( i );
}

bool Morpher::isValidBase( const Model* model ) const
{
	return m_this->isValidBase( model );
}

VertexFormat Morpher::vertexFormat() const
{
	assert( m_this->base() );
	return m_this->base()->vertexFormat();
}

float Morpher::endTime() const
{
	float endTime = 0.f;

	for ( int i = 0 ; i < m_this->targets() ; ++i )
	{
		VectorInterpolator* vi = m_this->getTargetWeightController( i );
		float t = vi->endTime();
		if ( t > endTime )
			endTime = t;
	}

	return endTime;
}


} // sg
