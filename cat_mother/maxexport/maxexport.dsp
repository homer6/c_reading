# Microsoft Developer Studio Project File - Name="maxexport" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=maxexport - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "maxexport.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "maxexport.mak" CFG="maxexport - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "maxexport - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "maxexport"
# PROP Scc_LocalPath ".."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "build/Release"
# PROP Intermediate_Dir "build/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MAXEXPORT_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /w /W0 /GR /GX /Zi /Od /I "\3dsmax42\maxsdk\samples\controllers" /I "\3dsmax42\maxsdk\samples\modifiers\morpher" /I "\3dsmax5\maxsdk\samples\controllers" /I "\3dsmax5\maxsdk\samples\modifiers\morpher" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MAXEXPORT_EXPORTS" /Yu"StdAfx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40b /d "NDEBUG"
# ADD RSC /l 0x40b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib maxutil.lib geom.lib core.lib comctl32.lib mesh.lib /nologo /dll /incremental:yes /map /debug /machine:I386 /out:"lib/sgexport.dle"
# Begin Special Build Tool
TargetPath=.\lib\sgexport.dle
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(TargetPath) "\3DSMax5\plugins"
# End Special Build Tool
# Begin Target

# Name "maxexport - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AnimExportUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\BipedUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\BSPUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\ChunkUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\DebugUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\GmLineList.cpp
# End Source File
# Begin Source File

SOURCE=.\GmMaterial.cpp
# End Source File
# Begin Source File

SOURCE=.\GmModel.cpp
# End Source File
# Begin Source File

SOURCE=.\GmModelPrimitive.cpp
# End Source File
# Begin Source File

SOURCE=.\GmMorphChannel.cpp
# End Source File
# Begin Source File

SOURCE=.\GmPatch.cpp
# End Source File
# Begin Source File

SOURCE=.\GmUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\hermite.cpp
# End Source File
# Begin Source File

SOURCE=.\incoming.cpp
# End Source File
# Begin Source File

SOURCE=.\internalError.cpp
# End Source File
# Begin Source File

SOURCE=.\InterpolationCR.cpp
# End Source File
# Begin Source File

SOURCE=.\InterpolationLinear.cpp
# End Source File
# Begin Source File

SOURCE=.\InterpolationQuaternionSlerp.cpp
# End Source File
# Begin Source File

SOURCE=.\InterpolationStepped.cpp
# End Source File
# Begin Source File

SOURCE=.\KeyFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\KeyFrameContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\LODUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\maxexport.cpp
# End Source File
# Begin Source File

SOURCE=.\maxexport.rc
# End Source File
# Begin Source File

SOURCE=.\MorphUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\MySceneExport.cpp
# End Source File
# Begin Source File

SOURCE=.\MySceneExportClassDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\outgoing.cpp
# End Source File
# Begin Source File

SOURCE=.\PhyExportUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgressDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\QuaternionUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\SceneEnvironment.cpp
# End Source File
# Begin Source File

SOURCE=.\SceneExportUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\SgCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\SgDummy.cpp
# End Source File
# Begin Source File

SOURCE=.\SgLight.cpp
# End Source File
# Begin Source File

SOURCE=.\SgLOD.cpp
# End Source File
# Begin Source File

SOURCE=.\SgMesh.cpp
# End Source File
# Begin Source File

SOURCE=.\SgNode.cpp
# End Source File
# Begin Source File

SOURCE=.\SgUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\ShadowVolumeBuilder.cpp
# End Source File
# Begin Source File

SOURCE=.\SkinExportUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"StdAfx.h"
# End Source File
# Begin Source File

SOURCE=.\TmUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\WinUtil.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AbortExport.h
# End Source File
# Begin Source File

SOURCE=.\AnimExportUtil.h
# End Source File
# Begin Source File

SOURCE=.\BipedUtil.h
# End Source File
# Begin Source File

SOURCE=.\BSPTreeBuilderThread.h
# End Source File
# Begin Source File

SOURCE=.\BSPUtil.h
# End Source File
# Begin Source File

SOURCE=.\build.h
# End Source File
# Begin Source File

SOURCE=.\ChunkUtil.h
# End Source File
# Begin Source File

SOURCE=.\DebugUtil.h
# End Source File
# Begin Source File

SOURCE=.\GmLineList.h
# End Source File
# Begin Source File

SOURCE=.\GmMaterial.h
# End Source File
# Begin Source File

SOURCE=.\GmModel.h
# End Source File
# Begin Source File

SOURCE=.\GmModelPrimitive.h
# End Source File
# Begin Source File

SOURCE=.\GmMorphChannel.h
# End Source File
# Begin Source File

SOURCE=.\GmPatch.h
# End Source File
# Begin Source File

SOURCE=.\GmUtil.h
# End Source File
# Begin Source File

SOURCE=.\hermite.h
# End Source File
# Begin Source File

SOURCE=.\incoming.h
# End Source File
# Begin Source File

SOURCE=.\Interpolation.h
# End Source File
# Begin Source File

SOURCE=.\InterpolationCR.h
# End Source File
# Begin Source File

SOURCE=.\InterpolationLinear.h
# End Source File
# Begin Source File

SOURCE=.\InterpolationQuaternionSlerp.h
# End Source File
# Begin Source File

SOURCE=.\InterpolationStepped.h
# End Source File
# Begin Source File

SOURCE=.\KeyFrame.h
# End Source File
# Begin Source File

SOURCE=.\KeyFrameContainer.h
# End Source File
# Begin Source File

SOURCE=.\LODUtil.h
# End Source File
# Begin Source File

SOURCE=.\maxexport.h
# End Source File
# Begin Source File

SOURCE=.\MorphUtil.h
# End Source File
# Begin Source File

SOURCE=.\MySceneExport.h
# End Source File
# Begin Source File

SOURCE=.\MySceneExportClassDesc.h
# End Source File
# Begin Source File

SOURCE=.\NodeNameLess.h
# End Source File
# Begin Source File

SOURCE=.\outgoing.h
# End Source File
# Begin Source File

SOURCE=.\PhyExportUtil.h
# End Source File
# Begin Source File

SOURCE=.\ProgressDialog.h
# End Source File
# Begin Source File

SOURCE=.\ProgressInterface.h
# End Source File
# Begin Source File

SOURCE=.\QuaternionUtil.h
# End Source File
# Begin Source File

SOURCE=.\require.h
# End Source File
# Begin Source File

SOURCE=.\resampling.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\SceneEnvironment.h
# End Source File
# Begin Source File

SOURCE=.\SceneExportUtil.h
# End Source File
# Begin Source File

SOURCE=.\SgCamera.h
# End Source File
# Begin Source File

SOURCE=.\SgDummy.h
# End Source File
# Begin Source File

SOURCE=.\SgLight.h
# End Source File
# Begin Source File

SOURCE=.\SgLightDistanceLess.h
# End Source File
# Begin Source File

SOURCE=.\SgLOD.h
# End Source File
# Begin Source File

SOURCE=.\SgMesh.h
# End Source File
# Begin Source File

SOURCE=.\SgNode.h
# End Source File
# Begin Source File

SOURCE=.\SgUtil.h
# End Source File
# Begin Source File

SOURCE=.\ShadowVolumeBuilder.h
# End Source File
# Begin Source File

SOURCE=.\SkinExportUtil.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TmUtil.h
# End Source File
# Begin Source File

SOURCE=.\Version.h
# End Source File
# Begin Source File

SOURCE=.\WinUtil.h
# End Source File
# Begin Source File

SOURCE=.\wm3.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "docs"

# PROP Default_Filter "txt;doc"
# Begin Source File

SOURCE=..\docs\fileformat_gm.txt
# End Source File
# Begin Source File

SOURCE=..\docs\fileformat_sg.txt
# End Source File
# Begin Source File

SOURCE=.\docs\sgexport.txt
# End Source File
# End Group
# End Target
# End Project
