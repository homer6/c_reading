;
; Dead Justice install script 1.0
;
; Supports
; - custom install path
; - optional start menu shortcuts
; - uninstall
; 

; The name of the installer
Name "Dead Justice Demo"

; The file to write
OutFile "djdemo.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\Dead Justice Demo"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\DJ_Demo "Install_Dir"

; The text to prompt the user to enter a directory
ComponentText "This will install playable Dead Justice demo version on your computer. Please select install options"

; The text to prompt the user to enter a directory
DirText "Choose a directory to install in to:"

; The stuff to install
Section "Demo (required)"
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  ; Put file there
  File /r deadjustice\*.*
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\DJ_Demo "Install_Dir" "$INSTDIR"
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DJ_Demo" "DisplayName" "Dead Justice Demo (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DJ_Demo" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteUninstaller "uninstall.exe"
SectionEnd

; optional section
Section "Start Menu and desktop shortcuts"
  CreateDirectory "$SMPROGRAMS\Dead Justice Demo"
  CreateShortCut "$SMPROGRAMS\Dead Justice Demo\Dead Justice Demo.lnk" "$INSTDIR\deadjustice.exe" "" "$INSTDIR\deadjustice.exe" 0
  CreateShortCut "$SMPROGRAMS\Dead Justice Demo\Cat Mother Website.lnk" "http://www.catmother.com" "" "http://www.catmother.com" 0
  CreateShortCut "$SMPROGRAMS\Dead Justice Demo\readme.txt.lnk" "$INSTDIR\readme.txt" "" "$INSTDIR\readme.txt" 0
  CreateShortCut "$SMPROGRAMS\Dead Justice Demo\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$DESKTOP\Dead Justice Demo.lnk" "$INSTDIR\deadjustice.exe" "" "$INSTDIR\deadjustice.exe" 0
SectionEnd

; Uninstall stuff

UninstallText "This will uninstall Dead Justice Demo. Hit next to continue."

; special uninstall section.
Section "Uninstall"
  ; remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DJ_Demo"
  DeleteRegKey HKLM SOFTWARE\DJ_Demo
  ; remove files
  Delete $INSTDIR\*.*
  ; remove shortcuts, if any.
  Delete "$SMPROGRAMS\Dead Justice Demo\*.*"
  Delete "$DESKTOP\Dead Justice Demo.lnk"
  ; remove directories used.
  RMDir "$SMPROGRAMS\Dead Justice Demo"
  RMDir /r "$INSTDIR"
SectionEnd

; eof
