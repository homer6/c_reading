#include <util/Vector.h>
#include <math/Matrix4x4.h>


class KeyFrameContainer;


/** 
 * Utilities for 3DS Max animation exporting. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class AnimExportUtil
{
public:
	/** Percent of bounding box length created from position keys. */
	static float	MAX_POSITION_RESAMPLING_ERROR;

	/** Degrees. */
	static float	MAX_ROTATION_RESAMPLING_ERROR;

	/** Percent of scale. */
	static float	MAX_SCALE_RESAMPLING_ERROR;

	/** Percent of morph target weight. */
	static float	MAX_MORPH_RESAMPLING_ERROR;

	/** 
	 * Samples transformation animation frames. 
	 * See sampleRate() to get number of samples per second.
	 */
	static void			getTransformAnimation( INode* node, Interval animRange, util::Vector<math::Matrix4x4>* anim );

	/** 
	 * Resamples position animation from the transformation frames. 
	 * Note that the vector of frames must contain FULL range of animation,
	 * ignoring current start frame. This means that tm[0] must be state of frame 0 even though
	 * current Start frame would be 100.
	 */
	static void			resamplePositionAnimation( const util::Vector<math::Matrix4x4>& tm, Interval animRange, KeyFrameContainer* pos );

	/** 
	 * Resamples rotation animation from the transformation frames. 
	 * Note that the vector of frames must contain FULL range of animation,
	 * ignoring current start frame. This means that tm[0] must be state of frame 0 even though
	 * current Start frame would be 100.
	 */
	static void			resampleRotationAnimation( const util::Vector<math::Matrix4x4>& tm, Interval animRange, KeyFrameContainer* rot );

	/** 
	 * Resamples scale animation from the transformation frames. 
	 * Note that the vector of frames must contain FULL range of animation,
	 * ignoring current start frame. This means that tm[0] must be state of frame 0 even though
	 * current Start frame would be 100.
	 */
	static void			resampleScaleAnimation( const util::Vector<math::Matrix4x4>& tm, Interval animRange, KeyFrameContainer* scale );

	/** 
	 * Resamples scalar value animation from the frames. 
	 * Note that the vector of frames must contain FULL range of animation,
	 * ignoring current start frame. This means that tm[0] must be state of frame 0 even though
	 * current Start frame would be 100.
	 * @param maxErr Maximum absolute error.
	 */
	static void			resampleFloatAnimation( const util::Vector<float>& frames, Interval animRange, KeyFrameContainer* anim, float maxErr );

	/** 
	 * Adds position animation from the transformation frames. 
	 * Note that the vector of frames must contain FULL range of animation,
	 * ignoring current start frame. This means that tm[0] must be state of frame 0 even though
	 * current Start frame would be 100.
	 */
	static void			addPositionAnimation( const util::Vector<math::Matrix4x4>& tm, Interval animRange, KeyFrameContainer* pos );

	/** 
	 * Adds rotation animation from the transformation frames. 
	 * Note that the vector of frames must contain FULL range of animation,
	 * ignoring current start frame. This means that tm[0] must be state of frame 0 even though
	 * current Start frame would be 100.
	 */
	static void			addRotationAnimation( const util::Vector<math::Matrix4x4>& tm, Interval animRange, KeyFrameContainer* rot );

	/** 
	 * Adds scale animation from the transformation frames. 
	 * Note that the vector of frames must contain FULL range of animation,
	 * ignoring current start frame. This means that tm[0] must be state of frame 0 even though
	 * current Start frame would be 100.
	 */
	static void			addScaleAnimation( const util::Vector<math::Matrix4x4>& tm, Interval animRange, KeyFrameContainer* scale );

	/** 
	 * Adds scalar value animation from the frames. 
	 * Note that the vector of frames must contain FULL range of animation,
	 * ignoring current start frame. This means that tm[0] must be state of frame 0 even though
	 * current Start frame would be 100.
	 * @param maxErr Maximum absolute error.
	 */
	static void			addFloatAnimation( const util::Vector<float>& frames, Interval animRange, KeyFrameContainer* anim, float maxErr );

	/** Offsets key frame times so that exported animation range starts at time 0. */
	static void			offsetKeyTimes( KeyFrameContainer* anim, float time0 );

	/** Adds a key frame to the animation. */
	static void			addPositionKey( KeyFrameContainer& anim, const math::Matrix4x4& tm, float time );

	/** Adds a key frame to the animation. */
	static void			addRotationKey( KeyFrameContainer& anim, const math::Matrix4x4& tm, float time );

	/** Adds a key frame to the animation. */
	static void			addScaleKey( KeyFrameContainer& anim, const math::Matrix4x4& tm, float time );

	/** Returns number of samples per second in raw sampled animations. */
	static int			sampleRate();

	/** Returns true if the controller is TCB, Bezier or linear controller. */
	static bool			isStdKeyControl( Control* cont );

	/** Gets info is position, rotation or scale animated. */
	static void			isAnimated( const util::Vector<math::Matrix4x4>& anim, bool* animPos, bool* animRot, bool* animScl );

	/** 
	 * Gets the key framed range of position animation. 
	 * @exception Exception If the key-framed animation controller is unsupported type.
	 */
	static Interval		getKeyFramedPositionRange( INode* node3ds, Interval animRange );

	/** 
	 * Gets the key framed range of rotation animation. 
	 * @exception Exception If the key-framed animation controller is unsupported type.
	 */
	static Interval		getKeyFramedRotationRange( INode* node3ds, Interval animRange );

	/** 
	 * Gets the key framed range of scale animation. 
	 * @exception Exception If the key-framed animation controller is unsupported type.
	 */
	static Interval		getKeyFramedScaleRange( INode* node3ds, Interval animRange );
};
