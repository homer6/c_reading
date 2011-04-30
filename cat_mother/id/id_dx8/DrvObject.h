#ifndef _DRVOBJECT_H
#define _DRVOBJECT_H


class Dx8ObjectList;


/** 
 * Base class for DX8 driver objects. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class DrvObject
{
public:
	DrvObject();
	virtual ~DrvObject();

	static int		objects();
	static void		deleteAll();

private:
	friend class DrvObjectList;

	DrvObject*		m_next;

	DrvObject( const DrvObject& );
	DrvObject& operator=( const DrvObject& );
};



#endif // _DRVOBJECT_H
