# Microsoft Developer Studio Project File - Name="util" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=util - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "util.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "util.mak" CFG="util - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "util - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "util - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "util"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "util - Win32 Release"

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
# ADD LIB32 /nologo /out:"lib\util.lib"

!ELSEIF  "$(CFG)" == "util - Win32 Debug"

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
# ADD LIB32 /nologo /out:"lib\utild.lib"

!ENDIF 

# Begin Target

# Name "util - Win32 Release"
# Name "util - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Hashtable.cpp
# End Source File
# Begin Source File

SOURCE=.\Properties.cpp
# End Source File
# Begin Source File

SOURCE=.\Random.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Hashtable.h
# End Source File
# Begin Source File

SOURCE=.\Hashtable.inl
# End Source File
# Begin Source File

SOURCE=.\HashtableIterator.h
# End Source File
# Begin Source File

SOURCE=.\HashtableIterator.inl
# End Source File
# Begin Source File

SOURCE=.\Properties.h
# End Source File
# Begin Source File

SOURCE=.\Random.h
# End Source File
# Begin Source File

SOURCE=.\Vector.h
# End Source File
# Begin Source File

SOURCE=.\Vector.inl
# End Source File
# End Group
# Begin Group "internal"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\internal\config.h
# End Source File
# Begin Source File

SOURCE=.\internal\HashtablePair.h
# End Source File
# End Group
# Begin Group "extensions"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Allocator.h
# End Source File
# Begin Source File

SOURCE=.\Allocator.inl
# End Source File
# Begin Source File

SOURCE=.\Equal.h
# End Source File
# Begin Source File

SOURCE=.\ExProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\ExProperties.h
# End Source File
# Begin Source File

SOURCE=.\Hash.h
# End Source File
# Begin Source File

SOURCE=.\Less.h
# End Source File
# Begin Source File

SOURCE=.\lowerBound.h
# End Source File
# End Group
# End Target
# End Project
