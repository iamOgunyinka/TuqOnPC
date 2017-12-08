; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "TuqOnThePC"
#define MyAppVersion "1.0.3"
#define MyAppPublisher "Froist Inc."
#define MyAppURL "https://sproot.xyz/tuq_on_pc"
#define MyAppExeName "TuqOnThePC.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{EF1A953A-112B-47C8-9722-D2674B6C8F1D}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DisableProgramGroupPage=yes
OutputBaseFilename=setup
SetupIconFile=C:\Users\Josh\Downloads\96_twH_icon.ico
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
Source: "C:\Users\Josh\Downloads\release\TuqOnThePC.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Josh\Downloads\release\libeay32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Josh\Downloads\release\openssl.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Josh\Downloads\release\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Josh\Downloads\release\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Josh\Downloads\release\Qt5Network.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Josh\Downloads\release\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Josh\Downloads\release\ssleay32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Josh\Downloads\release\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{commonprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

