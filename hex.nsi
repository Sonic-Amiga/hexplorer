Name "ICY Hexplorer"
OutFile "hex_setup.exe"
InstallDir $PROGRAMFILES\hexplorer
DirText "This installer will copy program files to your computer." 

ComponentText "Do you want some icons?" " " " "


Section "Copy program files"
	SetOutPath $INSTDIR
	File "..\hexplorer\hexplo.exe"
	File "..\hexplorer\about.dll"
	File "..\hexplorer\hexplorer.dat"
	File "..\hexplorer\headers.dat"
	File "..\hexplorer\mclip.dat"
	File "..\hexplorer\structures.dat"
	File "..\hexplorer\help.html"
	File "..\hexplorer\sample.hem"
	File "..\hexplorer\remove upx081 header.hem"
	File "..\hexplorer\bytes.hem"
	File "..\hexplorer\dissect 8-bit.hem"
	File "..\hexplorer\dissect 16-bit.hem"

	

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Hexplorer" "DisplayName" "ICY Hexplorer (remove only)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Hexplorer" "UninstallString" '"$INSTDIR\uninstall.exe"'
	WriteUninstaller uninstall.exe
SectionEnd

SectionDivider ""

Section "Create Desktop Icon"
CreateShortCut "$DESKTOP\Hexplorer.lnk" "$INSTDIR\hexplo.exe"
SectionEnd

Section "Create Start Menu Icons"
CreateDirectory "$SMPROGRAMS\Hexplorer"
CreateShortCut "$SMPROGRAMS\Hexplorer\Hexplorer.lnk" "$INSTDIR\hexplo.exe"
CreateShortCut "$SMPROGRAMS\Hexplorer\Help.lnk" "$INSTDIR\help.html"
SectionEnd

Section "Uninstall"
DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Hexplorer"
Delete $INSTDIR\*.* 
RMDir $INSTDIR
Delete $DESKTOP\Hexplorer.lnk
Delete $SMPROGRAMS\Hexplorer\*.*
RMDir $SMPROGRAMS\Hexplorer

SectionEnd