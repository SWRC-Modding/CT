#include "../../Engine/Inc/Engine.h"
#include "../Inc/Mod.h"

/*
 * Property window crash fix
 */

bool(__fastcall*OriginalUClassIsDefaultValue)(UObject* Self, DWORD, const FPropertyInstance&) = NULL;

bool __fastcall UClassIsDefaultValueOverride(UObject* Self, DWORD edx, const FPropertyInstance& PropertyInstance){
	for(INT i = 1; i < PropertyInstance.NestedProperties.Num(); ++i){
		if(PropertyInstance.NestedProperties[i].Property->IsA(UStrProperty::StaticClass()))
			return false;
	}

	return OriginalUClassIsDefaultValue(Self, edx, PropertyInstance);
}

/*
 * UEditorEngine tick hook
 */

void(__fastcall*OriginalUEditorEngineTick)(USubsystem* Self, DWORD, FLOAT) = NULL;

void __fastcall UEditorEngineTickOverride(USubsystem* Self, DWORD edx, FLOAT DeltaTime){
	static bool appliedFixes = false;

	if(!appliedFixes){
		guard(ApplyEditorFixes)
		appliedFixes = true;

		GLog->Log("Applying crash fixes");
		/*
		 * Fix 1:
		 * Placing a FluidSurfaceInfo crashes because the constructor for FDynamicActor accesses its 'Skins' array which is empty by default.
		 * To fix this we simply resize the property in the default actor so that it contains an element.
		 */
		AFluidSurfaceInfo::StaticClass()->GetDefaultActor()->Skins.Set(1);
		/*
		 * Fix 2:
		 * Expanding a struct that contains a string property in an object property window causes a crash while the editor is checking for modified values
		 * in order to highlight them.
		 * This is fixed here by simply checking if there is a nested string property and not calling the original code if there is.
		 * As a downside this causes the string property to be drawn as bold text even though it may have not been modified but this shouldn't be a big deal
		 * since that case is rather rare anyway.
		 */
		OriginalUClassIsDefaultValue = static_cast<bool(__fastcall*)(UObject*, DWORD, const FPropertyInstance&)>(PatchVTable(UClass::StaticClass(), 29, UClassIsDefaultValueOverride));
		unguard;
	}

	OriginalUEditorEngineTick(Self, edx, DeltaTime);
}

/*
 * This class is instantiated when the dll is loaded.
 * It hooks the UnrealEdEngine::Tick function to perform initialization.
 * We have to use tick because we know that the engine is fully initialized once it is called.
 */
MOD_API struct EditorFixes{
	EditorFixes(){
		if(!GIsEditor || GIsUCC)
			return;

		OriginalUEditorEngineTick = static_cast<void(__fastcall*)(USubsystem*, DWORD, FLOAT)>(
			PatchDllClassVTable(*(FStringTemp(appBaseDir()) * appPackage() + ".exe"), "UUnrealEdEngine", "UObject", 32, UEditorEngineTickOverride)
		);
	}
} GEditorFixes;
