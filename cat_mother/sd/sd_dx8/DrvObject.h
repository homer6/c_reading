#ifndef _DRVOBJECT_H
#define _DRVOBJECT_H


class DrvObjectList;


/** 
 * Base class for driver objects. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class DrvObject
{
public:
	DrvObject();
	virtual ~DrvObject();

	virtual void	destroyDeviceObject() = 0;

	static void		destroyAllDeviceObjects();

	static int		objects();
	static void		deleteAll();

private:
	friend class DrvObjectList;

	DrvObject*		m_next;

	DrvObject( const DrvObject& );
	DrvObject& operator=( const DrvObject& );
};



#endif // _DRVOBJECT_H
