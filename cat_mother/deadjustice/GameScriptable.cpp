#include "GameScriptable.h"
#include "ScriptUtil.h"
#include <io/InputStream.h>
#include <io/ByteArrayOutputStream.h>
#include <io/InputStreamArchive.h>
#include <io/ByteArrayOutputStream.h>
#include <ps/ParticleSystem.h>
#include <ps/ParticleSystemManager.h>
#include <sg/Mesh.h>
#include <sg/ShadowVolume.h>
#include <sg/Light.h>
#include <sg/Camera.h>
#include <sg/Material.h>
#include <sg/Primitive.h>
#include <sg/Context.h>
#include <gd/GraphicsDevice.h>
#include <sgu/NodeUtil.h>
#include <snd/Sound.h>
#include <snd/SoundManager.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include <util/lowerBound.h>
#include <math/Vector3.h>
#include <script/VM.h>
#include <script/ClassTag.h>
#include <script/ScriptException.h>
#include <algorithm>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace ps;
using namespace sg;
using namespace sgu;
using namespace snd;
using namespace lang;
using namespace util;
using namespace math;
using namespace anim;
using namespace script;

//-----------------------------------------------------------------------------

ScriptMethod<GameScriptable> GameScriptable::sm_methods[] =
{
	//ScriptMethod<GameScriptable>( "funcName", script_funcName ),
	ScriptMethod<GameScriptable>( "addNonLinearTextureAnimation", script_addNonLinearTextureAnimation ),
	ScriptMethod<GameScriptable>( "addTextureAnimation", script_addTextureAnimation ),
	ScriptMethod<GameScriptable>( "addTimerEvent", script_addTimerEvent ),
	ScriptMethod<GameScriptable>( "addTimerWaitCondition", script_addTimerWaitCondition ),
	ScriptMethod<GameScriptable>( "fadeInSoundAt", script_fadeInSoundAt ),
	ScriptMethod<GameScriptable>( "fadeOutSound", script_fadeOutSound ),
	ScriptMethod<GameScriptable>( "getTimerEventCount", script_getTimerEventCount ),
	ScriptMethod<GameScriptable>( "getTextureAnimationTime", script_getTextureAnimationTime ),
	ScriptMethod<GameScriptable>( "getRandomInteger", script_getRandomInteger ),
	ScriptMethod<GameScriptable>( "include", script_include ),
	ScriptMethod<GameScriptable>( "loadSound", script_loadSound ),
	ScriptMethod<GameScriptable>( "name", script_name ),
	ScriptMethod<GameScriptable>( "playParticleSystem", script_playParticleSystem ),
	ScriptMethod<GameScriptable>( "playParticleSystemAt", script_playParticleSystemAt ),
	ScriptMethod<GameScriptable>( "playDirectedParticleSystemAt", script_playDirectedParticleSystemAt ),
	ScriptMethod<GameScriptable>( "playSound", script_playSound ),
	ScriptMethod<GameScriptable>( "playSoundAt", script_playSoundAt ),
	ScriptMethod<GameScriptable>( "playSoundOffset", script_playSoundOffset ),
	ScriptMethod<GameScriptable>( "playTextureAnimation", script_playTextureAnimation ),
	ScriptMethod<GameScriptable>( "setName", script_setName ),
	ScriptMethod<GameScriptable>( "random", script_random ),
	ScriptMethod<GameScriptable>( "removeTimerEvents", script_removeTimerEvents ),
	ScriptMethod<GameScriptable>( "setSeed", script_setSeed ),
	ScriptMethod<GameScriptable>( "stopSound", script_stopSound ),
	ScriptMethod<GameScriptable>( "stopTextureAnimation", script_stopTextureAnimation ),
};

//-----------------------------------------------------------------------------

GameScriptable::GameScriptable( VM* vm, io::InputStreamArchive* arch, 
	snd::SoundManager* soundMgr, ps::ParticleSystemManager* particleMgr ) :
	Scriptable( vm, ClassTag<GameScriptable>::getTag(vm) ),
	m_methodBase( -1 ),
	m_name( "" ),
	m_arch( arch ),
	m_soundMgr( soundMgr ),
	m_particleMgr( particleMgr ),
	m_rng(),
	m_texAnims( Allocator<P(TextureAnimation)>(__FILE__) ),
	m_timerEvents( Allocator<TimerEvent>(__FILE__) ),
	m_dt( 0.f )
{
	if ( vm )
		m_methodBase = ScriptUtil<GameScriptable,Scriptable>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

GameScriptable::~GameScriptable()
{
	removeTimerEvents();
}

int GameScriptable::getParam( VM* vm, const char* funcName, const char* type, float* num )
{
	return getParams( vm, funcName, type, num, 1 );
}

int GameScriptable::getParams( VM* vm, const char* funcName, const char* type, float* nums, int n )
{
	for ( int i = 0 ; i < n ; ++i )
	{
		if ( vm->getType(i+1) != VM::TYPE_NUMBER )
		{
			if ( n > 1 )
				throw ScriptException( Format("{0} expects {1}", funcName, type, n) );
			else
				throw ScriptException( Format("{0} expects {1}", funcName, type) );
		}
		nums[i] = vm->toNumber(i+1);
	}
	return 0;
}

const String& GameScriptable::name() const
{
	return m_name;
}

void GameScriptable::update( float dt )
{
	m_dt += dt;

	// signal timer events
	VM* vm = this->vm();
	for ( int i = 0 ; i < m_timerEvents.size() ; ++i )
	{
		TimerEvent& te = m_timerEvents[i];
		te.time -= dt;

		if ( te.time <= 0.f )
		{
			if ( vm->getRef(te.scriptFuncRef) )
			{
				if ( te.condition )
				{
					// wait for condition to become true
					vm->pushTable( this );
					call(0,1);
					if ( vm->top() > 0 )
					{
						bool timerPaused = vm->isNil(1);
						vm->pop();
						if ( timerPaused )
							break;
					}
				}
				else
				{
					// normal timer event
					vm->pushTable( this );
					call(0,0);
				}
			}
			vm->unref( te.scriptFuncRef );
			m_timerEvents.remove(i--);
		}
	}
}

sg::Node* GameScriptable::getRenderObject( sg::Camera* camera )
{
	// update texture animations
	if ( camera && m_dt > 0.f )
	{
		for ( int i = 0 ; i < m_texAnims.size() ; ++i )
		{
			TextureAnimation* texAnim = m_texAnims[i];
			if ( texAnim->time >= 0.f )
			{
				texAnim->time += m_dt;

				// get current frame
				float frameNumf = 0.f;
				texAnim->hint = texAnim->frameCtrl->getValue( texAnim->time, &frameNumf, 1, texAnim->hint );
				int frameNum = (int)( frameNumf + .5f );
				if ( frameNum < 0 )
					frameNum = 0;
				if ( frameNum > texAnim->lastFrame + 1 )
					frameNum = texAnim->lastFrame + 1;
				if ( frameNum >= texAnim->frames->frames() )
					frameNum = texAnim->frames->frames()-1;
				Texture* frame = texAnim->frames->getFrame( frameNum );

				// apply frame
				for ( int k = 0 ; k < texAnim->materials.size() ; ++k )
				{
					Material* mat = texAnim->materials[k];
					mat->setTexture( 0, frame );
				}

				// store frame number
				texAnim->lastFrame = frameNum;
			}
		}

		m_dt = 0.f;
	}
	return 0;
}

int	GameScriptable::getIndex( script::VM* vm, const char* funcName, int begin, int end, int param )
{
	if ( !vm->isNumber(param) )
		throw ScriptException( Format("Function {0} expects valid index in range [{1},{2}) as {3}.parameter", funcName, begin, end, param) );

	float v = vm->toNumber( param );
	int index = (int)v;
	if ( !(index >= begin && index < end) )
		throw ScriptException( Format("Function {0} expects valid index in range [{1},{2})", funcName, begin, end) );
	
	return index;
}

void GameScriptable::compile( const lang::String& scriptName )
{
	// read stream to byte (ASCII-7) buffer
	P(InputStream) in = m_arch->getInputStream( scriptName );
	ByteArrayOutputStream bout( in->available() );
	while ( in->available() > 0 )
	{
		char buf[16];
		int read = in->read( buf, sizeof(buf) );
		if ( read <= 0 )
			break;
		bout.write( buf, read );
	}
	in->close();

	// compile byte buffer
	String scriptNameStr = scriptName; // this is needed as user migh pass source()
	compileBuffer( bout.toByteArray(), bout.size(), scriptNameStr );

	// call script 'init' function
	pushMethod( "init" );
	call( 0, 0 );
}

int GameScriptable::getTimerEventCount( const lang::String& str ) const
{
	VM* vm = this->vm();
	vm->pushTable( this );
	int tabIndex = vm->top();
	int count = 0;

	for ( int i = 0 ; i < m_timerEvents.size() ; ++i )
	{
		const TimerEvent& te = m_timerEvents[i];

		if ( vm->getRef(te.scriptFuncRef) )
		{
			// check agains every function in the table
			int eventIndex = vm->top();
			vm->pushNil();
			while ( vm->next(tabIndex) )
			{
				if ( vm->isEqual(-1,eventIndex) )
				{
					// (-2,-1) key-value pair matches the event function, check the name
					String name = vm->toString(-2);
					if ( name.startsWith(str) )
					{
						// match!
						++count;
					}
				}
				vm->pop(); // iterator value
			}
			vm->pop(); // event
		}
	}

	vm->pop();
	return count;
}

int GameScriptable::removeTimerEvents()
{
	int removed = 0;

	if ( initialized() )
	{
		for ( int i = 0 ; i < m_timerEvents.size() ; ++i )
			vm()->unref( m_timerEvents[i].scriptFuncRef );

		removed = m_timerEvents.size();
		m_timerEvents.clear();
	}
	
	return removed;
}

sg::Light* GameScriptable::keylight() const
{
	return 0;
}

int GameScriptable::removeTimerEvents( const lang::String& str )
{
	assert( vm() );

	int removed = 0;
	VM* vm = this->vm();
	vm->pushTable( this );
	int tabIndex = vm->top();

	for ( int i = 0 ; i < m_timerEvents.size() ; ++i )
	{
		TimerEvent& te = m_timerEvents[i];

		if ( vm->getRef(te.scriptFuncRef) )
		{
			// check agains every function in the table
			int eventIndex = vm->top();
			vm->pushNil();
			while ( vm->next(tabIndex) )
			{
				if ( vm->isEqual(-1,eventIndex) )
				{
					// (-2,-1) key-value pair matches the event function, check the name
					String name = vm->toString(-2);
					if ( name.startsWith(str) )
					{
						// match! remove the event
						Debug::println( "removed timer event {0}", name );
						vm->unref( te.scriptFuncRef );
						m_timerEvents.remove( i-- );
						++removed;
						break;
					}
				}
				vm->pop(); // iterator value
			}
			vm->pop(); // event
		}
	}

	vm->pop();

	if ( 0 == removed )
		Debug::printlnWarning( "Tried to removeTimerEvents({0}) from {1}, but none found", str, name() );
	return removed;
}

float GameScriptable::random()
{
	static Random s_rng;
	return s_rng.nextFloat();
}

Node* GameScriptable::getAnchorNode( VM* vm, const char* funcName, int stackIndex )
{
	Node*		root	= getRenderObject(0);
	Node*		anchor	= root;

	if ( vm->top() == stackIndex )
	{
		// root node space particle system
	}
	else if ( vm->top() == stackIndex+1 && vm->isString(stackIndex+1) )
	{
		// child node space particle system
		String bone = vm->toString( stackIndex+1 );
		anchor = NodeUtil::findNodeByName( root, bone );
		if ( !anchor )
			throw ScriptException( Format("Node {0} not found from {1}", bone, name()) );
	}
	else if ( ( vm->top() == stackIndex+1 && vm->isNil(stackIndex+1) ) )
	{
		anchor = 0;
	}
	else
	{
		throw ScriptException( Format("{0} expects effect name and optional node name or nil", funcName) );
	}

	return anchor;
}

TextureAnimation* GameScriptable::getTextureAnimation( script::VM* vm, const char* funcName, int stackIndex )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects texture animation name", funcName) );

	String animName = vm->toString(stackIndex);
	
	for ( int i = 0 ; i < m_texAnims.size() ; ++i )
	{
		TextureAnimation* texAnim = m_texAnims[i];
		if ( texAnim->name == animName )
			return texAnim;
	}

	throw ScriptException( Format("{0} expects valid texture animation name", funcName) );
	return 0;
}

void GameScriptable::removeLightsAndCameras( Node* root )
{
	P(Node) rootNode = root;
	for ( Node* node = root ; node ; node = node->nextInHierarchy() )
	{
		if ( dynamic_cast<Light*>(node) )
		{
			Debug::printlnWarning( "Removed light {1} from {0}", root->name(), node->name() );
			node->unlink();
			node = root;
		}
		if ( dynamic_cast<Camera*>(node) )
		{
			Debug::printlnWarning( "Removed camera {1} from {0}", root->name(), node->name() );
			node->unlink();
			node = root;
		}
	}
}

void GameScriptable::replaceLightmapMaterialsWithShader( Node* scene, Shader* shader )
{
	// replace lightmap materials with lightmap shader
	for ( Node* node = scene ; node ; node = node->nextInHierarchy() )
	{
		Mesh* mesh = dynamic_cast<Mesh*>( node );
		if ( mesh )
		{
			for ( int i = 0 ; i < mesh->primitives() ; ++i )
			{
				P(Primitive) prim = mesh->getPrimitive(i);
				P(Material) mat = dynamic_cast<Material*>( prim->shader() );
				if ( mat && !mat->lighting() && mat->isTextureLayerEnabled(0) && mat->isTextureLayerEnabled(1) && !mat->isTextureLayerEnabled(2) &&
					mat->sourceBlend() == Material::BLEND_ONE && mat->destinationBlend() == Material::BLEND_ZERO )
				{
					Material::TextureArgument arg1, arg2;
					Material::TextureOperation op;
					mat->getTextureColorCombine( 0, &arg1, &op, &arg2 );
					if ( arg1 == Material::TA_TEXTURE && arg2 == Material::TA_DIFFUSE && op == Material::TOP_MODULATE )
					{
						mat->getTextureColorCombine( 1, &arg1, &op, &arg2 );
						if ( arg1 == Material::TA_TEXTURE && arg2 == Material::TA_CURRENT && op == Material::TOP_MODULATE )
						{
							Debug::println( "Replacing lightmap material {0} with shader", mat->name() );

							P(BaseTexture) dif = mat->getTexture(0);
							P(BaseTexture) lmap = mat->getTexture(1);

							P(Shader) fx = shader->clone();
							VertexFormat vf = prim->vertexFormat();
							assert( vf.hasNormal() && vf.textureCoordinates() == 2 && vf.getTextureCoordinateSize(0) == 2 && vf.getTextureCoordinateSize(1) == 2 );
							fx->setVertexFormat( vf );
							fx->setTexture( "tDiffuse", dif );
							fx->setTexture( "tLightMap", lmap );
							fx->setName( mat->name() );

							prim->setShader( fx );
						}
					}
				}
			}
		}
	}
}

void GameScriptable::setName( const String& name )
{
	m_name = name;
}

int GameScriptable::methodCall( VM* vm, int i )
{
	return ScriptUtil<GameScriptable,Scriptable>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

int GameScriptable::script_setName( VM* vm, const char* funcName )
{
	if ( vm->top() != 1 || vm->getType(1) != VM::TYPE_STRING )
		throw ScriptException( Format("{0} expects a name string", funcName) );

	m_name = vm->toString(1);
	return 0;
}

int GameScriptable::script_name( VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns name of the object as set by setName(<string>)", funcName) );
	vm->pushString( m_name );
	return 1;
}

int	GameScriptable::script_include( VM* vm, const char* funcName )
{
	if ( vm->getType(1) != VM::TYPE_STRING )
		throw ScriptException( Format("{0} expects script name", funcName) );
	String scriptName = vm->toString(1);

	// read stream to byte (ASCII-7) buffer
	P(InputStream) in = m_arch->getInputStream( scriptName );
	ByteArrayOutputStream bout( in->available() );
	while ( in->available() > 0 )
	{
		char buf[16];
		int read = in->read( buf, sizeof(buf) );
		if ( read <= 0 )
			break;
		bout.write( buf, read );
	}
	in->close();

	compileBuffer( bout.toByteArray(), bout.size(), scriptName );
	return 0;
}

int GameScriptable::script_random( VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns random in half-open range [0,1)", funcName) );
	vm->pushNumber( random() );
	return 1;
}

int GameScriptable::script_setSeed( VM* vm, const char* funcName )
{
	float v;
	getParam( vm, funcName, "random seed", &v );
	m_rng.setSeed( (long)v );
	return 0;
}

int GameScriptable::script_playTextureAnimation( VM* vm, const char* funcName )
{
	TextureAnimation* anim = getTextureAnimation( vm, funcName );
		//Debug::println( "Started texture animation {0}", anim->name );
	anim->time = 0.f;
	anim->lastFrame = 0;
	return 0;
}

int GameScriptable::script_stopTextureAnimation( VM* vm, const char* funcName )
{
	TextureAnimation* anim = getTextureAnimation( vm, funcName );
	//Debug::println( "Stopped texture animation {0} at frame {1}, time direction {2}", anim->name, 30.f*anim->frameCtrl->getNormalizedTime(anim->time), anim->frameCtrl->getTimeDirection(anim->time) );
	anim->time = -1.f;
	return 0;
}

int GameScriptable::script_getTextureAnimationTime( VM* vm, const char* funcName )
{
	TextureAnimation* anim = getTextureAnimation( vm, funcName );

	float time0 = anim->time;
	if ( time0 < 0.f )
		time0 = 0.f;
	
	float normtime = anim->frameCtrl->getNormalizedTime( time0 );
	float endTime = anim->frameCtrl->getKeyTime( anim->frameCtrl->keys()-1 );

	float left = endTime - normtime;
	if ( anim->frameCtrl->endBehaviour() == Interpolator::BEHAVIOUR_OSCILLATE )
	{
		if ( anim->frameCtrl->getTimeDirection(time0) > 0.f )
			left += endTime;
		else
			left = normtime;
	}

	vm->pushNumber( left );
	return 1;
}

int GameScriptable::script_addNonLinearTextureAnimation( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_STRING, VM::TYPE_TABLE};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects new animation name, source animation name and frame table", funcName) );
	
	int param = 1;
	String	animName	= vm->toString(param++);
	String	srcName		= vm->toString(param++);
	Table	frametab	= vm->toTable(param++);
	float	fps			= vm->toNumber(param++);

	// create copy of old animation
	TextureAnimation* srcAnim = getTextureAnimation( vm, funcName, 2 );
	P(TextureAnimation) anim = srcAnim;
	if ( animName != srcAnim->name )
		anim = new TextureAnimation( *srcAnim );
	anim->name = animName;
	anim->frameCtrl = new VectorInterpolator( *srcAnim->frameCtrl );

	// read frame numbers from the table
	int n = frametab.size();
	anim->frameCtrl->setKeys( n+1 );
	for ( int i = 1 ; i <= n+1 ; ++i )
	{
		float time = (float)(i-1) / fps;
		
		int index = i;
		if ( index > n )
			index = n;
		float frame = frametab.getNumber( index );
		if ( frame < 0 || (int)frame >= anim->frames->frames() )
			throw ScriptException( Format("{0} frame table item out of range ({1})", funcName, frame) );

		anim->frameCtrl->setKeyTime( i-1, time );
		anim->frameCtrl->setKeyValue( i-1, &frame, 1 );
	}

	if ( anim != srcAnim )
		m_texAnims.add( anim );
	return 0;
}

int GameScriptable::script_addTextureAnimation( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_STRING, VM::TYPE_STRING, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects animation name, target material name, image file name format string, first image index, last image index (inclusive), playback frame rate and end behaviour type (REPEAT, CONSTANT, OSCILLATE)", funcName) );

	int param = 1;
	String	animName = vm->toString(param++);
	String	matName = vm->toString(param++);
	String	imgNameFmt = vm->toString(param++);
	int		firstImg = (int)( vm->toNumber(param++) + 0.5f );
	int		lastImg = (int)( vm->toNumber(param++) + 0.5f );
	int		frames =  lastImg - firstImg + 1;
	float	fps = vm->toNumber(param++);
	String	endBehaviour = String(vm->toString(param++)).toUpperCase();

	if ( frames < 1 || fps < 1e-3f )
		throw ScriptException( Format("{0} expects animation name, target material name, image file name format, first image index, last image index (inclusive), playback frame rate and end behaviour type (REPEAT, CONSTANT, OSCILLATE)", funcName) );

	// load animation texture sequence
	P(TextureSequence) textures = new TextureSequence( m_arch, animName, imgNameFmt, firstImg, lastImg );

	// create frame controller
	P(VectorInterpolator) frameCtrl = new VectorInterpolator(1);
	frameCtrl->setInterpolation( VectorInterpolator::INTERPOLATE_STEPPED );
	
	// create default frame list
	frameCtrl->setKeys( frames+1 );
	for ( int i = 0 ; i < frames+1 ; ++i )
	{
		float frame = (float)i;
		float time = frame / fps;
		if ( frame >= frames )
			frame = frames-1.f;
		frameCtrl->setKeyTime( i, time );
		frameCtrl->setKeyValue( i, &frame, 1 );
	}

	// animation end behaviour
	Interpolator::BehaviourType behaviour = Interpolator::BEHAVIOUR_REPEAT;
	if ( endBehaviour == "RESET" )
		behaviour = Interpolator::BEHAVIOUR_RESET;
	else if ( endBehaviour == "CONSTANT" )
		behaviour = Interpolator::BEHAVIOUR_CONSTANT;
	else if ( endBehaviour == "REPEAT" )
		behaviour = Interpolator::BEHAVIOUR_REPEAT;
	else if ( endBehaviour == "OSCILLATE" )
		behaviour = Interpolator::BEHAVIOUR_OSCILLATE;
	frameCtrl->setEndBehaviour( behaviour );

	// create texture animation object
	P(TextureAnimation) texAnim = new TextureAnimation;
	texAnim->name = animName;
	texAnim->frames = textures;
	texAnim->frameCtrl = frameCtrl;
	
	// find target materials
	for ( Node* node = getRenderObject(0) ; node ; node = node->nextInHierarchy() )
	{
		Mesh* mesh = dynamic_cast<Mesh*>( node );
		if ( mesh )
		{
			for ( int i = 0 ; i < mesh->primitives() ; ++i )
			{
				Primitive* prim = mesh->getPrimitive( i );
				Shader* shader = prim->shader();
				if ( shader && shader->name() == matName )
				{
					Material* mat = dynamic_cast<Material*>( shader );
					if ( mat )
						texAnim->materials.add( mat );
				}
			}
		}
	}

	if ( 0 == texAnim->materials.size() )
		throw ScriptException( Format("{0} target material {1} not found", funcName, matName) );

	m_texAnims.add( texAnim );
	return 0;
}

float GameScriptable::screenWidth() const
{
	gd::GraphicsDevice* dev = Context::device();
	assert( dev );
	if ( dev )
		return (float)dev->width();
	return 0.f;
}

float GameScriptable::screenHeight() const
{
	gd::GraphicsDevice* dev = Context::device();
	assert( dev );
	if ( dev )
		return (float)dev->height();
	return 0.f;
}

void GameScriptable::setRenderPasses( Node* root, int solidPass, int transparentPass )
{
	for ( Node* node = root ; node ; node = node->nextInHierarchy() )
	{
		Mesh* mesh = dynamic_cast<Mesh*>( node );
		if ( mesh )
		{
			for ( int i = 0 ; i < mesh->primitives() ; ++i )
			{
				Primitive* prim = mesh->getPrimitive(i);
				if ( !dynamic_cast<ShadowVolume*>(prim) )
				{
					Shader* shader = prim->shader();
					if ( shader )
					{
						if ( shader->name().indexOf("SORT") != -1 )
						{
							shader->setPass( transparentPass );
						}
						else
						{
							Material* mat = dynamic_cast<Material*>( shader );
							if ( mat && mat->destinationBlend() != Material::BLEND_ZERO )
							{
								shader->setPass( transparentPass );
							}
							else
							{
								shader->setPass( solidPass );
							}
						}
					}
				}
			}
		}
	}
}

int GameScriptable::script_addTimerEvent( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_FUNCTION, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects function and time in seconds", funcName) );

	TimerEvent te;
	vm->pushValue(1);
	te.time = vm->toNumber(2);
	te.scriptFuncRef = vm->ref( true );
	te.condition = false;
	m_timerEvents.add( te );
	// breaks cut scenes:
	//std::sort( m_timerEvents.begin(), m_timerEvents.end() );
	return 0;
}

int GameScriptable::script_addTimerWaitCondition( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_FUNCTION, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects condition function (which returns false if condition should be still waited) and time in seconds", funcName) );

	TimerEvent te;
	vm->pushValue(1);
	te.time = vm->toNumber(2);
	te.scriptFuncRef = vm->ref( true );
	te.condition = true;
	m_timerEvents.add( 0, te );
	std::sort( m_timerEvents.begin(), m_timerEvents.end() );
	return 0;
}

int GameScriptable::script_removeTimerEvents( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects name start string of timer event functions to remove", funcName) );

	removeTimerEvents( vm->toString(1) );
	return 0;
}

int GameScriptable::script_playParticleSystem( VM* vm, const char* funcName )
{
	if ( vm->top() < 1 || !vm->isString(1) )
		throw ScriptException( Format("Functions {0} expects particle system name (without .psf suffix) and optional parent object name", funcName) );

	if ( !m_particleMgr )
		throw ScriptException( Format("Object {0} cannot play particle systems", name()) );

	String		name	= vm->toString(1) + String(".psf");
	P(Node)		anchor	= getAnchorNode( vm, funcName, 1 );

	if ( m_particleMgr->getActiveCount(name,anchor) > 10 )
	{
		Debug::printlnWarning( Format( "More than 10 copies of {0} particle system on anchor {1}.", name, anchor->name() ).format() );
	}
	m_particleMgr->play( name, anchor );
	return 0;
}

int	GameScriptable::script_playParticleSystemAt( script::VM* vm, const char* funcName )
{
	if ( vm->top() < 4 || !vm->isString(1) || !vm->isNumber(2) || !vm->isNumber(3) || !vm->isNumber(4) )
		throw ScriptException( Format("Functions {0} expects particle system description name (without .psf suffix), (x,y,z) offset and optional parent object name", funcName) );

	if ( !m_particleMgr )
		throw ScriptException( Format("Object {0} cannot play particle systems", name()) );
	
	String		name	= vm->toString(1) + String(".psf");
	Vector3		offset	= Vector3( vm->toNumber(2), vm->toNumber(3), vm->toNumber(4) );
	P(Node)		anchor	= getAnchorNode( vm, funcName, 4 );

	ParticleSystem* ps = m_particleMgr->play( name, anchor );
	ps->setPosition( offset );
	return 0;
}

int	GameScriptable::script_playDirectedParticleSystemAt( script::VM* vm, const char* funcName )
{
	if ( vm->top() < 7 || !vm->isString(1) || !vm->isNumber(2) || !vm->isNumber(3) || !vm->isNumber(4) || !vm->isNumber(5) || !vm->isNumber(6) || !vm->isNumber(7) )
		throw ScriptException( Format("Functions {0} expects particle system description name (without .psf suffix), (x,y,z) offset, (x,y,z) direction vector and optional parent object name", funcName) );

	if ( !m_particleMgr )
		throw ScriptException( Format("Object {0} cannot play particle systems", name()) );
	
	String		name	= vm->toString(1) + String(".psf");
	Vector3		offset	= Vector3( vm->toNumber(2), vm->toNumber(3), vm->toNumber(4) );
	Vector3		dir		= Vector3( vm->toNumber(5), vm->toNumber(6), vm->toNumber(7) ).normalize();
	P(Node)		anchor	= getAnchorNode( vm, funcName, 7 );

	ParticleSystem* ps = m_particleMgr->play( name, anchor );
	
	Matrix3x3 rot;
	rot.generateOrthonormalBasis( dir );

	ps->setPosition( offset );
	ps->setRotation( rot );

	return 0;
}

int GameScriptable::script_loadSound( VM* vm, const char* funcName )
{
	if ( vm->top() != 1 || !vm->isString(1) )
		throw ScriptException( Format("Functions {0} expects sound description name (without .sf suffix)", funcName) );

	if ( !m_soundMgr )
		throw ScriptException( Format("Object {0} cannot play sounds", name()) );
	
	String	name	= vm->toString(1) + String(".sf");

	m_soundMgr->load( name );
	return 0;
}

int GameScriptable::script_playSound( VM* vm, const char* funcName )
{
	if ( vm->top() < 1 || !vm->isString(1) )
		throw ScriptException( Format("Functions {0} expects sound description name (without .sf suffix) and optional parent object name", funcName) );

	if ( !m_soundMgr )
		throw ScriptException( Format("Object {0} cannot play sounds", name()) );
	
	String		name	= vm->toString(1) + String(".sf");
	P(Node)		anchor	= getAnchorNode( vm, funcName, 1 );

	m_soundMgr->play( name, anchor );
	return 0;
}

int GameScriptable::script_playSoundAt( VM* vm, const char* funcName )
{
	if ( vm->top() < 4 || !vm->isString(1) || !vm->isNumber(2) || !vm->isNumber(3) || !vm->isNumber(4) )
		throw ScriptException( Format("Functions {0} expects sound description name (without .sf suffix), (x,y,z) offset and optional parent object name", funcName) );

	if ( !m_soundMgr )
		throw ScriptException( Format("Object {0} cannot play sounds", name()) );
	
	String		name	= vm->toString(1) + String(".sf");
	Vector3		offset	= Vector3( vm->toNumber(2), vm->toNumber(3), vm->toNumber(4) );
	P(Node)		anchor	= getAnchorNode( vm, funcName, 4 );

	//if ( m_soundMgr->getActiveCount(name,anchor) == 0 )
	Sound* sound = m_soundMgr->play( name, anchor );
	sound->setPosition( offset );
	return 0;
}

int GameScriptable::script_playSoundOffset( VM* vm, const char* funcName )
{
	if ( vm->top() < 2 || !vm->isString(1) || !vm->isNumber(2) )
		throw ScriptException( Format("Functions {0} expects sound description name (without .sf suffix), start time and optional parent object name", funcName) );

	if ( !m_soundMgr )
		throw ScriptException( Format("Object {0} cannot play sounds", name()) );
	
	String		name		= vm->toString(1) + String(".sf");
	float		startTime	= vm->toNumber(2);
	P(Node)		anchor		= getAnchorNode( vm, funcName, 2 );

	Sound* sound = m_soundMgr->play( name, anchor );
	sound->setCurrentPosition( startTime );
	return 0;
}

int GameScriptable::script_stopSound( VM* vm, const char* funcName )
{
	if ( vm->top() < 1 || !vm->isString(1) )
		throw ScriptException( Format("Functions {0} expects sound description name (without .sf suffix) and optional parent object name", funcName) );

	if ( !m_soundMgr )
		throw ScriptException( Format("Object {0} cannot play sounds", name()) );
	
	String		name	= vm->toString(1) + String(".sf");
	P(Node)		anchor	= getAnchorNode( vm, funcName, 1 );

//	m_soundMgr->stop( name, anchor );
	return 0;
}

int GameScriptable::script_fadeInSoundAt( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),1) )
		throw ScriptException( Format("Functions {0} expects sound description name (without .sf suffix), fade time, (x,y,z) offset and optional parent object name", funcName) );

	if ( !m_soundMgr )
		throw ScriptException( Format("Object {0} cannot play sounds", name()) );
	
	String		name		= vm->toString(1) + String(".sf");
	float		fadeTime	= vm->toNumber(2);
	Vector3		offset		= Vector3( vm->toNumber(3), vm->toNumber(4), vm->toNumber(5) );
	P(Node)		anchor		= getAnchorNode( vm, funcName, 5 );

	Sound* sound = m_soundMgr->fadeIn( name, anchor, fadeTime );
	sound->setPosition( offset );
	return 0;
}

int GameScriptable::script_fadeOutSound( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_NUMBER, VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0]),1) )
		throw ScriptException( Format("Functions {0} expects sound description name (without .sf suffix), fade time, and optional parent object name", funcName) );

	if ( !m_soundMgr )
		throw ScriptException( Format("Object {0} cannot play sounds", name()) );
	
	String		name		= vm->toString(1) + String(".sf");
	float		fadeTime	= vm->toNumber(2);
	P(Node)		anchor		= getAnchorNode( vm, funcName, 2 );

	m_soundMgr->fadeOut( name, anchor, fadeTime );
	return 0;
}

io::InputStreamArchive* GameScriptable::archive() const	
{
	assert( m_arch );
	return m_arch;
}

snd::SoundManager* GameScriptable::soundManager() const
{
	assert( m_soundMgr );
	return m_soundMgr;
}

ps::ParticleSystemManager* GameScriptable::particleSystemManager() const
{
	assert( m_particleMgr );
	return m_particleMgr;
}

int GameScriptable::script_getTimerEventCount( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 1 || !vm->isString(1) )
		throw ScriptException( Format("Functions {0} returns number of timer event that start with specified string", funcName) );

	String str = vm->toString(1);
	vm->pushNumber( (float)getTimerEventCount(str) );
	return 1;
}

int GameScriptable::script_getRandomInteger( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 2 || !vm->isNumber(1) || !vm->isNumber(2) )
		throw ScriptException( Format("Functions {0} returns integer in range [first,second]", funcName) );
	
	float	begin	= vm->toNumber(1);
	float	end		= vm->toNumber(2);
	float	n		= end - begin;
	
	float r = begin + random() * (n+1.f);
	r = Math::floor( r );

	//Debug::println( Format( "{0} random integer {1}", name(), r ).format() );

	vm->pushNumber( r );
	return 1;
}

