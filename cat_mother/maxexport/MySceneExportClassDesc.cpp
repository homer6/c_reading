#include "StdAfx.h"
#include "MySceneExportClassDesc.h"
#include "MySceneExport.h"
#include "GmMaterial.h"

//-----------------------------------------------------------------------------

MySceneExportClassDesc::MySceneExportClassDesc()
{
	instance = 0;
}

int MySceneExportClassDesc::IsPublic()
{
	return TRUE;
}

void* MySceneExportClassDesc::Create( BOOL loading )
{
	return new MySceneExport( instance );
}

const TCHAR* MySceneExportClassDesc::ClassName()
{
	return _T("SgExport");
}

SClass_ID MySceneExportClassDesc::SuperClassID()
{
	return SCENE_EXPORT_CLASS_ID;
}

Class_ID MySceneExportClassDesc::ClassID()
{
	return Class_ID(0x5476072, 0x325a669b);
}

const TCHAR* MySceneExportClassDesc::Category()
{
	return "SgExportCategory";
}

