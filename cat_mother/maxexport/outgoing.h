#include "KeyFrame.h"


/** 
 * Return the outgoing tangent to the curve at key0. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
void outgoing( const KeyFrame* key0prev, const KeyFrame* key0, const KeyFrame* key1, const KeyFrame* key1next, float* out, int channels );
