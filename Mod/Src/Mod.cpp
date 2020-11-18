#include "../Inc/Mod.h"

/*
 * PatchVTable
 */
void* PatchVTable(void* Object, INT Index, void* NewFunc){
	DWORD OldProtect;
	void** VTable = *reinterpret_cast<void***>(Object);

	if(!VirtualProtect(VTable + Index, sizeof(void*), PAGE_EXECUTE_READWRITE, &OldProtect)){
		GLog->Logf(NAME_Error, "Unable to patch vtable: VirtualProtect failed with error code %i", GetLastError());

		return NULL;
	}

	void* OldFunc = VTable[Index];

	VTable[Index] = NewFunc;

	return OldFunc;
}

void* PatchDllClassVTable(const TCHAR* DllName, const TCHAR* ClassName, const TCHAR* VTableName, INT Index, void* NewFunc){
	void* Handle = appGetDllHandle(DllName);

	if(!Handle){
		GLog->Logf(NAME_Error, "Unable to patch vtable for class '%s': Failed to load library '%s'", ClassName, DllName);

		return NULL;
	}

	void** VTable = static_cast<void**>(appGetDllExport(Handle, *(FStringTemp("??_7") + ClassName + "@@6B" + VTableName + "@@@")));

	if(!VTable){
		GLog->Logf(NAME_Error, "Unable to patch vtable for class '%s': Dll export for vtable '%s' not found", ClassName, VTableName);

		return NULL;
	}

	appFreeDllHandle(Handle); // Decrementing reference count just in case. This might cause a crash later if the dll wasn't already loaded which is required.

	return PatchVTable(&VTable, Index, NewFunc);
}

/*
 * FunctionOverride
 */

static TMap<UFunction*, UFunctionOverride*> FunctionOverrides;

static void __fastcall ScriptFunctionHook(UObject* Self, int, FFrame& Stack, void* Result){
	/*
	 * The stack doesn't contain any information about the called function at this point.
	 * We only know that the last four bytes at the top are either a UFunction* or an FName so we need to check both.
	 */
	UFunction* Function;
	FName      FunctionName;

	appMemcpy(&Function, Stack.Code - sizeof(UFunction*), sizeof(UFunction*));

	if(!FunctionOverrides.Find(Function)){
		Function = NULL;

		appMemcpy(&FunctionName, Stack.Code - sizeof(FName), sizeof(FName));

		if((reinterpret_cast<DWORD>(FunctionName.GetEntry()) & 0xFFFF0000) != 0 && // Seems to be enough to check for a valid name
		   !IsBadReadPtr(FunctionName.GetEntry(), sizeof(FNameEntry) - NAME_SIZE)){ // Using IsBadReadPtr as a backup check just in case
			Function = Self->FindFunction(FunctionName);
		}

		if(!Function) // If the current function is not on the stack, it is an event called from C++ which is stored in Stack.Node
			Function = static_cast<UFunction*>(Stack.Node);
	}

	UFunctionOverride* Override = FunctionOverrides[Function];
	bool IsEvent = Function == Stack.Node;

	checkSlow(Override);

	Function->FunctionFlags = Override->OriginalFunctionFlags;
	Function->Func = Override->OriginalNative;

	checkSlow(!Override->CurrentSelf);

	Override->CurrentSelf = Self;

	if(Self == Override->TargetObject || !Override->TargetObject){
		if(IsEvent)
			Override->OverrideObject->ProcessEvent(Override->OverrideFunction, Stack.Locals);
		else
			Override->OverrideObject->CallFunction(Stack, Result, Override->OverrideFunction);
	}else{
		if(IsEvent)
			(Self->*Function->Func)(Stack, Result);
		else
			Self->CallFunction(Stack, Result, Function);
	}

	Override->CurrentSelf = NULL;

	void* Temp = ScriptFunctionHook;
	appMemcpy(&Function->Func, &Temp, sizeof(Temp));
	Function->FunctionFlags |= FUNC_Native;
}

void UFunctionOverride::execInit(FFrame& Stack, void* Result){
	P_GET_OBJECT(UObject, InTargetObject);
	P_GET_NAME(TargetFuncName);
	P_GET_OBJECT(UObject, InOverrideObject);
	P_GET_NAME(OverrideFuncName);
	P_FINISH;

	Deinit();

	UBOOL OverrideForAllObjects = InTargetObject->IsA(UClass::StaticClass());

	TargetObject = OverrideForAllObjects ? NULL : InTargetObject;
	TargetFunction = (OverrideForAllObjects ? static_cast<UClass*>(InTargetObject)->GetDefaultObject() : InTargetObject)->FindFunctionChecked(TargetFuncName);
	OverrideObject = InOverrideObject;
	OverrideFunction = OverrideObject->FindFunctionChecked(OverrideFuncName);

	if(TargetFunction->iNative != 0){ // TODO: Allow this in the future
		appErrorf("Cannot override native final function '%s' in '%s'",
				  *TargetFunction->FriendlyName,
				  OverrideForAllObjects ? InTargetObject->GetName() : InTargetObject->GetClass()->GetName());
	}

	OriginalFunctionFlags = TargetFunction->FunctionFlags;
	TargetFunction->FunctionFlags |= FUNC_Native;
	OriginalNative = TargetFunction->Func;
	void* Temp = ScriptFunctionHook;
	appMemcpy(&TargetFunction->Func, &Temp, sizeof(Temp));

	FunctionOverrides[TargetFunction] = this;
}

void UFunctionOverride::execDeinit(FFrame& Stack, void* Result){
	P_FINISH;
	Deinit();
}

void UFunctionOverride::Destroy(){
	Deinit();
	Super::Destroy();
}

void UFunctionOverride::Deinit(){
	if(TargetFunction && FunctionOverrides.Find(TargetFunction) && FunctionOverrides[TargetFunction] == this){
		TargetFunction->FunctionFlags = OriginalFunctionFlags;
		TargetFunction->Func = OriginalNative;
		FunctionOverrides.Remove(TargetFunction);
	}

	TargetObject = NULL;
	TargetFunction = NULL;
	OverrideObject = NULL;
	OverrideFunction = NULL;
}
