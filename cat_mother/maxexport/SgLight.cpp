#include "StdAfx.h"
#include "SgLight.h"
#include "ChunkUtil.h"
#include <io/ChunkOutputStream.h>
#include <lang/Debug.h>
#include <lang/Math.h>
#include <lang/Float.h>

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

static String toString( SgLight::LightType type )
{
	switch ( type )
	{
	case SgLight::LIGHT_DIRECT:		return "directlight";
	case SgLight::LIGHT_POINT:		return "pointlight";
	case SgLight::LIGHT_SPOT:		return "spotlight";
	case SgLight::LIGHT_AMBIENT:	return "ambientlight";
	default:						return "invalid";
	}
}

//-----------------------------------------------------------------------------

SgLight::SgLight()
{
	type			= LIGHT_DIRECT;
	on				= true;
	color			= pix::Colorf(1,1,1);
	intensity		= 1.f;
	hotsize			= 0.f;
	fallsize		= 0.f;
	farAtten		= false;
	farAttenStart	= 0.f;
	farAttenEnd		= 100e6f;
	nearAtten		= false;
	nearAttenStart	= 0.f;
	nearAttenEnd	= 0.f;
	decay			= 0.f;
	decayRadius		= 0.f;
}

void SgLight::write( ChunkOutputStream* out ) const
{
	out->beginChunk( toString(type) );
	SgNode::writeNodeChunks( out );

	Debug::println( "    color = {0} {1} {2}", color.red(), color.green(), color.blue() );
	Debug::println( "    intensity = {0}", intensity );
	Debug::println( "    hotsize = {0}", hotsize );
	Debug::println( "    fallsize = {0}", fallsize );
	Debug::println( "    farAtten = {0}", farAtten );
	Debug::println( "    farAttenStart = {0}", farAttenStart );
	Debug::println( "    farAttenEnd = {0}", farAttenEnd );
	Debug::println( "    nearAtten = {0}", nearAtten );
	Debug::println( "    nearAttenStart = {0}", nearAttenStart );
	Debug::println( "    nearAttenEnd = {0}", nearAttenEnd );
	Debug::println( "    decay = {0}", decay );
	Debug::println( "    decayRadius = {0}", decayRadius );

	if ( color != pix::Colorf(1,1,1) )
		ChunkUtil::writeFloatChunk3( out, "color", color.red(), color.green(), color.blue() );

	if ( intensity != 1.f )
		ChunkUtil::writeFloatChunk( out, "intensity", intensity );

	if ( farAtten )
	{
		float a0 = 1.f;
		float a1 = 0.f;
		float a2 = 0.f;
		ChunkUtil::writeFloatChunk3( out, "atten", a0, a1, a2 );
		ChunkUtil::writeFloatChunk( out, "range", farAttenEnd*2.f );
	}

	if ( type == LIGHT_SPOT )
	{
		ChunkUtil::writeFloatChunk( out, "innercone", Math::toRadians(hotsize) );
		ChunkUtil::writeFloatChunk( out, "outercone", Math::toRadians(fallsize) );
	}

	out->endChunk();
}

void SgLight::lit( const Vector3* vertexWorldPositions, 
	const Vector3* vertexWorldNormals,
	Vector3* vertexDiffuseColors,
	int verts )
{
	if ( on )
	{
		Matrix4x4 worldLightTm = getWorldTransform( 0.f );
		Vector3 worldLightPos = worldLightTm.translation();
		Vector3 worldLightDir = worldLightTm.rotation().getColumn(2).normalize();
		Vector3 worldLightVec = -worldLightDir;
		Vector3 lightColor = Vector3( color.red(), color.green(), color.blue() ) * intensity;
		float innerConeAngle = Math::toRadians( hotsize );
		float outerConeAngle = Math::toRadians( fallsize );

		switch ( type )
		{
		case LIGHT_DIRECT:
			for ( int i = 0 ; i < verts ; ++i )
			{
				float ndotl = vertexWorldNormals[i].dot( worldLightVec );
				if ( ndotl > 0.f )
				{
					if ( farAtten || nearAtten )
						ndotl *= getAttenuation( vertexWorldPositions[i], worldLightPos );
					vertexDiffuseColors[i] += lightColor * ndotl;
				}
			}
			break;

		case LIGHT_POINT:
			for ( int i = 0 ; i < verts ; ++i )
			{
				Vector3 worldLightVec = (worldLightPos - vertexWorldPositions[i]);
				float len = worldLightVec.length();
				if ( len > Float::MIN_VALUE )
					worldLightVec *= 1.f / len;

				float ndotl = vertexWorldNormals[i].dot( worldLightVec );
				if ( ndotl > 0.f )
				{
					if ( farAtten || nearAtten )
						ndotl *= getAttenuation( vertexWorldPositions[i], worldLightPos );
					vertexDiffuseColors[i] += lightColor * ndotl;
				}
			}
			break;

		case LIGHT_SPOT:
			for ( int i = 0 ; i < verts ; ++i )
			{
				worldLightVec = (worldLightPos - vertexWorldPositions[i]);
				float len = worldLightVec.length();
				if ( len > Float::MIN_VALUE )
					worldLightVec *= 1.f / len;

				float ndotl = vertexWorldNormals[i].dot( worldLightVec );
				if ( ndotl > 0.f )
				{
					if ( farAtten || nearAtten )
						ndotl *= getAttenuation( vertexWorldPositions[i], worldLightPos );

					float spot = 0.f;
					float ang = 2.f * Math::acos( -(worldLightDir.dot(worldLightVec)) );
					if ( ang < outerConeAngle )
					{
						if ( ang <= innerConeAngle )
							spot = 1.f;
						else
							spot = (outerConeAngle - ang) / (outerConeAngle - innerConeAngle);
					}
					ndotl *= spot;

					vertexDiffuseColors[i] += lightColor * ndotl;
				}
			}
			break;
		}

		// clamp output
		for ( int i = 0 ; i < verts ; ++i )
		{
			for ( int k = 0 ; k < 3 ; ++k )
			{
				Vector3& dif = vertexDiffuseColors[i];
				if ( dif[k] > 1.f )
					dif[k] = 1.f;
			}
		}
	}
}

float SgLight::getAttenuation( const math::Vector3& worldVertexPos,
	const math::Vector3& worldLightPos ) const
{
	const float dist = (worldVertexPos - worldLightPos).length();
	float ndotl = 1.f;

	if ( decay > 0.f && dist > decayRadius )
	{
		float d = Math::pow( decayRadius/dist, decay );
		ndotl *= d;
	}

	if ( farAtten )
	{
		float atten = (dist - farAttenStart);
		if ( (farAttenEnd - farAttenStart) > Float::MIN_VALUE )
			atten *= 1.f / (farAttenEnd - farAttenStart);

		if ( atten < 0.f )
			atten = 0.f;
		else if ( atten > 1.f )
			atten = 1.f;

		ndotl *= 1.f - atten;
	}

	if ( nearAtten )
	{
		float atten = (dist - nearAttenStart);
		if ( (nearAttenEnd - nearAttenStart) > Float::MIN_VALUE )
			atten *= 1.f / (nearAttenEnd - nearAttenStart);

		if ( atten < 0.f )
			atten = 0.f;
		else if ( atten > 1.f )
			atten = 1.f;

		ndotl *= atten;
	}

	return ndotl;
}
