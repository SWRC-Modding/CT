# CT
This repository contains the C++ headers for Star Wars Republic Commando that enable modders to write native code.
This could be native functions that are callable from UnrealScript but also pure native classes that do everything in C++.

#### Visual Studio .NET 2003
In order to write native code for Republic Commando you first need Visual Studio .NET 2003 which can be found [here](https://wiki.swrc-modding.net/index.php?title=MS_Visual_Studio_2003).
Newer versions could potentially work but probably don't, so just use this one for maximum compatibility.

#### Why write native code?
Native classes can either be implemented in both UnrealScript and C++ but also purely in C++. There are two reasons for
writing native code:
##### 1. It's faster:
UnrealScript, according to Epic, is about 20 times slower than C++. That means functions that do heavy processing definitely
benefit from being implemented in native code.
##### 2. It gives access to features that UnrealScript doesn't have
UnrealScript is meant for gameplay programming. While it is quite good at that, it doesn't really provide any means to access
more advanced engine features or other low-level functionality like for example file I/O.

#### Getting started
Clone this repository into your Star Wars Republic Commando folder next to GameData with
> git clone https://github.com/Leon280698/CT.git Code

Alternatively you can also download the zip but make sure to rename the 'CT' folder to 'Code'.

Now go ahead and open the solution 'CT.sln' in Visual Studio .NET 2003. You will see that it already contains some projects.
There's 'UCC' which is a command-line tool that, among other things, is used to compile UnrealScript source code into .u
packages and to generate headers and source files for native classes.
The others are my personal projects that I store in this repository for convenience. You can use them as a reference or just
ignore/delete them if you want.

#### Creating a new project
In order to create a new native coding project, right click the solution 'CT' in the solution explorer to the left.
Select 'Add' -> 'New Project' and create a new 'Empty Project (.NET)'. We're actually not going to use .NET but this old version
of Visual Studio has problems on newer versions of windows where the create project wizard is broken. The empty .NET project
is the only (usable) one that works. From now on I'm going to refer to the new project as 'ModPackage'.
Once you have created your new project it should show up in the solution explorer. Right click it, select 'Properties' and adjust the
following settings:
* General
  * Output Directory: &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; $(SolutionDir)/\.\./GameData/System
  * Configuration Type: &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Dynamic Library (.dll)
  * Use Managed Extensions: &nbsp;&nbsp;&nbsp;&nbsp; No
  * Character Set: &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Not Set
* C/C++
  * Code Generation
    * Runtime Library: &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Multi-threaded DLL (/MD)
    * Struct Member Alignment: &nbsp;&nbsp; 4 Bytes (/Zp4)
  * Advanced
    * Calling Convention: &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; __fastcall (/Gr)
* Linker
  * Advanced
    * Import Library: &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; $(ProjectDir)/lib/$(TargetName).lib

In case you have an issue where the properties don't show up and there is just a blank gray area, you can fix it by installing .NET Framework 1.0.

Also under your project you have an item called 'References'. Right click, select 'Add Reference' and choose the 'UCC' project.
The UCC project is configured in such a way that it automatically compiles the UnrealScript code whenever you compile your native code.
Selecting UCC as a reference makes sure that the UCC.exe is built before your project so that it can generate the headers and sources that
your own project needs.

#### Writing code
Now that the project setup is out of the way, it's finally time to write some code. Open windows explorer and navigate to the folder
that contains your Visual Studio project. That should be in '(...)\\Star Wars Republic Commando\\Code\\MyPackge'.
In there create three folders called 'Classes', 'Inc' and 'Src'. It is very important that you create them manually since the ucc will not be
able to generate the native implementations for your classes if the Inc and Src folders don't exist.
The Classes folder is going to contain your UnrealScript source code. As an example I'm creating a very simple and quite useless class
that has one native function which takes in a string parameter and returns an int.
Here's what it looks like:
```Java
class MyClass extends Actor native
                            placeable;

var int TestInt;

native final function int TestNativeFunc(string param);

function PostBeginPlay(){
    local int SomeValue;

    SomeValue = TestNativeFunc("Hello World!");

    log("TestInt:"@TestInt@"SomeValue:"@SomeValue);
}
```
The class must be declared as native. It has one member called TestInt that I'm going to modify in C++ and then print out
in UnrealScript, just to demonstrate that both the UnrealScript and C++ class refer to the exact same object in memory.
Save this in your projects 'Classes' folder as 'MyClass.uc'. You now have to compile this script to a .u package and generate the
C++ header and source file for your project. In order to do this, expand the 'UCC' project and open 'ucc.ini'.
Look for the lines that say 'EditPackages+=SomePackageName' under '[Editor.EditorEngine]' and add your package like so:
> EditPackages+=ModPackage

In visual studio hit F7 or right click the solution and select 'Build'. If there are no errors in your script, the ucc should have generated
the files 'ModPackageClasses.h' and 'ModPackageClasses.cpp' in the Inc and Src folders respectively.
Drag and drop those onto your visual studio project so that they become a part of it and are recognized by the compiler.
Their content should look like this:
##### ModPackageClasses.h
```C++
/*===========================================================================
    C++ class definitions exported from UnrealScript.
    This is automatically generated by the tools.
    DO NOT modify this manually! Edit the corresponding .uc files instead!
===========================================================================*/

#ifndef ModPackage_NATIVE_DEFS
#define ModPackage_NATIVE_DEFS

#if SUPPORTS_PRAGMA_PACK
#pragma pack (push,4)
#endif

#ifndef MODPACKAGE_API
#define MODPACKAGE_API DLL_IMPORT
#endif

class MODPACKAGE_API AMyClass : public AActor
{
public:
    INT TestInt;
    void execTestNativeFunc(FFrame& Stack, void* Result);
    DECLARE_CLASS(AMyClass,AActor,0,ModPackage)
    NO_DEFAULT_CONSTRUCTOR(AMyClass)
    DECLARE_NATIVES(AMyClass)
};

#if SUPPORTS_PRAGMA_PACK
#pragma pack (pop)
#endif

#if __STATIC_LINK

#define AUTO_INITIALIZE_REGISTRANTS_MOD \
	AMyClass::StaticClass(); \

#endif // __STATIC_LINK

#endif // CORE_NATIVE_DEFS
```
##### ModPackageClasses.cpp
```C++
/*===========================================================================
    C++ class definitions exported from UnrealScript.
    This is automatically generated by the tools.
    DO NOT modify this manually! Edit the corresponding .uc files instead!
===========================================================================*/

#include "ModPackagePrivate.h"

IMPLEMENT_PACKAGE(ModPackage)

IMPLEMENT_CLASS(AMyClass);
FNativeEntry<AMyClass> AMyClass::StaticNativeMap[] = {
	MAP_NATIVE(TestNativeFunc,0)
	{NULL,NULL}
};
LINK_NATIVES(AMyClass);
```

# To be continued...