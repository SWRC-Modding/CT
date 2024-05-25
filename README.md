# C++ Headers for Star Wars Republic Commando

This repository contains the C++ headers for Star Wars Republic Commando and everything that is needed to create mods written in C++. The code is based on the Unreal Tournament v432 public sources. It was made compatible with Republic Commando by reverse engineering the Game's binaries. Since this is an ongoing effort, not everything is 100% usable and there is still some information missing. But creating gameplay mods and some more advanced things are possible and are already being done in existing projects.  
The project is called `CT` because that was the name used for the original project files when the game was in development.

## Projects contained in this repository
The following projects are part of the Visual Studio solution and built by default:
* __UCC__  
	An command line tool to compile UnrealScript code, run a dedicated server and more.
* __CT__  
	A custom game executable that adds some useful functionality like changing render devices at runtime and an improved EditActor command.  
	It also loads the Republic Commando Fix at startup if it is installed.  
	This is not required for the fix to work. It will also be loaded automatically by the default executable with a modified System.ini (comes with the download).
* __Mod__  
	A Fix for the infamous bump mapping crash, other smaller fixes and some useful functionality for modding like UnrealScript code injection.
* __ModMPGame__  
	A multiplayer addon that adds admin commands, bot support and a skin changer for skins like Delta 38 and other normally unavailable ones.  
	This is a server only mod. The clients don't need to have anything installed.
* __OpenGLDrv__  
	An OpenGL 4.5 renderer.
* __ModEd__  
	A modified UnrealEd executable with some enhancements.

## Building
First you need to clone this repository into the Republic Commando installation directory next to `GameData`. It should be called `Code` because that's where the game expects the UnrealScript files. But the name can be configured in `System.ini`.
```
git clone https://github.com/SWRC-Modding/CT.git Code
```

You can then just open the solution `CT.sln` and build it from within Visual Studio but you can also use `build.bat` if you want to use a different text editor or just wish to build on the command line. It is used as follows:
```
build.bat <configuration>
build.bat clean <configuration>
```
where `<configuration>` is either `debug` or `release`. `debug` is the default if no arguments are given.  
All dlls and executables are automatically copied into `..\GameData\System`.  

If you want to use a more recent Visual Studio version you need to first open the solution and convert it. Then you can use the `build.bat` file or build from within the IDE.  
Everything was tested to work with Visual Studio 2022 without any issues.  
  
Follow this [guide](https://wiki.swrc-modding.net/index.php?title=Writing_Native_Code) on how to create C++ based mods.  

Join our modding [Discord](https://discord.gg/3u69jMa) and check out the [Wiki](https://wiki.swrc-modding.net/index.php?title=Main_Page)!  
