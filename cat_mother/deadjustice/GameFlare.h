#ifndef _GAMEFLARE_H
#define _GAMEFLARE_H


#include <sg/Node.h>


/** 
 * Simple flare to be rendered.
 * @see GameFlareSet
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GameFlare
{
public:
	GameFlare()																		: m_parent(0), m_fade(-1.f) {}
	explicit GameFlare( sg::Node* parent )											: m_parent(parent), m_fade(-1.f) {}

	/** Sets flare fade level or -1 if fading is invalidated (by camera view change). */
	void			setFade( float fade )											{m_fade=fade;}

	/** Returns parent node of the flare. */
	sg::Node*		parent() const													{return m_parent;}

	/** Returns current fade level or -1 if not checked yet. */
	float			fade() const													{return m_fade;}

private:
	P(sg::Node)		m_parent;
	float			m_fade;
};


#endif // _GAMEFLARE_H
