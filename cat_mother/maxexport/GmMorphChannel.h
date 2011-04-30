#ifndef _GMMORPHCHANNEL_H
#define _GMMORPHCHANNEL_H


#include <lang/Object.h>
#include <lang/String.h>
#include "KeyFrameContainer.h"


class GmModel;


/**
 * Single weighted channel in Morpher.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GmMorphChannel :
	public lang::Object
{
public:
	lang::String		name;
	KeyFrameContainer	weightAnim;
	GmModel*			target;

	GmMorphChannel();
	GmMorphChannel( const lang::String& chnName, const KeyFrameContainer& chnWeightAnim, GmModel* chnTarget );
	~GmMorphChannel();
};


#endif // _GMMORPHCHANNEL_H
