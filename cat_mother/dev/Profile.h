#ifndef _DEV_PROFILE_H
#define _DEV_PROFILE_H


#include <dev/TimeStamp.h>


namespace dev
{


/** 
 * Class for profiling code. Thread-safe.
 *
 * High resolution timer is started in the constructor of the profile object 
 * and stopped automatically in the destructor when the code scope is exited.
 * The statistics of Profile objects with identical scope names are merged together.
 *
 * After all blocks of code have been profiled, the class functions can 
 * then be used to report statistics. Statistics include for example number of
 * times scope executed and time spend in the code.
 *
 * Usage example:
 * <pre>
    void func1() 
    { 
        Profile profileFunc1( "func1" ); 
        ...
    } 

    void func1() 
    { 
        Profile profileFunc2( "func2" ); 
        ...
    }

    // report execution times of the functions to console
    func1();
    func2();
    for ( int i = 0 ; i < Profile::count() ; ++i ) 
    { 
		Profile::BlockInfo* block = Profile::get(i);
        cout << block->name() << ": " << block->time() << "\n"; 
    }   
	</pre>
 *
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Profile
{
public:
	/** Interface to information about profiled scope. */
	class BlockInfo
	{
	public:
		/** Returns name of the scope. ASCII-7 characters only. */
		virtual const char*		name() const = 0;

		/** Returns time (in seconds) spend in the scope since last reset. */
		virtual double			time() const = 0;

		/** Returns number of times the scope has been profiled since last reset. */
		virtual int				count() const = 0;
	};

	/** 
	 * Starts profiling current scope. 
	 * All profile objects with the same scope name 
	 * will contribute to the same result.
	 *
	 * @param name The name of the profiled scope. ASCII-7 characters only.
	 */
	explicit Profile( const char* name );

	/** Ends scope profiling. */
	~Profile();

	/** 
	 * Clears all profiling information. 
	 * Profiling can't be in progress when this function is called.
	 */
	static void				reset();

	/** Returns number of profiled scopes. */
	static int				count();

	/** Returns ith profiled scope. */
	static BlockInfo*		get( int index );

	/** Sets profiling enabled/disabled. */
	static void				setEnabled( bool enabled );

private:
	class BlockInfoImpl;
	class ProfileStaticData;
	friend class ProfileStaticData;

	static bool		sm_enabled;
	BlockInfoImpl*	m_info;
	TimeStamp		m_ticks;
	long			m_allocBlocks;
	long			m_allocBytes;
	long			m_freedBlocks;
	long			m_freedBytes;

	Profile();
	Profile( const Profile& );
	Profile& operator=( const Profile& );
};


} // dev


#endif // _DEV_PROFILE_H
