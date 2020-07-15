@echo off

if /I "%1" == "debug" (
	set CONFIG=debug
) else (
	set CONFIG=release
)

"%VS71COMNTOOLS%\..\IDE\devenv.com" /build %CONFIG% CT.sln
