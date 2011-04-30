#ifndef _PIX_COLOR_H
#define _PIX_COLOR_H


#include <stdint.h>


namespace pix 
{


class Colorf;


/**
 * 4-byte RGBA color.
 *
 * The order of the stored color channel values from the least 
 * signifigant to the most signifigant is blue, green, red and alpha.
 *
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Color
{
public:
	/**
	 * Describes order of stored color channels.
	 */
	enum Channels
	{ 
		/// Index for red channel data
		RED_INDEX				= 2,
		/// Index for green channel data
		GREEN_INDEX				= 1,
		/// Index for blue channel data
		BLUE_INDEX				= 0,
		/// Index for alpha channel data
		ALPHA_INDEX				= 3,
		/// Number of color channels.
		SIZE					= 4,
	};

	/** Creates white opaque color. */
	Color()																			{*reinterpret_cast<uint32_t*>(m_c) = 0xFFFFFFFF;}

	/** Constructs a color from colorf type. Clamps channel values to valid range. */
	explicit Color( const Colorf& source );

	/** Constructs a color from channel values. */
	Color( uint8_t red, uint8_t green, uint8_t blue, 
		uint8_t alpha = maxIntensity() )											{m_c[RED_INDEX]=red; m_c[GREEN_INDEX]=green; m_c[BLUE_INDEX]=blue; m_c[ALPHA_INDEX]=alpha;}

	/** 
	 * Constructs color from packed 32-bit unsigned integer. 
	 * Alpha channels is stored to bits 24-31,
	 * red channel to 16-23, green channel to 8-15 and blue to 0-7.
	 */
	explicit Color( uint32_t colorValue )											{*reinterpret_cast<uint32_t*>(m_c) = colorValue;}

	/** 
	 * Sets the red component of the color 
	 */
	void			setRed( uint8_t newValue )										{m_c[RED_INDEX]=newValue;}
	
	/** 
	 * Sets the green component of the color 
	 */
	void			setGreen( uint8_t newValue )									{m_c[GREEN_INDEX]=newValue;}
	
	/** 
	 * Sets the blue component of the color 
	 */
	void			setBlue( uint8_t newValue )										{m_c[BLUE_INDEX]=newValue;}
	
	/** 
	 * Sets the alpha component of the color 
	 */
	void			setAlpha( uint8_t newValue )									{m_c[ALPHA_INDEX]=newValue;}

	/** 
	 * Returns a random-access iterator to the first element. 
	 */
	uint8_t*		begin()															{return m_c;}
	
	/** 
	 * Returns a random-access iterator that points just beyond the last element. 
	 */
	uint8_t*		end()															{return m_c + SIZE;}

	/** 
	 * Adds Other color to this color component-wise. 
	 * Wraps at maximum and minimum intensities.
	 */
	Color&			operator+=( const Color& other )								{return *this = *this + other;}
	
	/** 
	 * Subtracts Other color from this color component-wise. 
	 * Wraps at maximum and minimum intensities.
	 */
	Color&			operator-=( const Color& other )								{return *this = *this - other;}
	
	/** 
	 * Returns ith component of the color. 
	 * Note that the color is dependent on index order specified by enum channels. 
	 * @see Channels
	 */
	uint8_t&		operator[]( int index )											{return m_c[index];}

	/** Returns the red component of the color */
	uint8_t			red() const														{return m_c[RED_INDEX];}
	
	/** Returns the green component of the color */
	uint8_t			green() const													{return m_c[GREEN_INDEX];}
	
	/** Returns the blue component of the color */
	uint8_t			blue() const													{return m_c[BLUE_INDEX];}
	
	/** Returns the alpha component of the color */
	uint8_t			alpha() const													{return m_c[ALPHA_INDEX];}

	/** 
	 * Adds Other color to this color component-wise. 
	 * Wraps at maximum and minimum intensities.
	 */
	Color			operator+( const Color& other ) const							{Color ret(*this); add(ret.begin(),ret.end(),other.begin()); return ret;}

	/** 
	 * Subtracts Other color from this color component-wise. 
	 * Wraps at maximum and minimum intensities.
	 */
	Color			operator-( const Color& other ) const							{Color ret(*this); subtract(ret.begin(),ret.end(),other.begin()); return ret;}

	/** Component-wise equality. */
	bool			operator==( const Color& other ) const							{return equal(begin(),end(),other.begin());}

	/** Component-wise inequality. */
	bool			operator!=( const Color& other ) const							{return !this->operator==(other);}

	/** Returns true if every element is in finite range. (always true) */
	bool			finite() const													{return true;}

	/** 
	 * Returns ith component of the color. 
	 * Note that the color is dependent on index order specified by enum channels. 
	 * @see Channels
	 */
	const uint8_t&	operator[]( int index ) const									{return m_c[index];}

	/** Returns a random-access iterator to the first element. */
	const uint8_t*	begin() const													{return m_c;}
	
	/** Returns a random-access iterator that points just beyond the last element. */
	const uint8_t*	end() const														{return m_c + SIZE;}

	/** 
	 * Returns color packed to 32-bit unsigned integer. 
	 * Alpha channels is stored to bits 24-31,
	 * red channel to 16-23, green channel to 8-15 and blue to 0-7.
	 */
	uint32_t		toInt32() const													{return *reinterpret_cast<const uint32_t*>(m_c);}

	/** Returns zero intensity value (0) of color channel. */
	static uint8_t	zeroIntensity()													{return uint8_t(0);}

	/** Returns maximum intensity value (255) of color channel. */
	static uint8_t	maxIntensity()													{return uint8_t(255);}

private:
	uint8_t			m_c[SIZE];

	static bool		equal( const uint8_t* begin, const uint8_t* end, const uint8_t* begin2 )	{for ( ; begin != end ; ++begin ) if ( *begin != *begin2++ ) return false; return true;}
	static void		set( uint8_t* begin, uint8_t* end, uint8_t value )							{for ( ; begin != end ; ++begin ) *begin = value;}
	static void		add( uint8_t* begin, uint8_t* end, const uint8_t* begin2 )					{for ( ; begin != end ; ++begin, ++begin2 ) *begin = (uint8_t)(*begin + *begin2);}
	static void		subtract( uint8_t* begin, uint8_t* end, const uint8_t* begin2 )				{for ( ; begin != end ; ++begin, ++begin2 ) *begin = (uint8_t)(*begin - *begin2);}
	static bool		validIndex( int index )														{return index >= 0 && index < SIZE;}
};


} // pix


#endif // _PIX_COLOR_H
