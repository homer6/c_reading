/** 
 * Window handling utilities. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class WinUtil
{
public:
	/** 
	 * Flushes message queue. 
	 * @return false if quit is requested.
	 */
	static bool flushWindowMessages();
};
