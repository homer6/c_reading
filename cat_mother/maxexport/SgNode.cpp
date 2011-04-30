#include "StdAfx.h"
#include "SgNode.h"
#include "ChunkUtil.h"
#include "AnimExportUtil.h"
#include <io/ChunkOutputStream.h>
#include <lang/Debug.h>
#include <math/Quaternion.h>

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

SgNode::SgNode() :
	name(""),
	pos(3, KeyFrame::DATA_SCALAR),
	rot(4, KeyFrame::DATA_QUATERNION),
	scl(3, KeyFrame::DATA_SCALAR),
	parent(-1),
	target(-1),
	parentNode(0),
	targetNode(0),
	castShadows(true),
	renderable(true),
	resampleAnimations(true),
	tmAnim( Allocator<Matrix4x4>(__FILE__,__LINE__) )
{
}

SgNode::~SgNode()
{
}

void SgNode::write( io::ChunkOutputStream* out ) const
{
	out->beginChunk( "node" );
	writeNodeChunks( out );
	out->endChunk();
}
	
void SgNode::writeNodeChunks( ChunkOutputStream* out ) const
{
	Debug::println( "" );
	Debug::println( "  Writing {0}:", name );
	Debug::println( "    parent = {0}", parent );
	Debug::println( "    target = {0}", target );
	Debug::println( "    pos = {0} keys", pos.keys() );
	Debug::println( "    rot = {0} keys", rot.keys() );
	Debug::println( "    scl = {0} keys", scl.keys() );
	Debug::println( "    renderable = {0}", renderable );

	ChunkUtil::writeStringChunk( out, "name", name );
	
	if ( -1 != parent )
		ChunkUtil::writeIntChunk( out, "parent", parent );

	if ( -1 != target )
		ChunkUtil::writeIntChunk( out, "target", target );

	ChunkUtil::writeAnimChunk( out, "pos", pos );
	ChunkUtil::writeAnimChunk( out, "rotq", rot );
	ChunkUtil::writeAnimChunk( out, "scl", scl );

	if ( !renderable )
		ChunkUtil::writeIntChunk( out, "renderable", renderable );
}

bool SgNode::isAnimated() const
{
	return pos.keys() > 1 || rot.keys() > 1 || scl.keys() > 1 || 
		-1 != target || (parentNode && parentNode->isAnimated());
}

math::Matrix4x4 SgNode::getTransform( float time ) const
{
	require( pos.keys() > 0 );
	require( rot.keys() > 0 );

	math::Matrix4x4 tm(1);

	// position
	if ( pos.keys() > 0 )
	{
		float v[3];
		pos.getValue( time, v, 3 );
		tm.setTranslation( math::Vector3(v[0], v[1], v[2]) );
	}
	
	// rotation
	if ( rot.keys() > 0 )
	{
		float v[4];
		rot.getValue( time, v, 4 );
		math::Quaternion rotq( v[0], v[1], v[2], v[3] );
		math::Matrix3x3 rot( rotq );
		tm.setRotation( rot );
	}

	// scale
	if ( scl.keys() > 0 )
	{
		float v[3];
		scl.getValue( time, v, 3 );
		math::Matrix3x3 rot = tm.rotation();
		for ( int i = 0 ; i < 3 ; ++i )
			rot.setColumn( i, rot.getColumn(i)*v[i] );
		tm.setRotation( rot );
	}
	
	return tm;
}

math::Matrix4x4 SgNode::getWorldTransform( float time ) const
{
	math::Matrix4x4 tm = getTransform( time );
	for ( SgNode* parent = parentNode ; parent ; parent = parent->parentNode )
		tm = parent->getTransform(time) * tm;
	return tm;
}

void SgNode::resampleTransformAnimation( Interval animRange, INode* node3ds )
{
	// find out which components of the transformation are animated
	//Debug::println( "{0}({1})", __FILE__, __LINE__ );
	const Vector<Matrix4x4>& anim = tmAnim;
	require( anim.size() > 0 );
	bool posAnim = true;
	bool rotAnim = true;
	bool sclAnim = true;
	AnimExportUtil::isAnimated( anim, &posAnim, &rotAnim, &sclAnim );

	// check key frame limits
	Interval posKeyRange = AnimExportUtil::getKeyFramedPositionRange( node3ds, animRange );
	Interval rotKeyRange = AnimExportUtil::getKeyFramedRotationRange( node3ds, animRange );
	Interval sclKeyRange = AnimExportUtil::getKeyFramedScaleRange( node3ds, animRange );

	Debug::println( "Node {0} has position keys in range [{1},{2}]", name, posKeyRange.Start()/GetTicksPerFrame(), posKeyRange.End()/GetTicksPerFrame() );
	Debug::println( "Node {0} has rotation keys in range [{1},{2}]", name, rotKeyRange.Start()/GetTicksPerFrame(), rotKeyRange.End()/GetTicksPerFrame() );
	Debug::println( "Node {0} has scale keys in range [{1},{2}]", name, sclKeyRange.Start()/GetTicksPerFrame(), sclKeyRange.End()/GetTicksPerFrame() );

	// resample needed animations
	//Debug::println( "{0}({1})", __FILE__, __LINE__ );
	Matrix4x4 tm0 = anim[0];
	
	//Debug::println( "{0}({1})", __FILE__, __LINE__ );
	if ( posAnim )
	{
		if ( resampleAnimations && AnimExportUtil::MAX_POSITION_RESAMPLING_ERROR > 1e-3f )
			AnimExportUtil::resamplePositionAnimation( anim, posKeyRange, &pos );
		else
			AnimExportUtil::addPositionAnimation( anim, posKeyRange, &pos );
		AnimExportUtil::offsetKeyTimes( &pos, TicksToSec(animRange.Start()) );
	}
	else
	{
		AnimExportUtil::addPositionKey( pos, tm0, 0.f );
	}

	//Debug::println( "{0}({1})", __FILE__, __LINE__ );
	if ( rotAnim && target == -1 )
	{
		if ( resampleAnimations && AnimExportUtil::MAX_ROTATION_RESAMPLING_ERROR > 1e-3f )
			AnimExportUtil::resampleRotationAnimation( anim, rotKeyRange, &rot );
		else
			AnimExportUtil::addRotationAnimation( anim, rotKeyRange, &rot );
		AnimExportUtil::offsetKeyTimes( &rot, TicksToSec(animRange.Start()) );
	}
	else
	{
		AnimExportUtil::addRotationKey( rot, tm0, 0.f );
	}

	//Debug::println( "{0}({1})", __FILE__, __LINE__ );
	if ( sclAnim )
	{
		if ( resampleAnimations && AnimExportUtil::MAX_SCALE_RESAMPLING_ERROR > 1e-3f )
			AnimExportUtil::resampleScaleAnimation( anim, sclKeyRange, &scl );
		else
			AnimExportUtil::addScaleAnimation( anim, sclKeyRange, &scl );
		AnimExportUtil::offsetKeyTimes( &scl, TicksToSec(animRange.Start()) );
	}
	else
	{
		AnimExportUtil::addScaleKey( scl, tm0, 0.f );
	}

	Debug::println( "Resampled node {0} pos/rot/scl anims: {1}/{2}/{3} keys", name, pos.keys(), rot.keys(), scl.keys() );
}
