#ifndef _DRVOBJECT_H
#define _DRVOBJECT_H


class Dx8ObjectList;


/** 
 * Base class for graphics library DX8 driver objects. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class DrvObject
{
public:
	DrvObject();
	virtual ~DrvObject();

	/** Called after device has been reset. */
	virtual void	resetDeviceObject() {}

	/** Called after device memory has been lost. */
	virtual void	destroyDeviceObject() = 0;

	virtual long	textureMemoryUsed() const;

	static void		resetAllDeviceObjects();
	static void		destroyAllDeviceObjects();
	static long		totalTextureMemoryUsed();

	static int		objects();
	static void		deleteAll();

private:
	friend class DrvObjectList;

	DrvObject*		m_next;

	DrvObject( const DrvObject& );
	DrvObject& operator=( const DrvObject& );
};



#endif // _DRVOBJECT_H
