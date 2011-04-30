#ifndef _OVERLAYBITMAP_H
#define _OVERLAYBITMAP_H


#include "GameScriptable.h"
#include <sg/Mesh.h>
#include <sg/Texture.h>
#include <util/Vector.h>


namespace sg {
	class Mesh;
	class Sprite;
	class TriangleList;
	class Camera;}


/**
 * On-screen info bitmap and set of positioned sprites using the bitmap. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class OverlayBitmap :
	public GameScriptable
{
public:
	OverlayBitmap( script::VM* vm, io::InputStreamArchive* arch, const lang::String& filename, sg::TriangleList* tri );

	~OverlayBitmap();

	/** 
	 * Update overlays. 
	 * @exception Exception
	 * @exception ScriptException
	 */
	void		update( float dt );

	/** Renders overlay items. */
	void		render();

	/** Removes all sprites. */
	void		clear();

	/** Returns object to be used in rendering. */
	sg::Node*	getRenderObject( sg::Camera* camera );

private:
	static const float	ALIGN_CENTER;
	static const float	ALIGN_RIGHT;
	static const float	ALIGN_BOTTOM;
	static const float  ALIGN_LEFT;

	// scripting
	int									m_methodBase;
	static ScriptMethod<OverlayBitmap>	sm_methods[];

	// script independent variables
	P(io::InputStreamArchive)	m_arch;
	P(sg::TriangleList)			m_tri;
	P(sg::Sprite)				m_bmp;

	// overlay items
	P(sg::Mesh)					m_sprites;

	/** 
	 * Finds sprite by id. 
	 * @exception Exception If sprite not found.
	 */
	int		getSpriteIndex( float id );

	int		methodCall( script::VM* vm, int i );
	int		script_addSprite( script::VM* vm, const char* funcName );
	int		script_getSpritePosition( script::VM* vm, const char* funcName );
	int		script_getSpriteRotation( script::VM* vm, const char* funcName );
	int		script_getSpriteScale( script::VM* vm, const char* funcName );
	int		script_height( script::VM* vm, const char* funcName );
	int		script_removeSprite( script::VM* vm, const char* funcName );
	int		script_setSpritePosition( script::VM* vm, const char* funcName );
	int		script_setSpriteRotation( script::VM* vm, const char* funcName );
	int		script_setSpriteScale( script::VM* vm, const char* funcName );
	int		script_width( script::VM* vm, const char* funcName );

	OverlayBitmap( const OverlayBitmap& );
	OverlayBitmap& operator=( const OverlayBitmap& );
};


#endif // _OVERLAYBITMAP_H
