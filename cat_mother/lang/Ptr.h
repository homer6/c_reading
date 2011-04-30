#ifndef _LANG_PTR_H
#define _LANG_PTR_H


#ifndef _LANG_CONFIG_INL_H
#include <lang/internal/config_inl.h>
#endif


namespace lang
{


/**
 * Thread-safe smart pointer to an object of class T.
 * T must implement at least addReference() and release() methods.
 * release() must destroy the object if no more references are left.
 * Initial reference count of an object must be 0.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class T> class Ptr
{
public:
	/** Null pointer. */
	Ptr()																			{m_object = 0;}

	/** Releases reference to the object. */
	~Ptr()																			{if ( m_object ) m_object->release();}

	/** Increments object reference count and stores the reference. */
	Ptr( const Ptr<T>& other )														{T* obj = other.ptr(); if ( obj ) obj->addReference(); m_object = obj;}

	/** Increments object reference count and stores the reference. */
	Ptr( T* other )																	{if ( other ) other->addReference(); m_object = other;}

	/** 
	 * Releases old reference if any, increments other object reference 
	 * count and stores the new reference. 
	 */
	Ptr<T>& operator=( const Ptr<T>& other )										{T* obj = other.ptr(); if ( obj ) obj->addReference(); if ( m_object ) m_object->release(); m_object = obj; return *this;}

	/** Returns true if the references point to the same unique object. */
	bool	operator==( const T* other ) const										{return m_object == other;}

	/** Returns true if the references point to the same unique object. */
	bool	operator==( const Ptr<T>& other ) const									{return m_object == other.m_object;}
	
	/** Returns true if the references point to different unique objects. */
	bool	operator!=( const T* other ) const										{return m_object != other;}

	/** Returns true if the references point to different unique objects. */
	bool	operator!=( const Ptr<T>& other ) const									{return m_object != other.m_object;}

	/** Access to the object. */
	T&		operator*() const														{return *m_object;}

	/** Access to the object. */
	T*		operator->() const														{return m_object;}

	/** Access to the object. */

	operator T*() const																{return m_object;}

	/** Access to the object. */
	T*		ptr() const																{return m_object;}

private:
	T* m_object;
};


} // lang


#endif // _LANG_PTR_H
