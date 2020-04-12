/*=============================================================================
	UnStack.h: UnrealScript execution stack definition.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

class UStruct;

/*-----------------------------------------------------------------------------
	Constants & types.
-----------------------------------------------------------------------------*/

// Sizes.
enum{ MAX_STRING_CONST_SIZE = 1024               };
enum{ MAX_CONST_SIZE        = 16 * sizeof(TCHAR) };
enum{ MAX_FUNC_PARMS        = 16                 };

// UnrealScript intrinsic return value declaration.
#define RESULT_DECL void* const Result

//
// guardexec mechanism for script debugging.
//
#define unguardexecSlow unguardfSlow(("(%s @ %s : %04X)", Stack.Object->GetFullName(), Stack.Node->GetFullName(), Stack.Code - &Stack.Node->Script[0]))
#define unguardexec     unguardf    (("(%s @ %s : %04X)", Stack.Object->GetFullName(), Stack.Node->GetFullName(), Stack.Code - &Stack.Node->Script[0]))

/*
 * State flags.
 */
enum EStateFlags{
	STATE_Editable      = 0x00000001, // State should be user-selectable in UnrealEd.
	STATE_Auto          = 0x00000002, // State is automatic (the default state).
	STATE_Simulated     = 0x00000004, // State executes on client side.
};

/*
 * Function flags.
 */
enum EFunctionFlags{
	FUNC_Final          = 0x00000001, // Function is final (prebindable, non-overridable function).
	FUNC_Defined        = 0x00000002, // Function has been defined (not just declared).
	FUNC_Iterator       = 0x00000004, // Function is an iterator.
	FUNC_Latent         = 0x00000008, // Function is a latent state function.
	FUNC_PreOperator    = 0x00000010, // Unary operator is a prefix operator.
	FUNC_Singular       = 0x00000020, // Function cannot be reentered.
	FUNC_Net            = 0x00000040, // Function is network-replicated.
	FUNC_NetReliable    = 0x00000080, // Function should be sent reliably on the network.
	FUNC_Simulated      = 0x00000100, // Function executed on the client side.
	FUNC_Exec           = 0x00000200, // Executable from command line.
	FUNC_Native         = 0x00000400, // Native function.
	FUNC_Event          = 0x00000800, // Event function.
	FUNC_Operator       = 0x00001000, // Operator function.
	FUNC_Static         = 0x00002000, // Static function.
	FUNC_NoExport       = 0x00004000, // Don't export intrinsic function to C++.
	FUNC_Const          = 0x00008000, // Function doesn't modify this object.
	FUNC_Invariant      = 0x00010000, // Return value is purely dependent on parameters; no state dependencies or internal state changes.
	FUNC_Public         = 0x00020000, // Function is accessible in all classes (if overridden, parameters much remain unchanged).
	FUNC_Private        = 0x00040000, // Function is accessible only in the class it is defined in (cannot be overriden, but function name may be reused in subclasses.  IOW: if overridden, parameters don't need to match, and Super.Func() cannot be accessed since it's private.)
	FUNC_Protected      = 0x00080000, // Function is accessible only in the class it is defined in and subclasses (if overridden, parameters much remain unchanged).
	FUNC_Delegate       = 0x00100000, // Function is actually a delegate.

	// Combinations of flags.
	FUNC_FuncInherit        = FUNC_Exec | FUNC_Event,
	FUNC_FuncOverrideMatch	= FUNC_Exec | FUNC_Final | FUNC_Latent | FUNC_PreOperator | FUNC_Iterator | FUNC_Static | FUNC_Public | FUNC_Protected,
	FUNC_NetFuncFlags       = FUNC_Net | FUNC_NetReliable,
};


enum ERuntimeUCFlags{
	RUC_ArrayLengthSet      = 0x01,	// Setting the length of an array, not an element of the array
};

/*
 * Evaluatable expression item types.
 */
enum EExprToken{
	// Variable references.
	EX_LocalVariable        = 0x00, // A local variable.
	EX_InstanceVariable     = 0x01, // An object variable.
	EX_DefaultVariable      = 0x02, // Default variable for a concrete object.

	// Tokens.
	EX_Return               = 0x04, // Return from function.
	EX_Switch               = 0x05, // Switch.
	EX_Jump                 = 0x06, // Goto a local address in code.
	EX_JumpIfNot            = 0x07, // Goto if not expression.
	EX_Stop                 = 0x08, // Stop executing state code.
	EX_Assert               = 0x09, // Assertion.
	EX_Case                 = 0x0A, // Case.
	EX_Nothing              = 0x0B, // No operation.
	EX_LabelTable           = 0x0C, // Table of labels.
	EX_GotoLabel            = 0x0D, // Goto a label.
	EX_EatString            = 0x0E, // Ignore a dynamic string.
	EX_Let                  = 0x0F, // Assign an arbitrary size value to a variable.
	EX_DynArrayElement      = 0x10, // Dynamic array element.!!
	EX_New                  = 0x11, // New object allocation.
	EX_ClassContext         = 0x12, // Class default metaobject context.
	EX_MetaCast             = 0x13, // Metaclass cast.
	EX_LetBool              = 0x14, // Let boolean variable.
	EX_LineNumber           = 0x15, // Set current source code line number in stack frame.
	EX_EndFunctionParms     = 0x16, // End of function call parameters.
	EX_Self                 = 0x17, // Self object.
	EX_Skip                 = 0x18, // Skippable expression.
	EX_Context              = 0x19, // Call a function through an object context.
	EX_ArrayElement         = 0x1A, // Array element.
	EX_VirtualFunction      = 0x1B, // A function call with parameters.
	EX_FinalFunction        = 0x1C, // A prebound function call with parameters.
	EX_IntConst             = 0x1D, // Int constant.
	EX_FloatConst           = 0x1E, // Floating point constant.
	EX_StringConst          = 0x1F, // String constant.
	EX_ObjectConst          = 0x20, // An object constant.
	EX_NameConst            = 0x21, // A name constant.
	EX_RotationConst        = 0x22, // A rotation constant.
	EX_VectorConst          = 0x23, // A vector constant.
	EX_ByteConst            = 0x24, // A byte constant.
	EX_IntZero              = 0x25, // Zero.
	EX_IntOne               = 0x26, // One.
	EX_True                 = 0x27, // Bool True.
	EX_False                = 0x28, // Bool False.
	EX_NativeParm           = 0x29, // Native function parameter offset.
	EX_NoObject             = 0x2A, // NoObject.
	EX_IntConstByte         = 0x2C, // Int constant that requires 1 byte.
	EX_BoolVariable         = 0x2D, // A bool variable which requires a bitmask.
	EX_DynamicCast          = 0x2E, // Safe dynamic class casting.
	EX_Iterator             = 0x2F, // Begin an iterator operation.
	EX_IteratorPop          = 0x30, // Pop an iterator level.
	EX_IteratorNext         = 0x31, // Go to next iteration.
	EX_StructCmpEq          = 0x32, // Struct binary compare-for-equal.
	EX_StructCmpNe          = 0x33, // Struct binary compare-for-unequal.
	EX_UnicodeStringConst   = 0x34, // Unicode string constant.
	EX_RangeConst           = 0x35, // A range constant.
	EX_StructMember         = 0x36, // Struct member.
	EX_DynArrayLength       = 0x37, // A dynamic array length for setting/getting
	EX_GlobalFunction       = 0x38, // Call non-state version of a function.
	EX_PrimitiveCast        = 0x39, // A casting operator for primitives which reads the type as the subsequent byte
	EX_DynArrayInsert       = 0x40, // Inserts into a dynamic array
	EX_DynArrayRemove       = 0x41, // Removes from a dynamic array
	EX_DebugInfo            = 0x42, // Debug information
	EX_DelegateFunction     = 0x43, // Call to a delegate function
	EX_DelegateProperty     = 0x44, // Delegate expression
	EX_LetDelegate          = 0x45, // Assignment to a delegate

	// Natives.
	EX_ExtendedNative       = 0x60,
	EX_FirstNative          = 0x70,
	EX_Max                  = 0x1000,
};


enum ECastToken{
	CST_RotatorToVector     = 0x39,
	CST_ByteToInt           = 0x3A,
	CST_ByteToBool          = 0x3B,
	CST_ByteToFloat         = 0x3C,
	CST_IntToByte           = 0x3D,
	CST_IntToBool           = 0x3E,
	CST_IntToFloat          = 0x3F,
	CST_BoolToByte          = 0x40,
	CST_BoolToInt           = 0x41,
	CST_BoolToFloat         = 0x42,
	CST_FloatToByte         = 0x43,
	CST_FloatToInt          = 0x44,
	CST_FloatToBool         = 0x45,
	//
	CST_ObjectToBool        = 0x47,
	CST_NameToBool          = 0x48,
	CST_StringToByte        = 0x49,
	CST_StringToInt         = 0x4A,
	CST_StringToBool        = 0x4B,
	CST_StringToFloat       = 0x4C,
	CST_StringToVector      = 0x4D,
	CST_StringToRotator     = 0x4E,
	CST_VectorToBool        = 0x4F,
	CST_VectorToRotator     = 0x50,
	CST_RotatorToBool       = 0x51,
	CST_ByteToString        = 0x52,
	CST_IntToString         = 0x53,
	CST_BoolToString        = 0x54,
	CST_FloatToString       = 0x55,
	CST_ObjectToString      = 0x56,
	CST_NameToString        = 0x57,
	CST_VectorToString      = 0x58,
	CST_RotatorToString     = 0x59,
	CST_Max                 = 0xFF,
};


/*
 * Latent functions.
 */
enum EPollSlowFuncs{
	EPOLL_Sleep               = 384,
	EPOLL_FinishAnim          = 385,
	EPOLL_FinishInterpolation = 302,
};

/*-----------------------------------------------------------------------------
	Execution stack helpers.
-----------------------------------------------------------------------------*/

/*
 * Information about script execution at one stack level.
 */
struct CORE_API FFrame : public FOutputDevice{
	// Variables.
	UStruct* Node;
	UObject* Object;
	BYTE*    Code;
	BYTE*    Locals;
	INT      LineNum;

	// Constructors.
	FFrame(UObject* InObject);
	FFrame(UObject* InObject, UStruct* InNode, INT CodeOffset, void* InLocals);

	// FOutputDevice interface.
	virtual void Serialize(const TCHAR* V, EName Event);

	// Functions.
	void Step(UObject* Context, RESULT_DECL);
	INT ReadInt();
	UObject* ReadObject();
	FLOAT ReadFloat();
	INT ReadWord();
	FName ReadName();
};

/*
 * Information about script execution at the main stack level.
 *
 * This part of an actor's script state is saveable at any time.
 */
struct CORE_API FStateFrame : public FFrame{
	// Variables.
	FFrame* CurrentFrame;
	UState* StateNode;
	QWORD   ProbeMask;
	INT     LatentAction;

	// Constructors.
	FStateFrame(UObject* InObject);

	// Functions.
	const TCHAR* Describe();
};

/*-----------------------------------------------------------------------------
	Script execution helpers.
-----------------------------------------------------------------------------*/

/*
 * Base class for UnrealScript iterator lists.
 */
struct FIteratorList{
	FIteratorList* Next;

	FIteratorList(){}
	FIteratorList(FIteratorList* InNext) : Next(InNext){}
	FIteratorList* GetNext(){ return Next; }
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
