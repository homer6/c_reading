#ifndef _MESHBUILDER_MESHBUILDERITEM_H
#define _MESHBUILDER_MESHBUILDERITEM_H


#include <lang/Object.h>


namespace mb
{


/** 
 * Base class for objects contained in MeshBuilder.
 * MeshBuilderItems are always owned by some MeshBuilder.
 * Item's lifetime is defined by the lifetime of the builder.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class MeshBuilderItem
{
public:
	///
	MeshBuilderItem();

	///
	virtual ~MeshBuilderItem();

private:
	MeshBuilderItem( const MeshBuilderItem& );
	MeshBuilderItem& operator=( const MeshBuilderItem& );
};


} // mb


#endif // _MESHBUILDER_MESHBUILDERITEM_H
