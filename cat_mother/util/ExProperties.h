#ifndef _UTIL_EXPROPERTIES_H
#define _UTIL_EXPROPERTIES_H


#include <util/Properties.h>


namespace util
{


/** 
 * Extended properties class.
 * Contains primitive parsing operations.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ExProperties :
	public Properties
{
public:
	/** 
	 * Sets integer value property. 
	 * @param name Name of the property.
	 * @param x New value of the property.
	 */
	void			setInteger( const lang::String& name, int x );

	/** 
	 * Sets floating point value property. 
	 * @param name Name of the property.
	 * @param x New value of the property.
	 */
	void			setFloat( const lang::String& name, float x );

	/** 
	 * Sets boolean value property. 
	 * @param name Name of the property.
	 * @param x New value of the property.
	 */
	void			setBoolean( const lang::String& name, bool x );

	/** 
	 * Reads integer array value property. 
	 * @param name Name of the property.
	 * @param buffer Array of values to set.
	 * @param count Number of values in the array.
	 */
	void			setIntegers( const lang::String& name, 
						const int* buffer, int count );

	/** 
	 * Sets floating point array value property. 
	 * @param name Name of the property.
	 * @param buffer Array of values to set.
	 * @param count Number of values in the array.
	 */
	void			setFloats( const lang::String& name, 
						const float* buffer, int count );

	/** 
	 * Sets boolean array value property.
	 * @param name Name of the property.
	 * @param buffer Array of values to set.
	 * @param count Number of values in the array.
	 */
	void			setBooleans( const lang::String& name, 
						const bool* buffer, int count );

	/** 
	 * Sets string array value property.
	 * @param name Name of the property.
	 * @param buffer Array of values to set.
	 * @param count Number of values in the array.
	 */
	void			setStrings( const lang::String& name, 
						const lang::String* buffer, int count );

	/** 
	 * Returns integer value property. 
	 * @param name Name of the property.
	 * @exception NumberFormatException
	 */
	int				getInteger( const lang::String& name ) const;

	/** 
	 * Returns floating point value property. 
	 * @param name Name of the property.
	 * @exception NumberFormatException
	 */
	float			getFloat( const lang::String& name ) const;

	/** 
	 * Returns boolean value property. 
	 * @param name Name of the property.
	 * @exception NumberFormatException
	 */
	bool			getBoolean( const lang::String& name ) const;

	/** 
	 * Reads integer array value property. 
	 * @param name Name of the property.
	 * @param buffer Array of values to get.
	 * @param count Number of values in the array.
	 * @exception NumberFormatException
	 */
	void			getIntegers( const lang::String& name, 
						int* buffer, int count ) const;

	/** 
	 * Returns floating point array value property. 
	 * @param name Name of the property.
	 * @param buffer Array of values to get.
	 * @param count Number of values in the array.
	 * @exception NumberFormatException
	 */
	void			getFloats( const lang::String& name, 
						float* buffer, int count ) const;

	/** 
	 * Returns boolean array value property.
	 * @param name Name of the property.
	 * @param buffer Array of values to get.
	 * @param count Number of values in the array.
	 * @exception NumberFormatException
	 */
	void			getBooleans( const lang::String& name, 
						bool* buffer, int count ) const;

	/** 
	 * Returns string array value property.
	 * @param name Name of the property.
	 * @param buffer Array of values to get.
	 * @param count Number of values in the array.
	 * @exception Exception
	 */
	void			getStrings( const lang::String& name, 
						lang::String* buffer, int count ) const;
};


} // util


#endif // _UTIL_EXPROPERTIES_H
