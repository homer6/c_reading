#ifndef _COMPTR_H
#define _COMPTR_H


#ifndef WIN32
#error Cannot use COM on non-Win32 platform
#endif


namespace music
{


/** 
 * Smart pointer to COM object. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class T> class COMPtr
{
public:
	/** Null pointer. */
	COMPtr()																		{m_object = 0;}

	/** Releases reference to the object. */
	~COMPtr()																		{if ( m_object ) m_object->Release();}

	/** Increments object reference count and stores the reference. */
	COMPtr( const COMPtr<T>& other )												{T* obj = other.ptr(); if ( obj ) obj->AddRef(); m_object = obj;}

	/** Increments object reference count and stores the reference. */
	COMPtr( T* other )																{if ( other ) other->AddRef(); m_object = other;}

	/** 
	 * Releases old reference if any, increments other object reference 
	 * count and stores the new reference. 
	 */
	COMPtr<T>& operator=( const COMPtr<T>& other )									{T* obj = other.ptr(); if ( obj ) obj->AddRef(); if ( m_object ) m_object->Release(); m_object = obj; return *this;}

	/** Returns true if the references point to the same unique object. */
	bool	operator==( const T* other ) const										{return m_object == other;}

	/** Returns true if the references point to the same unique object. */
	bool	operator==( const COMPtr<T>& other ) const								{return m_object == other.m_object;}
	
	/** Returns true if the references point to different unique objects. */
	bool	operator!=( const T* other ) const										{return m_object != other;}

	/** Returns true if the references point to different unique objects. */
	bool	operator!=( const COMPtr<T>& other ) const								{return m_object != other.m_object;}

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


} // music


#endif // _COMPTR_H
