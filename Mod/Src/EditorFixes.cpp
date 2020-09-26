#include "../../Engine/Inc/Engine.h"
#include "../Inc/Mod.h"

void(__fastcall*OriginalUEditorEngineTick)(void*, DWORD, FLOAT) = NULL;

void __fastcall UEditorEngineTickOverride(void* Self, DWORD edx, FLOAT DeltaTime){
	static bool appliedFixes = false;

	if(!appliedFixes){
		appliedFixes = true;

		GLog->Log("Applying crash fixes");
		guard(ApplyEditorFixes)
		/*
		 * Placing a FluidSurfaceInfo crashes because the constructor for FDynamicActor accesses its 'Skins' array which is empty by default.
		 * To fix this we simply resize the property in the default actor so that it contains an element.
		 */
		AFluidSurfaceInfo::StaticClass()->GetDefaultActor()->Skins.Set(1);
		// TODO: Add more
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
		if(!GIsEditor)
			return;

		void* handle = appGetDllHandle(*(FStringTemp(appBaseDir()) * appPackage() + ".exe")); // This works even if the exe was renamed
		check(handle);
		void** vfPtr = static_cast<void**>(appGetDllExport(handle, "??_7UUnrealEdEngine@@6BUObject@@@"));
		check(vfPtr);

		OriginalUEditorEngineTick = static_cast<void(__fastcall*)(void*, DWORD, FLOAT)>(PatchVTable(&vfPtr, 32, UEditorEngineTickOverride));

		appFreeDllHandle(handle); // Not actually freeing, just decrementing the refCount in case that could break something later
	}
} GEditorFixes;
