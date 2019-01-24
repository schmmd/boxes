; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=Boxes
AppVerName=Boxes 1.02
AppPublisher=Michael Schmitz
DefaultDirName={pf}\Boxes
DefaultGroupName=Boxes
AlwaysCreateUninstallIcon=yes
; uncomment the following line if you want your installation to run on NT 3.51 too.
; MinVersion=4,3.51

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; MinVersion: 4,4

[Files]
Source: "C:\Michael\Programming\C++\Games\Boxes\boxes.exe"; DestDir: "{app}"; CopyMode: alwaysoverwrite

[Icons]
Name: "{group}\Boxes"; Filename: "{app}\boxes.exe"; WorkingDir: "{app}"
Name: "{userdesktop}\Boxes"; Filename: "{app}\boxes.exe"; MinVersion: 4,4; Tasks: desktopicon; WorkingDir: "{app}"

[Run]
Filename: "{app}\boxes.exe"; Description: "Launch Boxes"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: "{app}\settings.dat"
