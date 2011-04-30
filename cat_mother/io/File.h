#ifndef _IO_FILE_H
#define _IO_FILE_H


#include <lang/String.h>


namespace io 
{


/** 
 * Abstract representation of file and directory pathnames.
 * Abstract path name consists of following parts: 
 * <ul>
 * <li>Optional system-dependent root name. Root name makes the path absolute.</li>
 * <li>Optional directory name with child directories separated by '/'</li>
 * <li>Optional file name, separated from the directory name by '/'</li>
 * </ul>
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class File
{
public:
	/** 
	 * System-dependent name separator character. 
	 * Used when constructing platform-dependent path names.
	 */
	static const lang::Char		separatorChar;

	/** 
	 * System-dependent name separator character represented as a string. 
	 * Used when constructing platform-dependent path names.
	 */
	static const lang::String	separator;

	/** 
	 * System-dependent path separator character. 
	 * Path separator is used to separate filenames in a sequence of 
	 * files given as a path list.
	 */
	static const lang::Char		pathSeparatorChar;

	/** 
	 * System-dependent path separator character represented as a string. 
	 * Path separator is used to separate filenames in a sequence of 
	 * files given as a path list.
	 */
	static const lang::String	pathSeparator;

	/** Null-file. */
	File();

	/** Converts the given pathname string to abstract pathname. */
	explicit File( const lang::String& path );

	/** Converts the given parent and file name to abstract pathname. */
	File( const lang::String& parent, const lang::String& name );

	/** Returns true if the file exists. */
	bool			exists() const;

	/** Returns true if this abstract path name is absolute path. */
	bool			isAbsolute() const;

	/** Returns true if the file denoted by this abstract pathname is a directory. */
	bool			isDirectory() const;

	/** Returns true if the file denoted by this abstract pathname is a normal file. */
	bool			isFile() const;

	/** Returns the length of the file denoted by this abstract pathname. */
	long			length() const;

	/** Returns abstract path name string. */
	lang::String	getPath() const;

	/** Returns the name of the file or directory. */
	lang::String	getName() const;

	/** 
	 * Returns abstract parent path. Parent path consists of
	 * all path names except the last one.
	 * Returned string does not include path separator at the end.
	 * @exception IOException
	 */
	lang::String	getParent() const;

	/** 
	 * Returns abstract absolute path. Absolute path consists of
	 * all parent paths and the filename.
	 * @exception IOException
	 */
	lang::String	getAbsolutePath() const;

	/** 
	 * Lists the files in the directory. 
	 * @param buffer [out] Buffer for the file names.
	 * @param bufferSize Maximum number of entries that can fit to the buffer.
	 * @return Needed buffer size to store all file names in the directory.
	 */
	int				list( lang::String* buffer, int bufferSize ) const;
	
private:
	lang::String	m_path;
};


} // io


#endif // _IO_FILE_H
