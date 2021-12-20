@echo off

set JUCEDIR=C:\JUCE
set VSDIR=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE
set INNOSETUPDIR=C:\Program Files (x86)\Inno Setup 6
set WORKSPACE=..\..\..\
echo.

echo Using variables:
echo JUCEDIR = %JUCEDIR%
echo VSDIR = %VSDIR%
echo WORKSPACE = %WORKSPACE%
echo.

echo Exporting Projucer project
"%JUCEDIR%\Projucer.exe" --resave %WORKSPACE%\SpaConBridge.jucer
echo.

echo Build release
"%VSDIR%\devenv.com" %WORKSPACE%\Builds\VisualStudio2022\SpaConBridge.sln /build Release
echo.

echo Build installer
"%INNOSETUPDIR%\ISCC.exe" create_installer.iss
echo.

echo Move setup executable to workspace root
move Output\SpaConBridgeSetup*.exe %WORKSPACE%