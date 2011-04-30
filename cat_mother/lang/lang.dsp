# Microsoft Developer Studio Project File - Name="lang" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=lang - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "lang.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "lang.mak" CFG="lang - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "lang - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "lang - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "lang"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "lang - Win32 Release"

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
# ADD CPP /nologo /MD /W4 /GR /GX /O2 /I "internal" /I "..\external\gc\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "LANG_WIN32" /YX /FD /c
# ADD BASE RSC /l 0x40b /d "NDEBUG"
# ADD RSC /l 0x40b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\lang.lib"

!ELSEIF  "$(CFG)" == "lang - Win32 Debug"

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
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /ZI /Od /I "internal" /I "..\external\gc\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "LANG_WIN32" /YX /FD /GZ /c
# ADD BASE RSC /l 0x40b /d "_DEBUG"
# ADD RSC /l 0x40b /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\langd.lib"
# Begin Special Build Tool
TargetPath=.\lib\langd.lib
SOURCE="$(InputPath)"
PostBuild_Cmds=lib $(TargetPath) ../mem/lib/memd.lib
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "lang - Win32 Release"
# Name "lang - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Character.cpp
# End Source File
# Begin Source File

SOURCE=.\Double.cpp
# End Source File
# Begin Source File

SOURCE=.\Float.cpp
# End Source File
# Begin Source File

SOURCE=.\Integer.cpp
# End Source File
# Begin Source File

SOURCE=.\Long.cpp
# End Source File
# Begin Source File

SOURCE=.\Math.cpp
# ADD CPP /U "_MSC_EXTENSIONS"
# End Source File
# Begin Source File

SOURCE=.\Object.cpp
# End Source File
# Begin Source File

SOURCE=.\Short.cpp
# End Source File
# Begin Source File

SOURCE=.\String.cpp
# End Source File
# Begin Source File

SOURCE=.\System.cpp
# End Source File
# Begin Source File

SOURCE=.\Thread.cpp
# End Source File
# Begin Source File

SOURCE=.\Throwable.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Array.h
# End Source File
# Begin Source File

SOURCE=.\Char.h
# End Source File
# Begin Source File

SOURCE=.\Character.h
# End Source File
# Begin Source File

SOURCE=.\Double.h
# End Source File
# Begin Source File

SOURCE=.\Error.h
# End Source File
# Begin Source File

SOURCE=.\Exception.h
# End Source File
# Begin Source File

SOURCE=.\Float.h
# End Source File
# Begin Source File

SOURCE=.\Integer.h
# End Source File
# Begin Source File

SOURCE=.\Long.h
# End Source File
# Begin Source File

SOURCE=.\Math.h
# End Source File
# Begin Source File

SOURCE=.\Number.h
# End Source File
# Begin Source File

SOURCE=.\NumberFormatException.h
# End Source File
# Begin Source File

SOURCE=.\Object.h
# End Source File
# Begin Source File

SOURCE=.\Ptr.h
# End Source File
# Begin Source File

SOURCE=.\Short.h
# End Source File
# Begin Source File

SOURCE=.\String.h
# End Source File
# Begin Source File

SOURCE=.\System.h
# End Source File
# Begin Source File

SOURCE=.\Thread.h
# End Source File
# Begin Source File

SOURCE=.\Throwable.h
# End Source File
# End Group
# Begin Group "internal"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\internal\config.h
# End Source File
# Begin Source File

SOURCE=.\internal\config_inl.h
# End Source File
# Begin Source File

SOURCE=.\internal\Mutex.cpp
# End Source File
# Begin Source File

SOURCE=.\internal\Mutex.h
# End Source File
# Begin Source File

SOURCE=.\internal\NumberParse.h
# End Source File
# Begin Source File

SOURCE=.\internal\Semaphore.cpp
# End Source File
# Begin Source File

SOURCE=.\internal\Semaphore.h
# End Source File
# Begin Source File

SOURCE=.\internal\Thread_impl.cpp
# End Source File
# Begin Source File

SOURCE=.\internal\Thread_impl.h
# End Source File
# End Group
# Begin Group "extensions"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Debug.cpp
# End Source File
# Begin Source File

SOURCE=.\Debug.h
# End Source File
# Begin Source File

SOURCE=.\DynamicLinkLibrary.cpp
# End Source File
# Begin Source File

SOURCE=.\DynamicLinkLibrary.h
# End Source File
# Begin Source File

SOURCE=.\Format.cpp
# End Source File
# Begin Source File

SOURCE=.\Format.h
# End Source File
# Begin Source File

SOURCE=.\Formattable.cpp
# End Source File
# Begin Source File

SOURCE=.\Formattable.h
# End Source File
# Begin Source File

SOURCE=.\NumberReader.cpp
# End Source File
# Begin Source File

SOURCE=.\NumberReader.h
# End Source File
# Begin Source File

SOURCE=.\UTF16.h
# End Source File
# Begin Source File

SOURCE=.\UTFConverter.cpp
# End Source File
# Begin Source File

SOURCE=.\UTFConverter.h
# End Source File
# End Group
# End Target
# End Project
