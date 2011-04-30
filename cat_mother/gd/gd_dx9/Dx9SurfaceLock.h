#ifndef _DX8SURFACELOCK_H
#define _DX8SURFACELOCK_H


/** 
 * Class for locking/unlocking D3D surface automatically. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Dx9SurfaceLock
{
public:
	/** Result of the operation. */
	HRESULT hr;

	/** Locks surface. Releases the surface if error. */
	Dx9SurfaceLock( IDirect3DSurface9* surface, D3DLOCKED_RECT* rc ) :
		m_surface(surface)
	{
		hr = surface->LockRect( rc, 0, 0 );
		if ( hr != D3D_OK )
			surface->Release(); 
	}

	/** Unlocks and releases the surface. */
	~Dx9SurfaceLock()
	{
		if ( hr == D3D_OK )
		{
			m_surface->UnlockRect();
			m_surface->Release(); 
		}
	}

private:
	IDirect3DSurface9*	m_surface;

	Dx9SurfaceLock();
	Dx9SurfaceLock( const Dx9SurfaceLock& );
	Dx9SurfaceLock& operator=( const Dx9SurfaceLock& );
};


#endif // _DX8SURFACELOCK_H
