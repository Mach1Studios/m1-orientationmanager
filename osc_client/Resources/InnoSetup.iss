[Setup]
AppName=M1-OrientationOSC
AppVersion=1.0.0
Uninstallable=no
SourceDir="{#BuildDir}"
DefaultDirName="{pf64}\Mach1\OSC-Tool"
DefaultGroupName=M1-OrientationOSC
OutputBaseFilename=M1-OrientationOSC_WIN
[Files]
Source: "build\M1-OrientationOSC_artefacts\Release\M1-OrientationOSC.exe"; DestDir: {app}; Flags: recursesubdirs
Source: "Resources\M1-OrientationManager.exe"; DestDir: {app}; Flags: recursesubdirs
Source: "Resources\settings.json"; DestDir: {app}; Flags: recursesubdirs