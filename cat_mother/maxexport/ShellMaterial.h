#ifndef _SHELLMATERIAL_H
#define _SHELLMATERIAL_H


/**
 * @author Toni Aittoniemi
 */
class ShellMaterial 
{
public:
	ShellMaterial() : original(0), baked(0) {} 
	ShellMaterial( Mtl* base, Mtl* bake ) : original(base), baked(bake) {}

	Mtl*	original;
	Mtl*	baked;

	operator < ( ShellMaterial& other ) const
	{
		return original < other.original;
	}
	operator == ( ShellMaterial& other ) const
	{
		return (original == other.original) && (baked == other.baked);
	}
};


#endif
