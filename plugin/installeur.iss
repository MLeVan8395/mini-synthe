; Installeur Windows du Mini-synthé pédagogique (Inno Setup)
; © 2026 Mickael Le Van – tous droits réservés
; Fabriqué automatiquement par GitHub (voir .github/workflows/plugin-windows.yml)

#ifndef AppVersion
  #define AppVersion "0.1.0"
#endif

[Setup]
AppId={{8C1F6A52-3D7B-4E0A-9B1E-6A2D4F5C7E91}
AppName=Mini-synthé pédagogique
AppVersion={#AppVersion}
AppVerName=Mini-synthé pédagogique {#AppVersion}
AppPublisher=Mickael Le Van
AppPublisherURL=https://mlevan8395.github.io/mini-synthe/
AppCopyright=© 2026 Mickael Le Van – tous droits réservés
DefaultDirName={autopf}\Mini-synthe
DefaultGroupName=Mini-synthé
DisableProgramGroupPage=yes
DisableDirPage=yes
OutputDir=..\installeur
OutputBaseFilename=Installer-Mini-synthe
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
WizardStyle=modern
UninstallDisplayName=Mini-synthé pédagogique

[Languages]
Name: "fr"; MessagesFile: "compiler:Languages\French.isl"

[Types]
Name: "full"; Description: "Installation complète (plugin + logiciel autonome)"
Name: "custom"; Description: "Choisir"; Flags: iscustom

[Components]
Name: "vst3"; Description: "Plugin VST3 pour Cubase, Reason et les autres logiciels de musique"; Types: full custom
Name: "standalone"; Description: "Logiciel autonome (sans logiciel de musique)"; Types: full

[Files]
Source: "..\build\MiniSynthe_artefacts\Release\VST3\Mini-synthe.vst3\*"; DestDir: "{commoncf64}\VST3\Mini-synthe.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: vst3
Source: "..\build\MiniSynthe_artefacts\Release\Standalone\Mini-synthe.exe"; DestDir: "{app}"; Flags: ignoreversion; Components: standalone
Source: "LISEZMOI.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENCES-TIERS.txt"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\Mini-synthé"; Filename: "{app}\Mini-synthe.exe"; Components: standalone
Name: "{group}\Lisez-moi"; Filename: "{app}\LISEZMOI.txt"
Name: "{group}\Désinstaller le Mini-synthé"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\LISEZMOI.txt"; Description: "Lire le mode d'emploi"; Flags: postinstall shellexec skipifsilent unchecked
Filename: "{app}\Mini-synthe.exe"; Description: "Lancer le logiciel autonome"; Flags: postinstall nowait skipifsilent unchecked; Components: standalone
