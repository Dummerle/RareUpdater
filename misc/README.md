## qt-mingw64-static.ps1

 .SYNOPSIS

  Build a static version of Qt for Windows.

 .DESCRIPTION

  This scripts compiles and installs a static version of Qt.
  It assumes that a prebuilt Qt / MinGW environment and the Qt source are
  already installed, typically in C:\Qt. This prebuilt environment uses shared
  libraries. It is supposed to remain the main development environment for Qt.
  This script adds a static version of the Qt libraries in order to allow the
  construction of standalone and self-sufficient executable.

  This script is typically run from the Windows Explorer.

  To allow scripts to run in powershell
  - `set-executionpolicy remotesigned`

  Requirements:
  - Windows PowerShell 3.0 or higher.

 .PARAMETER `QtDir`

  The Directory where Qt is installed

 .PARAMETER `QtBuildDir`

  The directory the scripp will build Qt. The default Z: drive
  is a ramdisk using ImDisk for example. ImDisk is free and opensource.
  https://sourceforge.net/projects/imdisk-toolkit/

 .PARAMETER `QtVersion`

  The version of Qt to build. It should be already downloaded though the
  Qt Maintenance Tool. It also needs OpenSSL installed through the Maintenance Tool.

 .PARAMETER `RubyPath`

  The top-level directory where Ruby is installed.
  Grab it from here: https://rubyinstaller.org/downloads/

  PARAMETER `PerlPath`

  The top-level directory where Straberry Perl is installed.
  Grab it from here: https://strawberryperl.com/
