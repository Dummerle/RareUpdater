@echo off

set QtDir=C:\Devel\Qt
set QtBuildDir=Z:\Qt5-Static
set QtVersion=5.15.2
set PythonPath=C:\Devel\Python27
set RubyPath=C:\Devel\Ruby31-x64
set PerlPath=C:\Devel\Strawberry

rd /s/q %QtBuildDir%
mkdir %QtBuildDir%

set QtToolsDir=%QtDir%\Tools
set OpenSSLDir=%QtToolsDir%\OpenSSL\Win_x64

set QtStaticDir=%QtDir%\Static

:: Qt installation directory.
set QtDstDir=%QtStaticDir%\%QtVersion%-msvc2015-x64

:: Build the directory tree where the static version of Qt will be installed.
mkdir %QtStaticDir%
rd /s/q %QtDstDir%
mkdir %QtDstDir%

:: Directory of expanded packages.
set QtSrcDir=%QtDir%\%QtVersion%\Src

:: Set a clean path including MSVC2015.
REM set PATH=%SystemRoot%\system32;%SystemRoot%;%SystemRoot%\system32\WindowsPowerShell\v1.0\;
REM set PATH=C:\Program Files (x86)\Microsoft SDKs\Windows\v8.1A\bin\NETFX 4.5.1 Tools\x64\;%PATH%
REM set PATH=C:\Program Files (x86)\HTML Help Workshop;%PATH%
REM set PATH=C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools;%PATH%
REM set PATH=C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE;%PATH%
REM set PATH=C:\Windows\Microsoft.NET\Framework64\v4.0.30319;%PATH%
REM set PATH=C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\BIN\amd64;%PATH%
REM set PATH=C:\Program Files (x86)\MSBuild\14.0\bin\amd64;%PATH%

:: Add prerequisites in the path
set PATH=%OpenSSLDir%\bin;%OpenSSLDir%\lib;%OpenSSLDir%\include;%PATH%

set PATH=%PythonPath%;%PATH%
set PATH=%RubyPath%\bin;%PATH%
set PATH=%PerlPath%\c\bin;%PerlPath%\perl\site\bin;%PerlPath%\perl\bin;%PATH%

set PATH=%QtSrcDir%\qtbase\bin;%PATH%
set PATH=%QtBuildDir%\qtbase\bin;%PATH%

echo PATH: %PATH%

:: Force English locale to avoid weird effects of tools localization.
set LANG=en

:: Set environment variable QT_INSTALL_PREFIX. Documentation says it should be
:: used by configure as prefix but this does not seem to work. So, we will
:: also specify -prefix option in configure.
set QT_INSTALL_PREFIX=%QtDstDir%
set OPENSSL_LIBS=-llibssl -llibcrypto -lcrypt32 -lgdi32 -lws2_32

set QtConfig=^
-no-glib ^
-static ^
-static-runtime ^
-release ^
-optimize-size ^
-strip ^
-gc-binaries ^
-platform win32-msvc ^
-opensource ^
-confirm-license ^
-prefix %QtDstDir% ^
-ssl ^
-openssl ^
-openssl-linked ^
-I %OpenSSLDir%\include ^
-L %OpenSSLDir%\lib ^
-qt-freetype ^
-qt-harfbuzz ^
-qt-pcre ^
-qt-zlib ^
-qt-libpng ^
-qt-libjpeg ^
-qt-sqlite ^
-qt-tiff ^
-qt-webp ^
-qt-doubleconversion ^
-no-iconv ^
-no-dbus ^
-no-opengl ^
-no-zstd ^
-no-jasper ^
-no-mng ^
-no-icu ^
-no-fontconfig ^
-sql-sqlite ^
-no-sql-ibase ^
-no-sql-mysql ^
-no-sql-odbc ^
-no-sql-psql ^
-no-sql-sqlite2 ^
-skip qt3d ^
-skip qtactiveqt ^
-skip qtandroidextras ^
-skip qtcharts ^
-skip qtconnectivity ^
-skip qtdatavis3d ^
-skip qtdeclarative ^
-skip qtdoc ^
-skip qtgamepad ^
-skip qtlocation ^
-skip qtlottie ^
-skip qtmacextras ^
-skip qtmultimedia ^
-skip qtnetworkauth ^
-skip qtpurchasing ^
-skip qtquick3d ^
-skip qtquickcontrols ^
-skip qtquickcontrols2 ^
-skip qtquicktimeline ^
-skip qtremoteobjects ^
-skip qtscript ^
-skip qtsensors ^
-skip qtspeech ^
-skip qtwayland ^
-skip qtwebglplugin ^
-skip qtwebview ^
-skip webengine ^
-make libs ^
-nomake tools ^
-nomake examples ^
-nomake tests

echo CONFIG: %QtConfig%

:: Configure, compile and install Qt.
pushd %QtBuildDir%
%QtSrcDir%\configure.bat %QtConfig%
nmake
nmake install
popd
