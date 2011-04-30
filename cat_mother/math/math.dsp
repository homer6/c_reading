# Microsoft Developer Studio Project File - Name="math" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=math - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "math.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "math.mak" CFG="math - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "math - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "math - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "math"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "math - Win32 Release"

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
# ADD BASE RSC /l 0x40b /d "NDEBUG"
# ADD RSC /l 0x40b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\math.lib"

!ELSEIF  "$(CFG)" == "math - Win32 Debug"

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
# ADD BASE RSC /l 0x40b /d "_DEBUG"
# ADD RSC /l 0x40b /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\mathd.lib"

!ENDIF 

# Begin Target

# Name "math - Win32 Release"
# Name "math - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BezierUtilVector3.cpp
# End Source File
# Begin Source File

SOURCE=.\BSphere.cpp
# End Source File
# Begin Source File

SOURCE=.\BSphereBuilder.cpp
# End Source File
# Begin Source File

SOURCE=.\EigenUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\Intersection.cpp
# End Source File
# Begin Source File

SOURCE=.\Matrix3x3.cpp
# End Source File
# Begin Source File

SOURCE=.\Matrix4x4.cpp
# End Source File
# Begin Source File

SOURCE=.\MatrixMxN.cpp
# End Source File
# Begin Source File

SOURCE=.\Noise.cpp
# End Source File
# Begin Source File

SOURCE=.\OBBox.cpp
# End Source File
# Begin Source File

SOURCE=.\OBBoxBuilder.cpp
# End Source File
# Begin Source File

SOURCE=.\Quaternion.cpp
# End Source File
# Begin Source File

SOURCE=.\ShapeBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\Vector3.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BezierUtil.h
# End Source File
# Begin Source File

SOURCE=.\BezierUtil.inl
# End Source File
# Begin Source File

SOURCE=.\BezierUtilVector3.h
# End Source File
# Begin Source File

SOURCE=.\BSphere.h
# End Source File
# Begin Source File

SOURCE=.\BSphereBuilder.h
# End Source File
# Begin Source File

SOURCE=.\EigenUtil.h
# End Source File
# Begin Source File

SOURCE=.\FloatUtil.h
# End Source File
# Begin Source File

SOURCE=.\FloatUtil.inl
# End Source File
# Begin Source File

SOURCE=.\Intersection.h
# End Source File
# Begin Source File

SOURCE=.\lerp.h
# End Source File
# Begin Source File

SOURCE=.\Matrix3x3.h
# End Source File
# Begin Source File

SOURCE=.\Matrix3x3.inl
# End Source File
# Begin Source File

SOURCE=.\Matrix4x4.h
# End Source File
# Begin Source File

SOURCE=.\Matrix4x4.inl
# End Source File
# Begin Source File

SOURCE=.\MatrixMxN.h
# End Source File
# Begin Source File

SOURCE=.\Noise.h
# End Source File
# Begin Source File

SOURCE=.\OBBox.h
# End Source File
# Begin Source File

SOURCE=.\OBBoxBuilder.h
# End Source File
# Begin Source File

SOURCE=.\Quaternion.h
# End Source File
# Begin Source File

SOURCE=.\ShapeBuffer.h
# End Source File
# Begin Source File

SOURCE=.\Vector2.h
# End Source File
# Begin Source File

SOURCE=.\Vector3.h
# End Source File
# Begin Source File

SOURCE=.\Vector4.h
# End Source File
# Begin Source File

SOURCE=.\VectorN.h
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
# End Group
# End Target
# End Project
