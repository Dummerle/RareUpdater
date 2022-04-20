# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

# Tested with QT 5.15.2 on Windows 10
# https://mrfaptastic.github.io

<#
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

 .PARAMETER `NoPause`

  Do not wait for the user to press <enter> at end of execution. By default,
  execute a "pause" instruction at the end of execution, which is useful
  when the script was run from Windows Explorer.
#>

[CmdletBinding()]
param(
    $QtDir = "C:\Qt",
    $QtBuildDir = "Z:\Qt5-Static",
    $QtVersion = "5.15.2",
    $RubyPath = "C:\Ruby31-x64",
    $PerlPath = "C:\Strawberry",
    [switch]$NoPause = $false
)

# PowerShell execution policy.
Set-StrictMode -Version 3

#-----------------------------------------------------------------------------
# Main code
#-----------------------------------------------------------------------------

function Main
{
    Remove-Item $QtBuildDir -Recurse
    Create-Directory $QtBuildDir

    $QtToolsDir = "$QtDir\Tools"
    $QtMingwDir = "$QtToolsDir\mingw810_64"
    $OpenSSLDir = "$QtToolsDir\OpenSSL\Win_x64"

    $QtStaticDir = "$QtDir\Static" # NO TRAILING SLASH

    # Qt installation directory.
    $QtDstDir = "$QtStaticDir\$QtVersion-msvc2015"

    # Build the directory tree where the static version of Qt will be installed.
    Create-Directory $QtStaticDir
    Remove-Item $QtDstDir -Recurse
    Create-Directory $QtDstDir

    # Directory of expanded packages.
    $QtSrcDir = "$QtDir\$QtVersion\Src"



    # Set a clean path including MSVC2015.
    $env:Path = "$env:SystemRoot\system32;$env:SystemRoot;$env:SystemRoot\system32\WindowsPowerShell\v1.0\;"
    $env:Path = "C:\Program Files (x86)\Microsoft SDKs\Windows\v8.1A\bin\NETFX 4.5.1 Tools\x64\;$env:Path"
    $env:Path = "C:\Program Files (x86)\HTML Help Workshop;$env:Path"
    $env:Path = "C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools;$env:Path"
    $env:Path = "C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE;$env:Path"
    $env:Path = "C:\Windows\Microsoft.NET\Framework64\v4.0.30319;$env:Path"
    $env:Path = "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\BIN\amd64;$env:Path"
    $env:Path = "C:\Program Files (x86)\MSBuild\14.0\bin\amd64;$env:Path"

    # Add prerequisites in the path
    $env:Path = "$OpenSSLDir\bin;$OpenSSLDir\lib;$OpenSSLDir\include;$env:Path"

    $env:Path = "$RubyPath\bin;$env:Path"
    $env:Path = "$PerlPath\c\bin;$PerlPath\perl\site\bin;$PerlPath\perl\bin;$env:Path"

    $env:Path = "$QtSrcDir\qtbase\bin;$env:Path"
    $env:Path = "$QtBuildDir\qtbase\bin;$env:Path"

    Write-Output "PATH: $env:Path"

    # Check that the 'powershell' command is available
    # ... and ruby, python and perl - as per: qt-everywhere-src-X.XX.X\README
    #
    # https://forum.qt.io/topic/118511/static-qt-environment-error-using-qt5-15-0/7
    #
    [void] (Check-prequisites)

    # Force English locale to avoid weird effects of tools localization.
    $env:LANG = "en"

    # Set environment variable QT_INSTALL_PREFIX. Documentation says it should be
    # used by configure as prefix but this does not seem to work. So, we will
    # also specify -prefix option in configure.
    $env:QT_INSTALL_PREFIX = $QtDstDir
    $env:OPENSSL_LIBS="-llibssl -llibcrypto -lcrypt32 -lgdi32 -lws2_32"

    $QtConfig = (
        "-no-glib",
        "-static",
        "-static-runtime",
        "-release",
        "-optimize-size",
        "-strip",
        "-gc-binaries",
#        "-ccache",
        "-platform win32-msvc",
        "-opensource",
        "-confirm-license",
        "-prefix $QtDstDir",
        "-ssl",
        "-openssl",
        "-openssl-linked",
        "-I $OpenSSLDir\include",
        "-L $OpenSSLDir\lib",
        "-qt-freetype",
        "-qt-harfbuzz",
        "-qt-pcre",
        "-qt-zlib",
        "-qt-libpng",
        "-qt-libjpeg",
        "-qt-sqlite",
        "-qt-tiff",
        "-qt-webp",
        "-qt-doubleconversion",
#        "-qt-assimp",
        "-no-iconv",
        "-no-dbus",
        "-no-opengl",
        "-no-zstd",
        "-no-jasper",
        "-no-mng",
        "-no-icu",
        "-no-fontconfig",
#        "-no-gstreamer",
#        "-no-wmf",
        "-sql-sqlite",
        "-no-sql-ibase",
        "-no-sql-mysql",
        "-no-sql-odbc",
        "-no-sql-psql",
        "-no-sql-sqlite2",
        "-skip qt3d",
        "-skip qtactiveqt",
        "-skip qtandroidextras",
        "-skip qtcharts",
        "-skip qtconnectivity",
        "-skip qtdatavis3d",
        "-skip qtdeclarative",
        "-skip qtdoc",
        "-skip qtgamepad",
        "-skip qtlocation",
        "-skip qtlottie",
        "-skip qtmacextras",
        "-skip qtmultimedia",
        "-skip qtnetworkauth",
        "-skip qtpurchasing",
        "-skip qtquick3d",
        "-skip qtquickcontrols",
        "-skip qtquickcontrols2",
        "-skip qtquicktimeline",
        "-skip qtremoteobjects",
        "-skip qtscript",
        "-skip qtsensors",
        "-skip qtspeech",
#        "-skip qtsvg",
        "-skip qtwayland",
        "-skip qtwebglplugin",
        "-skip qtwebview",
        "-skip webengine",
        "-make libs",
        "-nomake tools",
        "-nomake examples",
        "-nomake tests"
    ) -join ' '

    Write-Output "CONFIG: $QtConfig"

    # Configure, compile and install Qt.
    Push-Location $QtBuildDir
    cmd /c "$QtSrcDir\configure.bat $QtConfig"
    nmake -j4
    nmake install
    Pop-Location

    # Patch Qt's installed mkspecs for static build of application.
    $File = "$QtDstDir\mkspecs\win32-g++\qmake.conf"
    @"
CONFIG += static
"@ | Out-File -Append $File -Encoding Ascii

    Exit-Script
}

#-----------------------------------------------------------------------------
# A function to exit this script. The Message parameter is used on error.
#-----------------------------------------------------------------------------

function Exit-Script ([string]$Message = "")
{
    $Code = 0
    if ($Message -ne "") {
        Write-Output "ERROR: $Message"
        $Code = 1
    }
    if (-not $NoPause) {
        pause
    }
    exit $Code
}

#-----------------------------------------------------------------------------
# Silently create a directory.
#-----------------------------------------------------------------------------

function Create-Directory ([string]$Directory)
{
    [void] (New-Item -Path $Directory -ItemType "directory" -Force)
}

#-----------------------------------------------------------------------------
# Check if PowerShell is available.
#-----------------------------------------------------------------------------

function Check-prequisites
{
    if (Get-Command "powershell.exe" -ErrorAction SilentlyContinue)
    {
       echo "PowerShell is available.. This is good"
    }
    else
    {
       Exit-Script "'PowerShell' command is not available, check your windows path variable for this user account."
    }

    if (Get-Command "ruby" -ErrorAction SilentlyContinue)
    {
       echo "Ruby is available.. This is good"
    }
    else
    {
       Exit-Script "'ruby' is not available, check your windows path variable for this user account or install per the Qt everything instructions!"
    }


    if (Get-Command "python" -ErrorAction SilentlyContinue)
    {
       echo "Python is available.. This is good"
    }
    else
    {
       Exit-Script "'python' is not available, check your windows path variable for this user account or install ActivePython per the Qt everything instructions!"
    }

    if (Get-Command "perl" -ErrorAction SilentlyContinue)
    {
       echo "perl is available.. This is good"
    }
    else
    {
       Exit-Script "'perl' is not available, check your windows path variable for this user account or install ActivePerl per the Qt everything instructions!"
    }


}

#-----------------------------------------------------------------------------
# Execute main code.
#-----------------------------------------------------------------------------

. Main
