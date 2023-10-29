@echo off
setlocal enabledelayedexpansion

rem Check if devenv.exe is in the PATH
set DEVENV=devenv
where /Q !DEVENV!

if %ERRORLEVEL% neq 0 (
	rem Try to detect Visual Studio .NET 2003
	set DEVENV="%VS71COMNTOOLS%\..\IDE\devenv.com"

	if not exist !DEVENV! (
		echo Failed to detect Visual Studio environment. Please run this inside of a x86 developer command prompt!
		exit /B 1
	)
)

if /I "%1" == "clean" (
	set CMD=clean
	set CONFIG="%2"
) else (
	set CMD=build
	set CONFIG="%1"
)

if /I %CONFIG% == "release" (
	set CONFIG=release
) else (
	set CONFIG=debug
)

!DEVENV! /%CMD% %CONFIG% CT.sln
