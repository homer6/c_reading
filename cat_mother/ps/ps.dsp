# Microsoft Developer Studio Project File - Name="ps" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ps - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ps.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ps.mak" CFG="ps - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ps - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ps - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "ps"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ps - Win32 Release"

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
# ADD LIB32 /nologo /out:"lib\ps.lib"

!ELSEIF  "$(CFG)" == "ps - Win32 Debug"

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
# ADD LIB32 /nologo /out:"lib\psd.lib"

!ENDIF 

# Begin Target

# Name "ps - Win32 Release"
# Name "ps - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Box.cpp
# End Source File
# Begin Source File

SOURCE=.\Flow.cpp
# End Source File
# Begin Source File

SOURCE=.\Gravity.cpp
# End Source File
# Begin Source File

SOURCE=.\HalfSphere.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSystemManager.cpp
# End Source File
# Begin Source File

SOURCE=.\PathParticleSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\Sphere.cpp
# End Source File
# Begin Source File

SOURCE=.\SpriteParticleSystem.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Box.h
# End Source File
# Begin Source File

SOURCE=.\Flow.h
# End Source File
# Begin Source File

SOURCE=.\Force.h
# End Source File
# Begin Source File

SOURCE=.\Gravity.h
# End Source File
# Begin Source File

SOURCE=.\HalfSphere.h
# End Source File
# Begin Source File

SOURCE=.\ParticleSystem.h
# End Source File
# Begin Source File

SOURCE=.\ParticleSystemManager.h
# End Source File
# Begin Source File

SOURCE=.\PathParticleSystem.h
# End Source File
# Begin Source File

SOURCE=.\Shape.h
# End Source File
# Begin Source File

SOURCE=.\Sphere.h
# End Source File
# Begin Source File

SOURCE=.\SpriteParticleSystem.h
# End Source File
# End Group
# Begin Group "internal"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\internal\config.h
# End Source File
# End Group
# Begin Group "docs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\docs\flo_file.txt
# End Source File
# Begin Source File

SOURCE=.\docs\psf_commands.txt
# End Source File
# End Group
# End Target
# End Project
