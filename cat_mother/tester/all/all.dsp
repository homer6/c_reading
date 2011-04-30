# Microsoft Developer Studio Project File - Name="all" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=all - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "all.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "all.mak" CFG="all - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "all - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "all - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "all - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x40b /d "NDEBUG"
# ADD RSC /l 0x40b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "all - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x40b /d "_DEBUG"
# ADD RSC /l 0x40b /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "all - Win32 Release"
# Name "all - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"

# Begin Source File

SOURCE="C:\projects\anim\test\test_VectorInterpolator.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\io\tests\test_ByteArrayStreams1.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\io\tests\test_Chunks.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\io\tests\test_DataStreams1.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\io\tests\test_DirectoryInputStreamArchive.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\io\tests\test_File.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\io\tests\test_FileStreams1.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\io\tests\test_InputStreamReader.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\lang\tests\test_Object.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\lang\tests\test_ProducerConsumer_Semaphore.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\lang\tests\test_Ref.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\lang\tests\test_String.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\lang\tests\test_StringFormat.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\lang\tests\test_System.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\lang\tests\test_Thread.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\math\tests\test_BoundBox.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\math\tests\test_BucketSort.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\math\tests\test_Curve.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\math\tests\test_Integration.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\math\tests\test_Intersection.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\math\tests\test_Matrix4x4.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\math\tests\test_Quat.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\math\tests\test_RadixSort.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\math\tests\test_Vector2.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\mb\tests\test_PolygonVertex.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\mb\tests\test_VertexMap.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\net\tests\test_InetAddress1.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\pix\tests\test_Image.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\pix\tests\test_SurfaceFormat.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\script\test\test_cppwrap.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\tester\all\test_all.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\util\tests\test_Hashtable.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\util\tests\test_Profile.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\util\tests\test_Properties.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\util\tests\test_Random.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\util\tests\test_sort.cpp"
# End Source File
# Begin Source File

SOURCE="C:\projects\util\tests\test_Vector.cpp"
# End Source File


# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
