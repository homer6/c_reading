inline int FloatUtil::floatToInt( float x )
{
#if defined(_MSC_VER) && defined(_M_IX86)

	int	xi;
	__asm 
	{
		fld	x
		fistp xi
	}
	return xi;

#else

	return (int)x;

#endif
}
