#ifndef _BELLCURVE_H
#define _BELLCURVE_H


/** 
 * Evaluates a bell curve on range (minlimit..maxlimit) between (0..1) 
 * @author Toni Aittoniemi
 */
class BellCurve
{
public:
	/** Value of a bell curve from minlimit to maxlimit @ pos. */
	static float evaluateFull( float minlimit, float maxlimit, float pos );

	/** Value of the positive half of a bell curve from minlimit to maxlimit @ pos. */
	static float evaluatePosHalf( float minlimit, float maxlimit, float pos );

	/** Value of the negative half of a bell curve from minlimit to maxlimit @ pos. */
	static float evaluateNegHalf( float minlimit, float maxlimit, float pos );

	/** Value of normal distribution bell curve ( mean = 0, standard deviation = 1 ) @ pos. */
	static float bell( float pos ); 
};


#endif // _BELLCURVE_H
