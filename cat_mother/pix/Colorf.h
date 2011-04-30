#ifndef _PIX_COLORF_H
#define _PIX_COLORF_H


namespace pix 
{


class Color;


/**
 * 4-float RGBA color.
 *
 * The order of the stored color channel values from the least 
 * signifigant to the most signifigant is red, green, blue and alpha.
 *
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Colorf
{
public:
	/**
	 * Describes order of stored color channels.
	 */
	enum Channels
	{ 
		/// Index for red channel data
		RED_INDEX				= 0,
		/// Index for green channel data
		GREEN_INDEX				= 1,
		/// Index for blue channel data
		BLUE_INDEX				= 2,
		/// Index for alpha channel data
		ALPHA_INDEX				= 3,
		/// Number of color channels.
		SIZE					= 4,
	};

	/** Creates white opaque color. */
	Colorf()																		{m_c[RED_INDEX]=m_c[GREEN_INDEX]=m_c[BLUE_INDEX]=m_c[ALPHA_INDEX]=1.f;}

	/** Constructs a color from color type. */
	explicit Colorf( const Color& source );

	/** Constructs a color from channel values. */
	Colorf( float red, float green, float blue, 
		float alpha = maxIntensity() )												{m_c[RED_INDEX]=red; m_c[GREEN_INDEX]=green; m_c[BLUE_INDEX]=blue; m_c[ALPHA_INDEX]=alpha;}

	/** Adds other color to this color component-wise. */
	Colorf&			operator+=( const Colorf& other )								{for ( int i = 0 ; i < SIZE ; ++i ) m_c[i] += other.m_c[i]; return *this;}
	
	/** Subtracts other color from this color component-wise. */
	Colorf&			operator-=( const Colorf& other )								{for ( int i = 0 ; i < SIZE ; ++i ) m_c[i] -= other.m_c[i]; return *this;}
	
	/** Scales the color component-wise. */
	Colorf&			operator*=( float s )											{for ( int i = 0 ; i < SIZE ; ++i ) m_c[i] *= s; return *this;}

	/** 
	 * Returns ith component of the color. 
	 * Note that the color is dependent on index order specified by enum channels. 
	 * @see Channels
	 */
	float&			operator[]( int index )											{return m_c[index];}

	/** Sets the red component of the color */
	void			setRed( float newValue )										{m_c[RED_INDEX]=newValue;}
	
	/** Sets the green component of the color */
	void			setGreen( float newValue )										{m_c[GREEN_INDEX]=newValue;}
	
	/** Sets the blue component of the color */
	void			setBlue( float newValue )										{m_c[BLUE_INDEX]=newValue;}
	
	/** Sets the alpha component of the color */
	void			setAlpha( float newValue )										{m_c[ALPHA_INDEX]=newValue;}

	/** Returns a random-access iterator to the first element. */
	float*			begin()															{return m_c;}
	
	/** Returns a random-access iterator that points just beyond the last element. */
	float*			end()															{return m_c + SIZE;}

	/** Returns the red component of the color */
	float			red() const														{return m_c[RED_INDEX];}
	
	/** Returns the green component of the color */
	float			green() const													{return m_c[GREEN_INDEX];}
	
	/** Returns the blue component of the color */
	float			blue() const													{return m_c[BLUE_INDEX];}
	
	/** Returns the alpha component of the color */
	float			alpha() const													{return m_c[ALPHA_INDEX];}

	/** Adds this and other color component-wise. */
	Colorf			operator+( const Colorf& other ) const							{Colorf c; for ( int i = 0 ; i < SIZE ; ++i ) c.m_c[i] = m_c[i] + other.m_c[i]; return c;}

	/** Subtracts this and other color component-wise. */
	Colorf			operator-( const Colorf& other ) const							{Colorf c; for ( int i = 0 ; i < SIZE ; ++i ) c.m_c[i] = m_c[i] - other.m_c[i]; return c;}

	/** Multiplies this and other color component-wise. */
	Colorf			operator*( const Colorf& other ) const							{Colorf c; for ( int i = 0 ; i < SIZE ; ++i ) c.m_c[i] = m_c[i] * other.m_c[i]; return c;}

	/** Returns the color scaled component-wise. */
	Colorf			operator*( float s ) const										{Colorf c; for ( int i = 0 ; i < SIZE ; ++i ) c.m_c[i] = m_c[i] * s; return c;}

	/** 
	 * Returns ith component of the color. 
	 * Note that the color is dependent on index order specified by enum channels. 
	 * @see Channels
	 */
	const float&	operator[]( int index ) const									{return m_c[index];}

	/** Component-wise equality. */
	bool			operator==( const Colorf& other ) const							{return equal(begin(),end(),other.begin());}

	/** Component-wise inequality. */
	bool			operator!=( const Colorf& other ) const							{return !this->operator==(other);}

	/** Returns true if every element is in finite range */
	bool			finite() const;

	/** Returns a random-access iterator to the first element. */
	const float*	begin() const													{return m_c;}
	
	/** Returns a random-access iterator that points just beyond the last element. */
	const float*	end() const														{return m_c + SIZE;}

	/** Returns clamped color to [min,max] range. */
	Colorf			saturate() const;

	/** Returns average of R,G,B channels. */
	float			brightness() const;

	/** Returns zero intensity value (0) of color channel. */
	static float	zeroIntensity()													{return 0.f;}

	/** Returns normalized maximum intensity (1) value of color channel. */
	static float	maxIntensity()													{return 1.f;}

private:
	float			m_c[SIZE];

	static bool		equal( const float* begin, const float* end, const float* begin2 )			{for ( ; begin != end ; ++begin ) if ( *begin != *begin2++ ) return false; return true;}
	static bool		validIndex( int index )														{return index >= 0 && index < SIZE;}
};


} // pix


#endif // _PIX_COLORF_H
