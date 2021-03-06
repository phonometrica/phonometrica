; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Phonometrica"
#define MyAppVersion "0.8.0-preview2"
#define MyAppPublisher "Julien Eychenne & L�a Courd�s-Murphy"
#define MyAppURL "http://www.phonometrica-ling.org"
#define MyAppExeName "phonometrica.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{BCFF6817-CA0C-4A62-890F-BB2BFA35E2D3}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf64}\{#MyAppName}
DisableDirPage=no
UsePreviousAppDir=yes
DisableProgramGroupPage=yes
LicenseFile="C:\Users\Julien Eychenne\source\repos\phonometrica\LICENSE"
OutputBaseFilename=setup_phonometrica
Compression=lzma
SolidCompression=yes
OutputDir=C:\Devel

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\Devel\build\release\phonometrica.exe"; DestDir: "{app}"; Flags: ignoreversion
;Source: "C:\Devel\build\release\phonometrica.exe.manifest"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Julien Eychenne\source\repos\phonometrica\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Devel\Deploy\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
;Source: "C:\Users\Julien\Documents\Devel\phonometrica\std\*"; DestDir: "{app}\std"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\Users\Julien Eychenne\source\repos\phonometrica\html\*"; DestDir: "{app}\html"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\Users\Julien Eychenne\source\repos\phonometrica\licenses\*"; DestDir: "{app}\other_licenses"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{commonprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

