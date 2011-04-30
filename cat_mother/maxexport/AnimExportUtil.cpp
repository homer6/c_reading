#include "StdAfx.h"
#include "KeyFrame.h"
#include "TmUtil.h"
#include "AnimExportUtil.h"
#include "KeyFrameContainer.h"
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Debug.h>
#include <lang/String.h>
#include <lang/Exception.h>
#include <util/Vector.h>
#include <math/Vector3.h>
#include <math/Matrix3x3.h>
#include <math/Quaternion.h>
#include "resampling.h"

//-----------------------------------------------------------------------------

#define INTERP_TYPE (KeyFrame::INTERPOLATE_LINEAR)

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

float AnimExportUtil::MAX_POSITION_RESAMPLING_ERROR		= 0.1f;

float AnimExportUtil::MAX_ROTATION_RESAMPLING_ERROR		= 0.1f;

float AnimExportUtil::MAX_SCALE_RESAMPLING_ERROR		= 0.5f;

float AnimExportUtil::MAX_MORPH_RESAMPLING_ERROR		= 1.f;

//-----------------------------------------------------------------------------

/** Clamps value to specified range. */
static float clamp( float x, float minx, float maxx )
{
	if ( x < minx )
		return minx;
	else if ( x > maxx )
		return maxx;
	return x;
}

/** Returns true if the channel data of the key frames are identical. */
static bool isEqualValue( const KeyFrame& a, const KeyFrame& b, int channels )
{
	for ( int i = 0 ; i < channels ; ++i )
		if ( a.getChannel(i) != b.getChannel(i) )
			return false;
	return true;
}

/** Returns specified axis (from rotation submatrix). */
static Vector3 getAxis( const Matrix4x4& m, int axis )
{
	return Vector3( m(0,axis), m(1,axis), m(2,axis) );
}

/** Normalizes vector if not null. */
static Vector3 normalize0( const Vector3& v0 )
{
	Vector3 v( v0 );
	float lensqr = v.lengthSquared();
	if ( lensqr > Float::MIN_VALUE )
		v *= 1.f/Math::sqrt(lensqr);
	else
		v = Vector3(0,0,0);
	return v;
}

/** Returns tick count aligned to even sample frame. */
static TimeValue alignTicks( TimeValue ticks )
{
	TimeValue dt = SGEXPORT_TICKS_PER_SAMPLE;
	return ((ticks+dt/2) / dt)*dt;
}

/** 
 * Resamples animation until maximum error is smaller than specified value. 
 * @param maxErr Maximum absolute error (meters).
 */
static void resamplePositionKeys( KeyFrameContainer& anim, Interval range, float maxErr, const Vector<Matrix4x4>& tm )
{
	TimeValue dticks = SGEXPORT_TICKS_PER_SAMPLE;
	int frame = range.Start() / dticks;
	require( frame >= 0 && frame < tm.size() );
	TimeValue rangeLen = (range.Duration()/SGEXPORT_TICKS_PER_SAMPLE)*SGEXPORT_TICKS_PER_SAMPLE;

	for ( TimeValue ticks = range.Start() ; ticks < range.End() ; ticks += dticks )
	{
		if ( ticks > range.End() )
			ticks = range.End();

		// find out error (distance) between real and sampled animation
		require( frame >= 0 && frame < tm.size() );
		const Matrix4x4& m = tm[frame++];
		Vector3 ref = m.translation();
		float cmp[3];
		anim.getValue( TicksToSec(ticks), cmp, 3 );
		float err = (Vector3(cmp[0],cmp[1],cmp[2]) - ref).length();

		// sample more accurately if needed
		if ( err > maxErr && rangeLen > dticks )
		{
			TimeValue halfRange = alignTicks( (range.End() + range.Start())/2 );
			int ind = halfRange/dticks;
			require( ind >= 0 && ind < tm.size() );
			AnimExportUtil::addPositionKey( anim, tm[ind], TicksToSec(halfRange) );
			if ( ticks <= halfRange )
				resamplePositionKeys( anim, Interval(range.Start(),halfRange), maxErr, tm );
			else
				resamplePositionKeys( anim, Interval(halfRange,range.End()), maxErr, tm );
		}

		if ( ticks == range.End() )
			break;
	}
}

/** 
 * Resamples animation until maximum error is smaller than specified angle (radians). 
 * @param maxErr Maximum absolute error (radians).
 */
static void resampleRotationKeys( KeyFrameContainer& anim, Interval range, float maxErr, const Vector<Matrix4x4>& tm )
{
	TimeValue dticks = SGEXPORT_TICKS_PER_SAMPLE;
	int frame = range.Start() / dticks;
	require( frame >= 0 && frame < tm.size() );
	TimeValue rangeLen = (range.Duration()/SGEXPORT_TICKS_PER_SAMPLE)*SGEXPORT_TICKS_PER_SAMPLE;

	for ( TimeValue ticks = range.Start() ; ticks < range.End() ; ticks += dticks )
	{
		if ( ticks > range.End() )
			ticks = range.End();

		// find out error (distance) between real and sampled animation
		require( frame >= 0 && frame < tm.size() );
		const Matrix4x4& m = tm[frame++];
		Matrix3x3 ref = m.rotation().orthonormalize();
		float q[4];
		anim.getValue( TicksToSec(ticks), q, 4 );
		Matrix3x3 cmp( Quaternion(q[0],q[1],q[2],q[3]) );
		float xang = Math::abs( Math::acos( clamp(cmp.getColumn(0).dot(ref.getColumn(0)), -1.f, 1.f) ) );
		float yang = Math::abs( Math::acos( clamp(cmp.getColumn(1).dot(ref.getColumn(1)), -1.f, 1.f) ) );
		float zang = Math::abs( Math::acos( clamp(cmp.getColumn(2).dot(ref.getColumn(2)), -1.f, 1.f) ) );
		float err = xang;
		if ( yang > err )
			err = yang;
		if ( zang > err )
			err = zang;

		// sample more accurately if needed
		if ( err > maxErr && rangeLen > dticks )
		{
			TimeValue halfRange = alignTicks( (range.End() + range.Start())/2 );
			AnimExportUtil::addRotationKey( anim, tm[halfRange/dticks], TicksToSec(halfRange) );
			if ( ticks <= halfRange )
				resampleRotationKeys( anim, Interval(range.Start(),halfRange), maxErr, tm );
			else
				resampleRotationKeys( anim, Interval(halfRange,range.End()), maxErr, tm );
		}

		if ( ticks == range.End() )
			break;
	}
}

/** 
 * Resamples animation until maximum error is smaller than specified value. 
 * @param maxErr Maximum relative error, i.e. 0.05f == 5%.
 */
static void resampleScaleKeys( KeyFrameContainer& anim, Interval range, float maxErr, const Vector<Matrix4x4>& tm )
{
	TimeValue dticks = SGEXPORT_TICKS_PER_SAMPLE;
	int frame = range.Start() / dticks;
	require( frame >= 0 && frame < tm.size() );
	TimeValue rangeLen = (range.Duration()/SGEXPORT_TICKS_PER_SAMPLE)*SGEXPORT_TICKS_PER_SAMPLE;

	for ( TimeValue ticks = range.Start() ; ticks < range.End() ; ticks += dticks )
	{
		if ( ticks > range.End() )
			ticks = range.End();

		// find out error (distance) between real and sampled animation
		require( frame >= 0 && frame < tm.size() );
		const Matrix4x4& m = tm[frame++];
		Vector3 ref;
		Vector3 a;
		a = getAxis(m,0); ref[0] = a.dot(normalize0(a));
		a = getAxis(m,1); ref[1] = a.dot(normalize0(a));
		a = getAxis(m,2); ref[2] = a.dot(normalize0(a));
		float cmp[3];
		anim.getValue( TicksToSec(ticks), cmp, 3 );
		float err = (Vector3(cmp[0],cmp[1],cmp[2]) - ref).length();
		if ( ref.length() > Float::MIN_VALUE )
			err /= ref.length();

		// sample more accurately if needed
		if ( err > maxErr && rangeLen > dticks )
		{
			TimeValue halfRange = alignTicks( (range.End() + range.Start())/2 );
			AnimExportUtil::addScaleKey( anim, tm[halfRange/dticks], TicksToSec(halfRange) );
			if ( ticks <= halfRange )
				resampleScaleKeys( anim, Interval(range.Start(),halfRange), maxErr, tm );
			else
				resampleScaleKeys( anim, Interval(halfRange,range.End()), maxErr, tm );
		}

		if ( ticks == range.End() )
			break;
	}
}

/** 
 * Resamples animation until maximum error is smaller than specified value. 
 * @param maxErr Maximum absolute error
 */
static void resampleFloatKeys( KeyFrameContainer& anim, Interval range, float maxErr, const Vector<float>& frames )
{
	TimeValue dticks = SGEXPORT_TICKS_PER_SAMPLE;
	int frame = range.Start() / dticks;
	require( frame >= 0 && frame < frames.size() );
	TimeValue rangeLen = (range.Duration()/SGEXPORT_TICKS_PER_SAMPLE)*SGEXPORT_TICKS_PER_SAMPLE;

	for ( TimeValue ticks = range.Start() ; ticks < range.End() ; ticks += dticks )
	{
		if ( ticks > range.End() )
			ticks = range.End();

		// find out error (distance) between real and sampled animation
		require( frame >= 0 && frame < frames.size() );
		float realValue = frames[frame++];
		float sampledValue = 0.f;
		anim.getValue( TicksToSec(ticks), &sampledValue, 1 );
		float err = Math::abs( realValue - sampledValue );

		// sample more accurately if needed
		if ( err > maxErr && rangeLen > dticks )
		{
			TimeValue halfRange = alignTicks( (range.End() + range.Start())/2 );
			anim.insertKey( KeyFrame( TicksToSec(halfRange), INTERP_TYPE, &frames[halfRange/dticks], 1 ) );

			if ( ticks <= halfRange )
				resampleFloatKeys( anim, Interval(range.Start(),halfRange), maxErr, frames );
			else
				resampleFloatKeys( anim, Interval(halfRange,range.End()), maxErr, frames );
		}

		if ( ticks == range.End() )
			break;
	}
}

/** Returns maximum column length squared. */
static Vector3 getSquaredScale( const Matrix3x3& m )
{
	Vector3 scalev;
	for ( int i = 0 ; i < 3 ; ++i )
	{
		float lensqr = m.getColumn(i).lengthSquared();
		scalev[i] = lensqr;
	}
	return scalev;
}

//-----------------------------------------------------------------------------

bool AnimExportUtil::isStdKeyControl( Control* cont )
{
	if ( !cont )
		return false;

	ulong partA = cont->ClassID().PartA();
	ulong partB = cont->ClassID().PartB();

	if ( 0 != partB )
		return false;

	switch ( partA ) 
	{
		case TCBINTERP_POSITION_CLASS_ID:
		case TCBINTERP_ROTATION_CLASS_ID:
		case TCBINTERP_SCALE_CLASS_ID:
		case HYBRIDINTERP_POSITION_CLASS_ID:
		case HYBRIDINTERP_ROTATION_CLASS_ID:
		case HYBRIDINTERP_SCALE_CLASS_ID:
		case LININTERP_POSITION_CLASS_ID:
		case LININTERP_ROTATION_CLASS_ID:
		case LININTERP_SCALE_CLASS_ID:
			return true;
	}

	return false;
}

void AnimExportUtil::getTransformAnimation( INode* node, Interval animRange, Vector<math::Matrix4x4>* anim )
{
	require( animRange.Start() <= animRange.End() );

	anim->clear();
	TimeValue dt = SGEXPORT_TICKS_PER_SAMPLE;
	for ( TimeValue t = animRange.Start() ; t <= animRange.End() ; t += dt )
	{
		Matrix4x4 tm = TmUtil::getModelToParentLH( node, t );
		anim->add( tm );
	}

	require( anim->size() > 0 );
}

void AnimExportUtil::addPositionKey( KeyFrameContainer& anim, const Matrix4x4& tm, float time )
{
	KeyFrame key;
	key.time = time;
	key.interpolation = INTERP_TYPE;
	key.setChannel( 0, tm(0,3) );
	key.setChannel( 1, tm(1,3) );
	key.setChannel( 2, tm(2,3) );
	anim.insertKey( key );
}

void AnimExportUtil::addRotationKey( KeyFrameContainer& anim, const Matrix4x4& tm, float time )
{
	Matrix3x3 rotm = tm.rotation().orthonormalize();
	Quaternion q( rotm );
	KeyFrame key;
	key.time = time;
	key.interpolation = INTERP_TYPE;
	key.setChannel( 0, q.x );
	key.setChannel( 1, q.y );
	key.setChannel( 2, q.z );
	key.setChannel( 3, q.w );
	anim.insertKey( key );
}

void AnimExportUtil::addScaleKey( KeyFrameContainer& anim, const Matrix4x4& tm, float time )
{
	KeyFrame key;
	key.time = time;
	key.interpolation = INTERP_TYPE;
	for ( int i = 0 ; i < 3 ; ++i )
	{
		Vector3 axis = getAxis( tm, i );
		float s = axis.dot( normalize0(axis) );
		key.setChannel( i, s );
	}
	anim.insertKey( key );
}

void AnimExportUtil::resamplePositionAnimation( const Vector<Matrix4x4>& tm, Interval animRange, KeyFrameContainer* pos )
{
	// find out maximum acceptable error
	float maxErrPercent = MAX_POSITION_RESAMPLING_ERROR;
	float defr = 1e9f;
	Vector3 minbox( defr, defr, defr );
	Vector3 maxbox(-defr,-defr,-defr );
	for ( int k = 0 ; k < tm.size() ; k += 2 )
	{
		Vector3 pos = tm[k].translation();
		minbox = pos.minElements( minbox );
		maxbox = pos.maxElements( maxbox );
	}
	float maxErr = (minbox-maxbox).length()/100.f * maxErrPercent;
	if ( maxErr < 1e-6f )
		maxErr = 1e-6f;

	int firstFrame = animRange.Start() / SGEXPORT_TICKS_PER_SAMPLE;
	require( firstFrame >= 0 && firstFrame < tm.size() );
	AnimExportUtil::addPositionKey( *pos, tm[firstFrame], TicksToSec(animRange.Start()) );
	AnimExportUtil::addPositionKey( *pos, tm.lastElement(), TicksToSec(animRange.End()) );
	Debug::println( "max position error = {0}", maxErr );
	resamplePositionKeys( *pos, animRange, maxErr, tm );
	if ( pos->keys() == 2 && isEqualValue(pos->getKey(0),pos->getKey(1),3) )
		pos->removeKey( 1 );
}

void AnimExportUtil::resampleRotationAnimation( const Vector<Matrix4x4>& tm, Interval animRange, KeyFrameContainer* rot )
{
	float maxErr = Math::toRadians(MAX_ROTATION_RESAMPLING_ERROR);
	int firstFrame = animRange.Start() / SGEXPORT_TICKS_PER_SAMPLE;
	require( firstFrame >= 0 && firstFrame < tm.size() );
	AnimExportUtil::addRotationKey( *rot, tm[firstFrame], TicksToSec(animRange.Start()) );
	AnimExportUtil::addRotationKey( *rot, tm.lastElement(), TicksToSec(animRange.End()) );
	Debug::println( "max rotation error = {0} degrees", Math::toDegrees(maxErr) );
	resampleRotationKeys( *rot, animRange, maxErr, tm );
	if ( rot->keys() == 2 && isEqualValue(rot->getKey(0),rot->getKey(1),4) )
		rot->removeKey( 1 );
}

void AnimExportUtil::resampleScaleAnimation( const Vector<Matrix4x4>& tm, Interval animRange, KeyFrameContainer* scale )
{
	float maxErrPercent = MAX_SCALE_RESAMPLING_ERROR;
	int firstFrame = animRange.Start() / SGEXPORT_TICKS_PER_SAMPLE;
	require( firstFrame >= 0 && firstFrame < tm.size() );
	AnimExportUtil::addScaleKey( *scale, tm[firstFrame], TicksToSec(animRange.Start()) );
	AnimExportUtil::addScaleKey( *scale, tm.lastElement(), TicksToSec(animRange.End()) );
	Debug::println( "max scale error = {0}%", maxErrPercent );
	resampleScaleKeys( *scale, animRange, maxErrPercent/100.f, tm );
	if ( scale->keys() == 2 && isEqualValue(scale->getKey(0),scale->getKey(1),3) )
		scale->removeKey( 1 );
}

void AnimExportUtil::resampleFloatAnimation( const util::Vector<float>& frames, Interval animRange, KeyFrameContainer* anim, float maxErr )
{
	int firstFrame = animRange.Start() / SGEXPORT_TICKS_PER_SAMPLE;
	require( firstFrame >= 0 && firstFrame < frames.size() );
	anim->insertKey( KeyFrame(TicksToSec(animRange.Start()),INTERP_TYPE,&frames[firstFrame],1) );
	anim->insertKey( KeyFrame(TicksToSec(animRange.End()),INTERP_TYPE,&frames.lastElement(),1) );
	resampleFloatKeys( *anim, animRange, maxErr, frames );
	if ( anim->keys() == 2 && isEqualValue(anim->getKey(0),anim->getKey(1),3) )
		anim->removeKey( 1 );
}

void AnimExportUtil::addPositionAnimation( const Vector<Matrix4x4>& tm, Interval animRange, KeyFrameContainer* pos )
{
	int firstFrame = animRange.Start() / SGEXPORT_TICKS_PER_SAMPLE;
	for ( TimeValue ticks = animRange.Start() ; ticks <= animRange.End() ; ticks += SGEXPORT_TICKS_PER_SAMPLE )
	{
		require( firstFrame >= 0 && firstFrame < tm.size() );
		AnimExportUtil::addPositionKey( *pos, tm[firstFrame++], TicksToSec(ticks) );
	}
}

void AnimExportUtil::addRotationAnimation( const Vector<Matrix4x4>& tm, Interval animRange, KeyFrameContainer* rot )
{
	int firstFrame = animRange.Start() / SGEXPORT_TICKS_PER_SAMPLE;
	for ( TimeValue ticks = animRange.Start() ; ticks <= animRange.End() ; ticks += SGEXPORT_TICKS_PER_SAMPLE )
	{
		require( firstFrame >= 0 && firstFrame < tm.size() );
		AnimExportUtil::addRotationKey( *rot, tm[firstFrame++], TicksToSec(ticks) );
	}
}

void AnimExportUtil::addScaleAnimation( const Vector<Matrix4x4>& tm, Interval animRange, KeyFrameContainer* scale )
{
	int firstFrame = animRange.Start() / SGEXPORT_TICKS_PER_SAMPLE;
	for ( TimeValue ticks = animRange.Start() ; ticks <= animRange.End() ; ticks += SGEXPORT_TICKS_PER_SAMPLE )
	{
		require( firstFrame >= 0 && firstFrame < tm.size() );
		AnimExportUtil::addScaleKey( *scale, tm[firstFrame++], TicksToSec(ticks) );
	}
}

void AnimExportUtil::addFloatAnimation( const util::Vector<float>& frames, Interval animRange, KeyFrameContainer* anim, float maxErr )
{
	int firstFrame = animRange.Start() / SGEXPORT_TICKS_PER_SAMPLE;
	for ( TimeValue ticks = animRange.Start() ; ticks <= animRange.End() ; ticks += SGEXPORT_TICKS_PER_SAMPLE )
	{
		require( firstFrame >= 0 && firstFrame < frames.size() );
		anim->insertKey( KeyFrame(TicksToSec(ticks),INTERP_TYPE,&frames[firstFrame++],1) );
	}
}

int AnimExportUtil::sampleRate()
{
	return SGEXPORT_SAMPLE_RATE;
}

void AnimExportUtil::isAnimated( const Vector<Matrix4x4>& anim, bool* animPos, bool* animRot, bool* animScl )
{
	bool posAnim = false;
	bool rotAnim = false;
	bool sclAnim = false;

	int animsize = anim.size();
	Vector3 pos0 = anim[0].translation();
	Matrix3x3 rot0 = anim[0].rotation();
	Vector3 scl0 = getSquaredScale( rot0 );
	for ( int k = 0 ; k < animsize ; ++k )
	{
		const Matrix4x4& tm = anim[k];
		Matrix3x3 rot = tm.rotation();
		if ( !posAnim )
		{
			Vector3 dpos = tm.translation() - pos0;
			if ( dpos.lengthSquared() > 1e-8f )
				posAnim = true;
		}
		if ( !rotAnim )
		{
			Matrix3x3 drot = rot - rot0;
			if ( getSquaredScale(drot).maxElement() > 1e-4f )
				rotAnim = true;
		}
		if ( !sclAnim )
		{
			Vector3 dscl = getSquaredScale(rot) - scl0;
			if ( Math::abs(dscl.maxElement()) > 1e-4f )
				sclAnim = true;
		}
	}

	*animPos = posAnim;
	*animRot = rotAnim;
	*animScl = sclAnim;
}

void AnimExportUtil::offsetKeyTimes( KeyFrameContainer* anim, float time0 )
{
	for ( int i = 0 ; i < anim->keys() ; ++i )
	{
		KeyFrame& key = anim->getKey(i);
		key.time = key.time - time0;
	}
}

Interval AnimExportUtil::getKeyFramedPositionRange( INode* node3ds, Interval animRange )
{
	require( node3ds->GetTMController() );

	TimeValue start = animRange.Start();
	TimeValue end = animRange.End();

	Control* cont = node3ds->GetTMController()->GetPositionController();
	IKeyControl* ikeys = cont ? GetKeyControlInterface( cont ) : 0;
	if ( ikeys )
	{
		ITCBPoint3Key key1;
		IBezPoint3Key key2;
		ILinPoint3Key key3;
		IKey* keyp = 0;

		if ( cont->ClassID() == Class_ID(TCBINTERP_POSITION_CLASS_ID, 0) )
			keyp = &key1;
		else if ( cont->ClassID() == Class_ID(HYBRIDINTERP_POSITION_CLASS_ID, 0) )
			keyp = &key2;
		else if ( cont->ClassID() == Class_ID(LININTERP_POSITION_CLASS_ID, 0) )
			keyp = &key3;

		if ( keyp )
		{
			if ( 0 == ikeys->GetNumKeys() )
				return Interval( start, start );

			ikeys->GetKey( 0, keyp );
			start = keyp->time;
			ikeys->GetKey( ikeys->GetNumKeys()-1, keyp );
			end = keyp->time;
		}
	}

	return Interval( start, end );
}

Interval AnimExportUtil::getKeyFramedRotationRange( INode* node3ds, Interval animRange )
{
	require( node3ds->GetTMController() );

	TimeValue start = animRange.Start();
	TimeValue end = animRange.End();

	// target defines rotation
	if ( node3ds->GetTarget() )
		return Interval( start, start );

	Control* cont = node3ds->GetTMController()->GetRotationController();
	IKeyControl* ikeys = cont ? GetKeyControlInterface( cont ) : 0;
	if ( ikeys )
	{
		ITCBRotKey key1;
		IBezQuatKey key2;
		ILinRotKey key3;
		IKey* keyp = 0;

		if ( cont->ClassID() == Class_ID(TCBINTERP_ROTATION_CLASS_ID, 0) )
			keyp = &key1;
		else if ( cont->ClassID() == Class_ID(HYBRIDINTERP_ROTATION_CLASS_ID, 0) )
			keyp = &key2;
		else if ( cont->ClassID() == Class_ID(LININTERP_ROTATION_CLASS_ID, 0) )
			keyp = &key3;

		if ( keyp )
		{
			if ( 0 == ikeys->GetNumKeys() )
				return Interval( start, start );

			ikeys->GetKey( 0, keyp );
			start = keyp->time;
			ikeys->GetKey( ikeys->GetNumKeys()-1, keyp );
			end = keyp->time;
		}
	}

	return Interval( start, end );
}

Interval AnimExportUtil::getKeyFramedScaleRange( INode* node3ds, Interval animRange )
{
	require( node3ds->GetTMController() );

	TimeValue start = animRange.Start();
	TimeValue end = animRange.End();

	Control* cont = node3ds->GetTMController()->GetScaleController();
	IKeyControl* ikeys = cont ? GetKeyControlInterface( cont ) : 0;
	if ( ikeys )
	{
		ITCBScaleKey key1;
		IBezScaleKey key2;
		ILinScaleKey key3;
		IKey* keyp = 0;

		if ( cont->ClassID() == Class_ID(TCBINTERP_SCALE_CLASS_ID, 0) )
			keyp = &key1;
		else if ( cont->ClassID() == Class_ID(HYBRIDINTERP_SCALE_CLASS_ID, 0) )
			keyp = &key2;
		else if ( cont->ClassID() == Class_ID(LININTERP_SCALE_CLASS_ID, 0) )
			keyp = &key3;

		if ( keyp )
		{
			if ( 0 == ikeys->GetNumKeys() )
				return Interval( start, start );

			ikeys->GetKey( 0, keyp );
			start = keyp->time;
			ikeys->GetKey( ikeys->GetNumKeys()-1, keyp );
			end = keyp->time;
		}
	}

	return Interval( start, end );
}

