# Microsoft Developer Studio Project File - Name="pix" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=pix - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pix.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pix.mak" CFG="pix - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pix - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "pix - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "pix"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pix - Win32 Release"

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
# ADD CPP /nologo /MD /W4 /GX /O2 /I "." /I "internal" /I "..\external\libjpeg" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x40b /d "NDEBUG"
# ADD RSC /l 0x40b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\pix.lib"

!ELSEIF  "$(CFG)" == "pix - Win32 Debug"

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
# ADD CPP /nologo /MDd /W4 /Gm /GX /ZI /Od /I "." /I "internal" /I "..\external\libjpeg" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x40b /d "_DEBUG"
# ADD RSC /l 0x40b /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\pixd.lib"

!ENDIF 

# Begin Target

# Name "pix - Win32 Release"
# Name "pix - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Color.cpp
# End Source File
# Begin Source File

SOURCE=.\Colorf.cpp
# End Source File
# Begin Source File

SOURCE=.\Image.cpp
# End Source File
# Begin Source File

SOURCE=.\Surface.cpp
# End Source File
# Begin Source File

SOURCE=.\SurfaceFormat.cpp
# End Source File
# Begin Source File

SOURCE=.\SurfaceUtil.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Color.h
# End Source File
# Begin Source File

SOURCE=.\Colorf.h
# End Source File
# Begin Source File

SOURCE=.\Image.h
# End Source File
# Begin Source File

SOURCE=.\Surface.h
# End Source File
# Begin Source File

SOURCE=.\SurfaceFormat.h
# End Source File
# Begin Source File

SOURCE=.\SurfaceUtil.h
# End Source File
# End Group
# Begin Group "internal"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\internal\config.h
# End Source File
# Begin Source File

SOURCE=.\internal\loadBMP.cpp
# End Source File
# Begin Source File

SOURCE=.\internal\loadBMP.h
# End Source File
# Begin Source File

SOURCE=.\internal\loadDDS.cpp
# End Source File
# Begin Source File

SOURCE=.\internal\loadDDS.h
# End Source File
# Begin Source File

SOURCE=.\internal\loadJPEG.cpp
# End Source File
# Begin Source File

SOURCE=.\internal\loadJPEG.h
# End Source File
# Begin Source File

SOURCE=.\internal\loadTGA.cpp
# End Source File
# Begin Source File

SOURCE=.\internal\loadTGA.h
# End Source File
# Begin Source File

SOURCE=.\internal\message.h
# End Source File
# End Group
# End Target
# End Project
