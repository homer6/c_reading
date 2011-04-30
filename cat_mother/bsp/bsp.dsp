# Microsoft Developer Studio Project File - Name="bsp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=bsp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bsp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bsp.mak" CFG="bsp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bsp - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "bsp - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "bsp"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bsp - Win32 Release"

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
# ADD CPP /nologo /MD /W4 /GX /O2 /I "internal" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\bsp.lib"

!ELSEIF  "$(CFG)" == "bsp - Win32 Debug"

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
# ADD CPP /nologo /MDd /W4 /Gm /GX /ZI /Od /I "internal" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\bspd.lib"

!ENDIF 

# Begin Target

# Name "bsp - Win32 Release"
# Name "bsp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BSPBalanceSplitSelector.cpp
# End Source File
# Begin Source File

SOURCE=.\BSPBoxSplitSelector.cpp
# End Source File
# Begin Source File

SOURCE=.\BSPCollisionUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\BSPFile.cpp
# End Source File
# Begin Source File

SOURCE=.\BSPNode.cpp
# End Source File
# Begin Source File

SOURCE=.\BSPPolygon.cpp
# End Source File
# Begin Source File

SOURCE=.\BSPScoreFunc.cpp
# End Source File
# Begin Source File

SOURCE=.\BSPScoreFunc.h
# End Source File
# Begin Source File

SOURCE=.\BSPSplitSelector.cpp
# End Source File
# Begin Source File

SOURCE=.\BSPStorage.cpp
# End Source File
# Begin Source File

SOURCE=.\BSPTree.cpp
# End Source File
# Begin Source File

SOURCE=.\BSPTreeBuilder.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BSPBalanceSplitSelector.h
# End Source File
# Begin Source File

SOURCE=.\BSPBoxSplitSelector.h
# End Source File
# Begin Source File

SOURCE=.\BSPCollisionUtil.h
# End Source File
# Begin Source File

SOURCE=.\BSPFile.h
# End Source File
# Begin Source File

SOURCE=.\BSPNode.h
# End Source File
# Begin Source File

SOURCE=.\BSPPolygon.h
# End Source File
# Begin Source File

SOURCE=.\BSPSplitSelector.h
# End Source File
# Begin Source File

SOURCE=.\BSPStorage.h
# End Source File
# Begin Source File

SOURCE=.\BSPTree.h
# End Source File
# Begin Source File

SOURCE=.\BSPTreeBuilder.h
# End Source File
# Begin Source File

SOURCE=.\ProgressIndicator.h
# End Source File
# End Group
# End Target
# End Project
