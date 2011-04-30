#ifndef _OVERLAYFONT_H
#define _OVERLAYFONT_H


#include "GameScriptable.h"
#include <sg/Font.h>
#include <util/Vector.h>


namespace sg {
	class Font;
	class TriangleList;
	class Camera;
	class Mesh;}


/** 
 * On-screen info font and set of positioned text strings. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class OverlayFont :
	public GameScriptable
{
public:
	OverlayFont( script::VM* vm, io::InputStreamArchive* arch, const lang::String& fontName, sg::TriangleList* tri );

	~OverlayFont();

	/** 
	 * Update overlays. 
	 * @exception Exception
	 * @exception ScriptException
	 */
	void		update( float dt );

	/** Renders overlay items. */
	void		render();

	/** Removes all texts. */
	void		clear();

	/** Returns object to be used in rendering. */
	sg::Node*	getRenderObject( sg::Camera* camera );

private:
	class TextItem
	{
	public:
		lang::String	str;
		float			x, y;	// top left
		float			id;

		TextItem() : x(0), y(0), id(0) {}
	};

	static const float	ALIGN_CENTER;
	static const float	ALIGN_RIGHT;
	static const float	ALIGN_BOTTOM;
	static const float  ALIGN_LEFT;

	// script independent variables
	P(io::InputStreamArchive)	m_arch;
	P(sg::Font)					m_font;

	// script dependent variables
	util::Vector<TextItem>		m_texts;
	int							m_textId;

	bool	hasTextItemIndex( float id ) const;
	int		getTextItemIndex( float id ) const;

	// scripting
	int									m_methodBase;
	static ScriptMethod<OverlayFont>	sm_methods[];

	int		methodCall( script::VM* vm, int i );
	int		script_addText( script::VM* vm, const char* funcName );
	int		script_getWidth( script::VM* vm, const char* funcName );
	int		script_getHeight( script::VM* vm, const char* funcName );
	int		script_getTextPosition( script::VM* vm, const char* funcName );
	int		script_replaceText( script::VM* vm, const char* funcName );
	int		script_removeText( script::VM* vm, const char* funcName );
	int		script_removeTexts( script::VM* vm, const char* funcName );
	int		script_setTextPosition( script::VM* vm, const char* funcName );
	int		script_getGlyphIndex( script::VM* vm, const char* funcName );
	int		script_getGlyphString( script::VM* vm, const char* funcName );
	int		script_numGlyphs( script::VM* vm, const char* funcName );

	OverlayFont( const OverlayFont& );
	OverlayFont& operator=( const OverlayFont& );
};


#endif // _OVERLAYFONT_H
