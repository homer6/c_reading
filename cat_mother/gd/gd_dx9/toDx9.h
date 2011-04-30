#include "Dx9Material.h"
#include <gd/GraphicsDevice.h>
#include <gd/LightState.h>
#include <pix/SurfaceFormat.h>
#include <math/Vector3.h>
#include <math/Matrix4x4.h>


void					toDx9( const pix::Colorf& s, D3DCOLORVALUE& d );
void					toDx9( const gd::LightState& s, D3DLIGHT9& d );
void					toDx9( const math::Vector3& s, D3DVECTOR& d );
void					toDx9( const math::Matrix4x4& s, D3DMATRIX& d );
D3DFORMAT				toDx9( const pix::SurfaceFormat& fmt );
D3DDEVTYPE				toDx9( gd::GraphicsDevice::RasterizerType rz );
D3DFOGMODE				toDx9( gd::GraphicsDevice::FogMode mode );
D3DTEXTUREFILTERTYPE	toDx9( gd::GraphicsDevice::TextureFilterType v );

D3DSTENCILOP			toDx9( gd::Material::StencilOperation sop );
D3DTEXTUREADDRESS		toDx9( gd::Material::TextureAddressMode v );
D3DTEXTUREFILTERTYPE	toDx9( gd::Material::TextureFilterType v );
D3DTEXTUREOP			toDx9( gd::Material::TextureOperation v );
DWORD					toDx9( gd::Material::TextureArgument v );
DWORD					toDx9( const gd::Material::TextureCoordinateTransformMode v );
DWORD					toDx9( const gd::Material::TextureCoordinateSourceType v );
void					toDx9( const Dx9Material::ReflectanceFactors& refl, D3DMATERIAL9& d3dmat );
D3DBLEND				toDx9( gd::Material::BlendMode v );
D3DCMPFUNC				toDx9( gd::Material::CmpFunc v );
D3DCULL					toDx9( gd::Material::CullMode v );
D3DMATERIALCOLORSOURCE	toDx9( gd::Material::MaterialColorSource v );

pix::SurfaceFormat		fromDx9( D3DFORMAT fmt );
void					fromDx9( const D3DMATRIX& s, math::Matrix4x4& d );
