#include <bsp/BSPNode.h>
#include <bsp/BSPTreeBuilder.h>
#include <bsp/BSPBoxSplitSelector.h>
#include <bsp/BSPBalanceSplitSelector.h>
#include <lang/Thread.h>


/**
 * Wrapper for building BSP tree in separate thread.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class BSPTreeBuilderThread :
	public lang::Thread
{
public:
	BSPTreeBuilderThread( bsp::BSPTreeBuilder& builder ) :
		m_builder(builder), m_root(0), m_done(false)
	{
		initMutex();
	}

	void run()
	{
		//bsp::BSPBalanceSplitSelector balance;
		bsp::BSPBoxSplitSelector balance;
		m_root = m_builder.build( &balance );
		setDone( true );
	}

	bool done() const
	{
		synchronized( this );
		return m_done;
	}

	float progress() const
	{
		return m_builder.progress();
	}

	P(bsp::BSPNode) root() const
	{
		return m_root;
	}

private:
	bsp::BSPTreeBuilder&	m_builder;
	P(bsp::BSPNode)			m_root;
	bool					m_done;

	void setDone( bool done )
	{
		synchronized( this );
		m_done = done;
	}
};
