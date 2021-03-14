@echo off

if /I "%1" == "debug" (
	set CONFIG=debug
	set CMD=%2
) else (
	set CONFIG=release
	set CMD=%1
)

if /I not "%CMD%" == "clean" set CMD="build"

"%VS71COMNTOOLS%\..\IDE\devenv.com" /%CMD% %CONFIG% CT.sln
