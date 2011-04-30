# Microsoft Developer Studio Project File - Name="gd_dx9" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=gd_dx9 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "gd_dx9.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gd_dx9.mak" CFG="gd_dx9 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gd_dx9 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gd_dx9 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gd_dx9 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "lib"
# PROP Intermediate_Dir "build/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GD_DX9_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W4 /GR /GX /O2 /I "..\gd_dx_common" /I "." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GD_DX9_EXPORTS" /Yu"StdAfx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib d3dx9.lib d3d9.lib /nologo /dll /machine:I386
# Begin Special Build Tool
TargetPath=.\lib\gd_dx9.dll
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(TargetPath) ..\..\DLL
# End Special Build Tool

!ELSEIF  "$(CFG)" == "gd_dx9 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "lib"
# PROP Intermediate_Dir "build/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GD_DX9_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /ZI /Od /I "..\gd_dx_common" /I "." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GD_DX9_EXPORTS" /Yu"StdAfx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ..\..\mem\lib\memd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib d3dx9d.lib d3d9.lib /nologo /dll /debug /machine:I386 /out:"lib/gd_dx9d.dll" /pdbtype:sept
# Begin Special Build Tool
TargetPath=.\lib\gd_dx9d.dll
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(TargetPath) ..\..\DLL
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "gd_dx9 - Win32 Release"
# Name "gd_dx9 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Dx9BaseTexture.cpp
# End Source File
# Begin Source File

SOURCE=.\Dx9CubeTexture.cpp
# End Source File
# Begin Source File

SOURCE=.\Dx9Effect.cpp
# End Source File
# Begin Source File

SOURCE=.\Dx9GraphicsDevice.cpp
# End Source File
# Begin Source File

SOURCE=.\Dx9GraphicsDriver.cpp
# End Source File
# Begin Source File

SOURCE=.\Dx9Material.cpp
# End Source File
# Begin Source File

SOURCE=.\Dx9Primitive.cpp
# End Source File
# Begin Source File

SOURCE=.\Dx9RenderingState.cpp
# End Source File
# Begin Source File

SOURCE=.\Dx9RenderToTexture.cpp
# End Source File
# Begin Source File

SOURCE=.\Dx9Texture.cpp
# End Source File
# Begin Source File

SOURCE=.\gd_dx9.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc
# End Source File
# Begin Source File

SOURCE=.\toDx9.cpp
# End Source File
# Begin Source File

SOURCE=.\toString.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Dx9BaseTexture.h
# End Source File
# Begin Source File

SOURCE=.\Dx9BaseTextureImplInterface.h
# End Source File
# Begin Source File

SOURCE=.\Dx9CubeTexture.h
# End Source File
# Begin Source File

SOURCE=.\Dx9Effect.h
# End Source File
# Begin Source File

SOURCE=.\Dx9GraphicsDevice.h
# End Source File
# Begin Source File

SOURCE=.\Dx9GraphicsDriver.h
# End Source File
# Begin Source File

SOURCE=.\Dx9Material.h
# End Source File
# Begin Source File

SOURCE=.\Dx9Primitive.h
# End Source File
# Begin Source File

SOURCE=.\Dx9RenderingState.h
# End Source File
# Begin Source File

SOURCE=.\Dx9RenderToTexture.h
# End Source File
# Begin Source File

SOURCE=.\Dx9SurfaceLock.h
# End Source File
# Begin Source File

SOURCE=.\Dx9Texture.h
# End Source File
# Begin Source File

SOURCE=.\gd_dx9.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\toDx9.h
# End Source File
# Begin Source File

SOURCE=.\toString.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\gd_dx_common\Array.h
# End Source File
# Begin Source File

SOURCE=..\gd_dx_common\DrvObject.cpp
# End Source File
# Begin Source File

SOURCE=..\gd_dx_common\DrvObject.h
# End Source File
# Begin Source File

SOURCE=..\gd_dx_common\error.cpp
# End Source File
# Begin Source File

SOURCE=..\gd_dx_common\error.h
# End Source File
# Begin Source File

SOURCE=..\gd_dx_common\LogFile.cpp
# End Source File
# Begin Source File

SOURCE=..\gd_dx_common\LogFile.h
# End Source File
# Begin Source File

SOURCE=..\gd_dx_common\operator_new.cpp
# End Source File
# Begin Source File

SOURCE=..\gd_dx_common\Sort.cpp
# End Source File
# Begin Source File

SOURCE=..\gd_dx_common\Sort.h
# End Source File
# Begin Source File

SOURCE=..\gd_dx_common\toSurfaceFormat.cpp
# End Source File
# Begin Source File

SOURCE=..\gd_dx_common\toSurfaceFormat.h
# End Source File
# End Group
# End Target
# End Project
