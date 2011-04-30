/**
 * Main export class description. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class MySceneExportClassDesc : 
	public ClassDesc
{
public:
	HINSTANCE		instance;

	MySceneExportClassDesc();

	int				IsPublic();
	void*			Create( BOOL loading=FALSE );
	const TCHAR*	ClassName();
	SClass_ID		SuperClassID();
	Class_ID		ClassID();
	const TCHAR*	Category();
};
