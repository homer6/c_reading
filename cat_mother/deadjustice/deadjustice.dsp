# Microsoft Developer Studio Project File - Name="deadjustice" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=deadjustice - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "deadjustice.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "deadjustice.mak" CFG="deadjustice - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "deadjustice - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "deadjustice - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "deadjustice - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zd /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes /map:"exe/deadjustice.map" /machine:I386 /out:"exe/deadjustice.exe"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "deadjustice - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"exe/deadjusticed.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "deadjustice - Win32 Release"
# Name "deadjustice - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BellCurve.cpp
# End Source File
# Begin Source File

SOURCE=.\BlendData.cpp
# End Source File
# Begin Source File

SOURCE=.\Blender.cpp
# End Source File
# Begin Source File

SOURCE=.\BoneCollisionBox.cpp
# End Source File
# Begin Source File

SOURCE=.\build.cpp
# End Source File
# Begin Source File

SOURCE=.\CollisionInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\ComputerControl.cpp
# End Source File
# Begin Source File

SOURCE=.\ControlBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ControlSector.cpp
# End Source File
# Begin Source File

SOURCE=.\ControlSet.cpp
# End Source File
# Begin Source File

SOURCE=.\Game.cpp
# End Source File
# Begin Source File

SOURCE=.\GameAction.cpp
# End Source File
# Begin Source File

SOURCE=.\GameBoxTrigger.cpp
# End Source File
# Begin Source File

SOURCE=.\GameBSPTree.cpp
# End Source File
# Begin Source File

SOURCE=.\GameCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\GameCell.cpp
# End Source File
# Begin Source File

SOURCE=.\GameCharacter.cpp
# End Source File
# Begin Source File

SOURCE=.\GameController.cpp
# End Source File
# Begin Source File

SOURCE=.\GameCutScene.cpp
# End Source File
# Begin Source File

SOURCE=.\GameDynamicObject.cpp
# End Source File
# Begin Source File

SOURCE=.\GameFlareSet.cpp
# End Source File
# Begin Source File

SOURCE=.\GameLevel.cpp
# End Source File
# Begin Source File

SOURCE=.\GameMusic.cpp
# End Source File
# Begin Source File

SOURCE=.\GameNoise.cpp
# End Source File
# Begin Source File

SOURCE=.\GameNoiseManager.cpp
# End Source File
# Begin Source File

SOURCE=.\GameObject.cpp
# End Source File
# Begin Source File

SOURCE=.\GameObjectList.cpp
# End Source File
# Begin Source File

SOURCE=.\GameObjectListItem.cpp
# End Source File
# Begin Source File

SOURCE=.\GamePath.cpp
# End Source File
# Begin Source File

SOURCE=.\GamePointObject.cpp
# End Source File
# Begin Source File

SOURCE=.\GamePortal.cpp
# End Source File
# Begin Source File

SOURCE=.\GameProjectile.cpp
# End Source File
# Begin Source File

SOURCE=.\GameScriptable.cpp
# End Source File
# Begin Source File

SOURCE=.\GameSphereObject.cpp
# End Source File
# Begin Source File

SOURCE=.\GameSurface.cpp
# End Source File
# Begin Source File

SOURCE=.\GameWeapon.cpp
# End Source File
# Begin Source File

SOURCE=.\GameWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\MorphAnimation.cpp
# End Source File
# Begin Source File

SOURCE=.\MovementAnimation.cpp
# End Source File
# Begin Source File

SOURCE=.\OverlayBitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\OverlayDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\OverlayFont.cpp
# End Source File
# Begin Source File

SOURCE=.\PhysicalCombatMove.cpp
# End Source File
# Begin Source File

SOURCE=.\printMemoryState.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjectileManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Script1.rc
# End Source File
# Begin Source File

SOURCE=.\TextureAnimation.cpp
# End Source File
# Begin Source File

SOURCE=.\TextureSequence.cpp
# End Source File
# Begin Source File

SOURCE=.\Timer.cpp
# End Source File
# Begin Source File

SOURCE=.\UserControl.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AnimationParams.h
# End Source File
# Begin Source File

SOURCE=.\BellCurve.h
# End Source File
# Begin Source File

SOURCE=.\BlendData.h
# End Source File
# Begin Source File

SOURCE=.\Blender.h
# End Source File
# Begin Source File

SOURCE=.\BoneCollisionBox.h
# End Source File
# Begin Source File

SOURCE=.\build.h
# End Source File
# Begin Source File

SOURCE=.\CollisionInfo.h
# End Source File
# Begin Source File

SOURCE=.\ComputerControl.h
# End Source File
# Begin Source File

SOURCE=.\ControlBase.h
# End Source File
# Begin Source File

SOURCE=.\ControlSector.h
# End Source File
# Begin Source File

SOURCE=.\ControlSet.h
# End Source File
# Begin Source File

SOURCE=.\Game.h
# End Source File
# Begin Source File

SOURCE=.\GameAction.h
# End Source File
# Begin Source File

SOURCE=.\GameBoxTrigger.h
# End Source File
# Begin Source File

SOURCE=.\GameBSPTree.h
# End Source File
# Begin Source File

SOURCE=.\GameCamera.h
# End Source File
# Begin Source File

SOURCE=.\GameCell.h
# End Source File
# Begin Source File

SOURCE=.\GameCharacter.h
# End Source File
# Begin Source File

SOURCE=.\GameController.h
# End Source File
# Begin Source File

SOURCE=.\GameCutScene.h
# End Source File
# Begin Source File

SOURCE=.\GameDynamicObject.h
# End Source File
# Begin Source File

SOURCE=.\GameFlare.h
# End Source File
# Begin Source File

SOURCE=.\GameFlareSet.h
# End Source File
# Begin Source File

SOURCE=.\GameLevel.h
# End Source File
# Begin Source File

SOURCE=.\GameMusic.h
# End Source File
# Begin Source File

SOURCE=.\GameNoise.h
# End Source File
# Begin Source File

SOURCE=.\GameNoiseManager.h
# End Source File
# Begin Source File

SOURCE=.\GameNoiseManager.inl
# End Source File
# Begin Source File

SOURCE=.\GameObject.h
# End Source File
# Begin Source File

SOURCE=.\GameObject.inl
# End Source File
# Begin Source File

SOURCE=.\GameObjectList.h
# End Source File
# Begin Source File

SOURCE=.\GameObjectListItem.h
# End Source File
# Begin Source File

SOURCE=.\GamePath.h
# End Source File
# Begin Source File

SOURCE=.\GamePointObject.h
# End Source File
# Begin Source File

SOURCE=.\GamePortal.h
# End Source File
# Begin Source File

SOURCE=.\GameProjectile.h
# End Source File
# Begin Source File

SOURCE=.\GameRenderPass.h
# End Source File
# Begin Source File

SOURCE=.\GameScriptable.h
# End Source File
# Begin Source File

SOURCE=.\GameSphereObject.h
# End Source File
# Begin Source File

SOURCE=.\GameSurface.h
# End Source File
# Begin Source File

SOURCE=.\GameWeapon.h
# End Source File
# Begin Source File

SOURCE=.\GameWindow.h
# End Source File
# Begin Source File

SOURCE=.\MorphAnimation.h
# End Source File
# Begin Source File

SOURCE=.\MovementAnimation.h
# End Source File
# Begin Source File

SOURCE=.\OverlayBitmap.h
# End Source File
# Begin Source File

SOURCE=.\OverlayDisplay.h
# End Source File
# Begin Source File

SOURCE=.\OverlayFont.h
# End Source File
# Begin Source File

SOURCE=.\PhysicalCombatMove.h
# End Source File
# Begin Source File

SOURCE=.\printMemoryState.h
# End Source File
# Begin Source File

SOURCE=.\ProjectileManager.h
# End Source File
# Begin Source File

SOURCE=.\ScriptUtil.h
# End Source File
# Begin Source File

SOURCE=.\TextureAnimation.h
# End Source File
# Begin Source File

SOURCE=.\TextureSequence.h
# End Source File
# Begin Source File

SOURCE=.\Timer.h
# End Source File
# Begin Source File

SOURCE=.\UserControl.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\exe\deadjustice\deadjustice.prop
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# End Group
# Begin Group "docs"

# PROP Default_Filter "*"
# Begin Source File

SOURCE=.\docs\AI_fighting.txt
# End Source File
# Begin Source File

SOURCE=.\docs\AI_head_turning.txt
# End Source File
# Begin Source File

SOURCE=.\docs\AI_scripting.txt
# End Source File
# Begin Source File

SOURCE=.\docs\build_prop.txt
# End Source File
# Begin Source File

SOURCE=.\docs\camera_commands.txt
# End Source File
# Begin Source File

SOURCE=.\docs\character_system.txt
# End Source File
# Begin Source File

SOURCE=.\docs\cut_scenes.txt
# End Source File
# Begin Source File

SOURCE=.\docs\deadjustice.txt
# End Source File
# Begin Source File

SOURCE=.\docs\DI_keys.txt
# End Source File
# Begin Source File

SOURCE=.\docs\level_rendering.txt
# End Source File
# Begin Source File

SOURCE=.\docs\movement_animation_notes.txt
# End Source File
# Begin Source File

SOURCE=.\docs\movement_animations.sxd
# End Source File
# Begin Source File

SOURCE=.\docs\moving_in_level.txt
# End Source File
# Begin Source File

SOURCE=.\docs\multiblend.txt
# End Source File
# Begin Source File

SOURCE=.\docs\NPC_states.sxd
# End Source File
# Begin Source File

SOURCE=.\docs\object_hierarchy.pla
# End Source File
# Begin Source File

SOURCE=..\docs\portalization.txt
# End Source File
# Begin Source File

SOURCE=.\docs\prop_profile.txt
# End Source File
# Begin Source File

SOURCE=.\docs\readme.txt
# End Source File
# Begin Source File

SOURCE=.\docs\script_ref.txt
# End Source File
# End Group
# End Target
# End Project
