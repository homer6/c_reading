# Microsoft Developer Studio Project File - Name="sg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=sg - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sg.mak" CFG="sg - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sg - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sg - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "sg"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sg - Win32 Release"

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
# ADD LIB32 /nologo /out:"lib\sg.lib"

!ELSEIF  "$(CFG)" == "sg - Win32 Debug"

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
# ADD LIB32 /nologo /out:"lib\sgd.lib"

!ENDIF 

# Begin Target

# Name "sg - Win32 Release"
# Name "sg - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BaseTexture.cpp
# End Source File
# Begin Source File

SOURCE=.\Camera.cpp
# End Source File
# Begin Source File

SOURCE=.\Context.cpp
# End Source File
# Begin Source File

SOURCE=.\ContextObject.cpp
# End Source File
# Begin Source File

SOURCE=.\CubeTexture.cpp
# End Source File
# Begin Source File

SOURCE=.\Curve.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectLight.cpp
# End Source File
# Begin Source File

SOURCE=.\Dummy.cpp
# End Source File
# Begin Source File

SOURCE=.\EdgeHash.cpp
# End Source File
# Begin Source File

SOURCE=.\Effect.cpp
# End Source File
# Begin Source File

SOURCE=.\Font.cpp
# End Source File
# Begin Source File

SOURCE=.\Light.cpp
# End Source File
# Begin Source File

SOURCE=.\LineList.cpp
# End Source File
# Begin Source File

SOURCE=.\LOD.cpp
# End Source File
# Begin Source File

SOURCE=.\Material.cpp
# End Source File
# Begin Source File

SOURCE=.\Mesh.cpp
# End Source File
# Begin Source File

SOURCE=.\Model.cpp
# End Source File
# Begin Source File

SOURCE=.\Morpher.cpp
# End Source File
# Begin Source File

SOURCE=.\MorphTarget.cpp
# End Source File
# Begin Source File

SOURCE=.\Node.cpp
# End Source File
# Begin Source File

SOURCE=.\PatchList.cpp
# End Source File
# Begin Source File

SOURCE=.\PointLight.cpp
# End Source File
# Begin Source File

SOURCE=.\PolygonAdjacency.cpp
# End Source File
# Begin Source File

SOURCE=.\Primitive.cpp
# End Source File
# Begin Source File

SOURCE=.\Scene.cpp
# End Source File
# Begin Source File

SOURCE=.\Shader.cpp
# End Source File
# Begin Source File

SOURCE=.\ShadowShader.cpp
# End Source File
# Begin Source File

SOURCE=.\ShadowVolume.cpp
# End Source File
# Begin Source File

SOURCE=.\SpotLight.cpp
# End Source File
# Begin Source File

SOURCE=.\Sprite.cpp
# End Source File
# Begin Source File

SOURCE=.\Texture.cpp
# End Source File
# Begin Source File

SOURCE=.\TriangleList.cpp
# End Source File
# Begin Source File

SOURCE=.\VertexFormat.cpp
# End Source File
# Begin Source File

SOURCE=.\ViewFrustum.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BaseTexture.h
# End Source File
# Begin Source File

SOURCE=.\Camera.h
# End Source File
# Begin Source File

SOURCE=.\Context.h
# End Source File
# Begin Source File

SOURCE=.\ContextObject.h
# End Source File
# Begin Source File

SOURCE=.\CubeTexture.h
# End Source File
# Begin Source File

SOURCE=.\Curve.h
# End Source File
# Begin Source File

SOURCE=.\DirectLight.h
# End Source File
# Begin Source File

SOURCE=.\Dummy.h
# End Source File
# Begin Source File

SOURCE=.\Edge.h
# End Source File
# Begin Source File

SOURCE=.\EdgeHash.h
# End Source File
# Begin Source File

SOURCE=.\Effect.h
# End Source File
# Begin Source File

SOURCE=.\Font.h
# End Source File
# Begin Source File

SOURCE=.\IndexLock.h
# End Source File
# Begin Source File

SOURCE=.\Light.h
# End Source File
# Begin Source File

SOURCE=.\LineList.h
# End Source File
# Begin Source File

SOURCE=.\LockException.h
# End Source File
# Begin Source File

SOURCE=.\LOD.h
# End Source File
# Begin Source File

SOURCE=.\Material.h
# End Source File
# Begin Source File

SOURCE=.\Material.inl
# End Source File
# Begin Source File

SOURCE=.\Mesh.h
# End Source File
# Begin Source File

SOURCE=.\Model.h
# End Source File
# Begin Source File

SOURCE=.\Model.inl
# End Source File
# Begin Source File

SOURCE=.\Morpher.h
# End Source File
# Begin Source File

SOURCE=.\MorphTarget.h
# End Source File
# Begin Source File

SOURCE=.\Node.h
# End Source File
# Begin Source File

SOURCE=.\Node.inl
# End Source File
# Begin Source File

SOURCE=.\PatchList.h
# End Source File
# Begin Source File

SOURCE=.\PointLight.h
# End Source File
# Begin Source File

SOURCE=.\PolygonAdjacency.h
# End Source File
# Begin Source File

SOURCE=.\PolygonAdjacency.inl
# End Source File
# Begin Source File

SOURCE=.\Primitive.h
# End Source File
# Begin Source File

SOURCE=.\Scene.h
# End Source File
# Begin Source File

SOURCE=.\Shader.h
# End Source File
# Begin Source File

SOURCE=.\ShadowShader.h
# End Source File
# Begin Source File

SOURCE=.\ShadowVolume.h
# End Source File
# Begin Source File

SOURCE=.\SpotLight.h
# End Source File
# Begin Source File

SOURCE=.\Sprite.h
# End Source File
# Begin Source File

SOURCE=.\Texture.h
# End Source File
# Begin Source File

SOURCE=.\TriangleList.h
# End Source File
# Begin Source File

SOURCE=.\TriangleList.inl
# End Source File
# Begin Source File

SOURCE=.\VertexAndIndexLock.h
# End Source File
# Begin Source File

SOURCE=.\VertexFormat.h
# End Source File
# Begin Source File

SOURCE=.\VertexFormat.inl
# End Source File
# Begin Source File

SOURCE=.\VertexLock.h
# End Source File
# Begin Source File

SOURCE=.\ViewFrustum.h
# End Source File
# Begin Source File

SOURCE=.\ViewFrustum.inl
# End Source File
# End Group
# Begin Group "internal"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\internal\AutoPtr.h
# End Source File
# Begin Source File

SOURCE=.\internal\BoundVolume.cpp
# End Source File
# Begin Source File

SOURCE=.\internal\BoundVolume.h
# End Source File
# Begin Source File

SOURCE=.\internal\config.h
# End Source File
# Begin Source File

SOURCE=.\internal\GdUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\internal\GdUtil.h
# End Source File
# Begin Source File

SOURCE=.\internal\NodeDistanceToCameraLess.h
# End Source File
# Begin Source File

SOURCE=.\internal\TextureCache.cpp
# End Source File
# Begin Source File

SOURCE=.\internal\TextureCache.h
# End Source File
# End Group
# End Target
# End Project
