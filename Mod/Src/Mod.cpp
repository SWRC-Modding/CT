#include "Mod.h"

/*
 * FunctionOverride
 * - Allows hooking UnrealScript functions of a specific object or all objects of a class.
 * - This works by replacing UFunction::Func with a custom function and adding the FUNC_Native flag to make sure it is always called directly.
 * - The hook then performs mapping between the UFunction and the override and calls it if the Self pointer is the target object.
 */

static TMap<UFunction*, UFunctionOverride*> FunctionOverrides;

static void __fastcall ScriptFunctionHook(UObject* Self, DWORD, FFrame& Stack, void* Result)
{
	/*
	 * The stack doesn't contain any information about the called function at this point.
	 * We only know that the last four bytes at the top are either a UFunction* or an FName so we need to check both.
	 */
	UFunction* Function;
	FName      FunctionName;

	appMemcpy(&Function, Stack.Code - sizeof(void*), sizeof(void*));

	if(!FunctionOverrides.Find(Function))
	{
		Function = NULL;

		appMemcpy(&FunctionName, Stack.Code - sizeof(FName), sizeof(FName));

		if(!((reinterpret_cast<DWORD>(FunctionName.GetEntry()) & 0xFFFF0000) == 0 || // Seems to be enough to check for a valid name
		     IsBadReadPtr(FunctionName.GetEntry(), sizeof(FNameEntry) - NAME_SIZE))) // Using IsBadReadPtr as a backup check just in case
		     {
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

	if(Self == Override->TargetObject || !Override->TargetObject)
	{
		if(IsEvent)
			Override->OverrideObject->ProcessEvent(Override->OverrideFunction, Stack.Locals);
		else
			Override->OverrideObject->CallFunction(Stack, Result, Override->OverrideFunction);
	}
	else
	{ // Function was not called on the target object so just call the original one
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

struct FNativeFinalOverride{
	Native     NativeFunc;
	UFunction* ScriptFunc;

	FNativeFinalOverride(Native InNativeFunc, UFunction* InScriptFunc)
		: NativeFunc(InNativeFunc),
		  ScriptFunc(InScriptFunc)
	{
	}
};

static TMap<INT, FNativeFinalOverride> NativeFinalFunctionOverrides;

static void __fastcall NativeFinalFunctionHook(UObject* Self, DWORD, FFrame& Stack, void* Result)
{
	INT Index = *(Stack.Code - 1);

	if(*reinterpret_cast<void**>(&GNatives[Index]) != NativeFinalFunctionHook)
	{
		Index = *(_WORD*)(Stack.Code - 1);

		if(*reinterpret_cast<void**>(&GNatives[Index]) == NativeFinalFunctionHook)
			++Stack.Code;
		else
			appErrorf("NativeFinalFunctionHook called with invalid function index");
	}

	const FNativeFinalOverride& Override = NativeFinalFunctionOverrides[Index];
	GNatives[Index] = Override.NativeFunc;
	Self->CallFunction(Stack, Result, Override.ScriptFunc);
	void* Temp = NativeFinalFunctionHook;
	appMemcpy(&GNatives[Index], &Temp, sizeof(Temp));
}

void UFunctionOverride::execInit(FFrame& Stack, void* Result)
{
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

	if(TargetFunction->iNative != 0)
	{
		NativeFinalFunctionOverrides[TargetFunction->iNative] = FNativeFinalOverride(GNatives[TargetFunction->iNative], OverrideFunction);
		void* Temp = NativeFinalFunctionHook;
		appMemcpy(&GNatives[TargetFunction->iNative], &Temp, sizeof(Temp));
	}
	else
	{
		OriginalFunctionFlags = TargetFunction->FunctionFlags;
		TargetFunction->FunctionFlags |= FUNC_Native;
		OriginalNative = TargetFunction->Func;
		void* Temp = ScriptFunctionHook;
		appMemcpy(&TargetFunction->Func, &Temp, sizeof(Temp));
		FunctionOverrides[TargetFunction] = this;
	}
}

void UFunctionOverride::execDeinit(FFrame& Stack, void* Result)
{
	P_FINISH;
	Deinit();
}

void UFunctionOverride::Destroy()
{
	Deinit();
	Super::Destroy();
}

void UFunctionOverride::Deinit()
{
	if(TargetFunction && TargetFunction->iNative)
	{
		GNatives[TargetFunction->iNative] = NativeFinalFunctionOverrides[TargetFunction->iNative].NativeFunc;
		NativeFinalFunctionOverrides.Remove(TargetFunction->iNative);
	}
	else if(TargetFunction && FunctionOverrides.Find(TargetFunction) && FunctionOverrides[TargetFunction] == this)
	{
		TargetFunction->FunctionFlags = OriginalFunctionFlags;
		TargetFunction->Func = OriginalNative;
		FunctionOverrides.Remove(TargetFunction);
	}

	TargetObject = NULL;
	TargetFunction = NULL;
	OverrideObject = NULL;
	OverrideFunction = NULL;
}
