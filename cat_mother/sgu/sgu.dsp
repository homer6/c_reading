# Microsoft Developer Studio Project File - Name="sgu" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=sgu - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sgu.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sgu.mak" CFG="sgu - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sgu - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sgu - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "sgu"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sgu - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "build/Release"
# PROP Intermediate_Dir "build/Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W4 /GR /GX /O2 /I "internal" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x40b /d "NDEBUG"
# ADD RSC /l 0x40b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\sgu.lib"

!ELSEIF  "$(CFG)" == "sgu - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "build/Debug"
# PROP Intermediate_Dir "build/Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /ZI /Od /I "internal" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x40b /d "_DEBUG"
# ADD RSC /l 0x40b /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\sgud.lib"

!ENDIF 

# Begin Target

# Name "sgu - Win32 Release"
# Name "sgu - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CameraUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\ContextUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\LineListUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\LookAtControl.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\ModelFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ModelFileCache.cpp
# End Source File
# Begin Source File

SOURCE=.\NodeGroupSet.cpp
# End Source File
# Begin Source File

SOURCE=.\NodeUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\SceneFile.cpp
# End Source File
# Begin Source File

SOURCE=.\SceneManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ShadowUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\TextureUtil.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CameraUtil.h
# End Source File
# Begin Source File

SOURCE=.\ContextUtil.h
# End Source File
# Begin Source File

SOURCE=.\LineListUtil.h
# End Source File
# Begin Source File

SOURCE=.\LookAtControl.h
# End Source File
# Begin Source File

SOURCE=.\MeshUtil.h
# End Source File
# Begin Source File

SOURCE=.\ModelFile.h
# End Source File
# Begin Source File

SOURCE=.\ModelFileCache.h
# End Source File
# Begin Source File

SOURCE=.\NodeGroupSet.h
# End Source File
# Begin Source File

SOURCE=.\NodeUtil.h
# End Source File
# Begin Source File

SOURCE=.\SceneFile.h
# End Source File
# Begin Source File

SOURCE=.\SceneManager.h
# End Source File
# Begin Source File

SOURCE=.\ShadowUtil.h
# End Source File
# Begin Source File

SOURCE=.\TextureUtil.h
# End Source File
# End Group
# Begin Group "internal"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\internal\ChunkUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\internal\ChunkUtil.h
# End Source File
# Begin Source File

SOURCE=.\internal\config.h
# End Source File
# End Group
# Begin Group "docs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\docs\fileformat_gm.txt
# End Source File
# Begin Source File

SOURCE=..\docs\fileformat_sg.txt
# End Source File
# End Group
# End Target
# End Project
