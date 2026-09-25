@echo off
setlocal enabledelayedexpansion

rem Check if devenv.exe is in the PATH
set DEVENV=devenv
where /Q !DEVENV!

if %ERRORLEVEL% neq 0 (
	rem Try to detect Visual Studio .NET 2003
	set DEVENV="%VS71COMNTOOLS%\..\IDE\devenv.com"

	if not exist !DEVENV! (
		echo Failed to detect Visual Studio environment. Please run this inside of an x86 developer command prompt!
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

pushd %CD%
cd %~dp0

if not exist ..\..\build\build.h (
	rem If build.h is not available build the VS solution
	!DEVENV! /%CMD% %CONFIG% CT.sln
) else (
	if %CMD% == clean (
		if exist .build (
			rmdir /S /Q .build
		)
	) else (
		if not exist .build\build.exe (
			mkdir .build
			cl build.c /Fe.build\build.exe
		)

		.build\build.exe --%CONFIG% --install
	)
)
