@echo off

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

if /I not "%CMD%" == "clean" set CMD="build"

"%VS71COMNTOOLS%\..\IDE\devenv.com" /%CMD% %CONFIG% CT.sln
