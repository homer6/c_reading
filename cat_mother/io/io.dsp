# Microsoft Developer Studio Project File - Name="io" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=io - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "io.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "io.mak" CFG="io - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "io - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "io - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "io"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "io - Win32 Release"

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
# ADD LIB32 /nologo /out:"lib\io.lib"

!ELSEIF  "$(CFG)" == "io - Win32 Debug"

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
# ADD LIB32 /nologo /out:"lib\iod.lib"

!ENDIF 

# Begin Target

# Name "io - Win32 Release"
# Name "io - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ByteArrayInputStream.cpp
# End Source File
# Begin Source File

SOURCE=.\ByteArrayOutputStream.cpp
# End Source File
# Begin Source File

SOURCE=.\DataInputStream.cpp
# End Source File
# Begin Source File

SOURCE=.\DataOutputStream.cpp
# End Source File
# Begin Source File

SOURCE=.\File.cpp
# End Source File
# Begin Source File

SOURCE=.\FileInputStream.cpp
# End Source File
# Begin Source File

SOURCE=.\FileOutputStream.cpp
# End Source File
# Begin Source File

SOURCE=.\FilterInputStream.cpp
# End Source File
# Begin Source File

SOURCE=.\FilterOutputStream.cpp
# End Source File
# Begin Source File

SOURCE=.\InputStream.cpp
# End Source File
# Begin Source File

SOURCE=.\InputStreamReader.cpp
# End Source File
# Begin Source File

SOURCE=.\OutputStream.cpp
# End Source File
# Begin Source File

SOURCE=.\OutputStreamWriter.cpp
# End Source File
# Begin Source File

SOURCE=.\Reader.cpp
# End Source File
# Begin Source File

SOURCE=.\Writer.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ByteArrayInputStream.h
# End Source File
# Begin Source File

SOURCE=.\ByteArrayOutputStream.h
# End Source File
# Begin Source File

SOURCE=.\DataInputStream.h
# End Source File
# Begin Source File

SOURCE=.\DataOutputStream.h
# End Source File
# Begin Source File

SOURCE=.\EOFException.h
# End Source File
# Begin Source File

SOURCE=.\File.h
# End Source File
# Begin Source File

SOURCE=.\FileInputStream.h
# End Source File
# Begin Source File

SOURCE=.\FileNotFoundException.h
# End Source File
# Begin Source File

SOURCE=.\FileOutputStream.h
# End Source File
# Begin Source File

SOURCE=.\FilterInputStream.h
# End Source File
# Begin Source File

SOURCE=.\FilterOutputStream.h
# End Source File
# Begin Source File

SOURCE=.\InputStream.h
# End Source File
# Begin Source File

SOURCE=.\InputStreamReader.h
# End Source File
# Begin Source File

SOURCE=.\IOException.h
# End Source File
# Begin Source File

SOURCE=.\OutputStream.h
# End Source File
# Begin Source File

SOURCE=.\OutputStreamWriter.h
# End Source File
# Begin Source File

SOURCE=.\Reader.h
# End Source File
# Begin Source File

SOURCE=.\UnsupportedEncodingException.h
# End Source File
# Begin Source File

SOURCE=.\Writer.h
# End Source File
# End Group
# Begin Group "internal"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\internal\config.h
# End Source File
# Begin Source File

SOURCE=.\internal\StdioEx.cpp
# End Source File
# Begin Source File

SOURCE=.\internal\StdioEx.h
# End Source File
# End Group
# Begin Group "extensions"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ChunkInputStream.cpp
# End Source File
# Begin Source File

SOURCE=.\ChunkInputStream.h
# End Source File
# Begin Source File

SOURCE=.\ChunkOutputStream.cpp
# End Source File
# Begin Source File

SOURCE=.\ChunkOutputStream.h
# End Source File
# Begin Source File

SOURCE=.\CommandReader.cpp
# End Source File
# Begin Source File

SOURCE=.\CommandReader.h
# End Source File
# Begin Source File

SOURCE=.\DirectoryInputStreamArchive.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectoryInputStreamArchive.h
# End Source File
# Begin Source File

SOURCE=.\InputStreamArchive.h
# End Source File
# End Group
# End Target
# End Project
