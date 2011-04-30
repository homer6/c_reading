#ifndef _MESHBUILDER_LINKEDITEM_H
#define _MESHBUILDER_LINKEDITEM_H


#include <mb/MeshBuilderItem.h>


namespace mb
{


/** 
 * Base class for linked list of objects.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class LinkedItem :
	public MeshBuilderItem
{
public:
	///
	LinkedItem();

	/** Inserts an item after this one. */
	void		append( LinkedItem* item );

	/** Unlinks this item from the list. */
	void		unlink();

	/** Returns next item in the list or 0 if this is the last one. */
	LinkedItem* next() const;

	/** Returns previous item in the list or 0 if this is the first one. */
	LinkedItem* previous() const;

private:
	LinkedItem*	m_next;
	LinkedItem*	m_previous;

	LinkedItem( const LinkedItem& );
	LinkedItem& operator=( const LinkedItem& );
};


} // mb


#endif // _MESHBUILDER_LINKEDITEM_H
