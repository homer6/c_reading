#include "KeyFrame.h"


/** 
 * Return the incoming tangent to the curve at key1. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
void incoming( const KeyFrame* key0prev, const KeyFrame* key0, const KeyFrame* key1, const KeyFrame* key1next, float* in, int channels );
