/*=============================================================================
	UnTemplate.h: Unreal templates.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Type information.
-----------------------------------------------------------------------------*/

/*
 * Type information for initialization.
 */
template<typename T>
struct TTypeInfoBase{
	typedef const T& ConstInitType;

	static UBOOL NeedsDestructor(){ return 1; }
	static UBOOL DefinitelyNeedsDestructor(){ return 0; }
	static const T& ToInit(const T& In){ return In; }
};

template<typename T>
struct TTypeInfo : public TTypeInfoBase<T>{};

template<typename T>
struct TTypeInfo<T*> : public TTypeInfoBase<T*>{
	static UBOOL NeedsDestructor(){ return 0; }
};

#define PRIMITIVE_TYPEINFO(type) \
	template<> \
	struct TTypeInfo<type> : public TTypeInfoBase<type>{ \
		static UBOOL NeedsDestructor(){ return 0; } \
	};

PRIMITIVE_TYPEINFO(BYTE)
PRIMITIVE_TYPEINFO(SBYTE)
PRIMITIVE_TYPEINFO(TCHAR)
PRIMITIVE_TYPEINFO(INT)
PRIMITIVE_TYPEINFO(DWORD)
PRIMITIVE_TYPEINFO(_WORD)
PRIMITIVE_TYPEINFO(SWORD)
PRIMITIVE_TYPEINFO(QWORD)
PRIMITIVE_TYPEINFO(SQWORD)
PRIMITIVE_TYPEINFO(FName)

/*-----------------------------------------------------------------------------
	Standard templates.
-----------------------------------------------------------------------------*/

template<typename T>
inline T Abs(const T A){ return (A >= static_cast<T>(0)) ? A : -A; }

template<typename T>
inline T Sgn(const T A){ return (A > 0) ? 1 : ((A < 0) ? -1 : 0); }

template<typename T>
inline T Max(const T A, const T B){ return (A >= B) ? A : B; }

template<typename T>
inline T Min(const T A, const T B){ return (A <= B) ? A : B; }

template<typename T>
inline T Square(const T A){ return A * A; }

template<typename T>
inline T Clamp(const T X, const T Min, const T Max){ return X < Min ? Min : X < Max ? X : Max; }

template<typename T>
inline T Align(const T Ptr, INT Alignment){ return (T)(((DWORD)Ptr + Alignment - 1) & ~(Alignment - 1)); }

template<typename T>
inline void Exchange(T& A, T& B){
	const T Temp = A;
	A = B;
	B = Temp;
}

template<typename T>
T Lerp(T& A, T& B, FLOAT Alpha){ return A + Alpha * (B - A); }

inline DWORD GetTypeHash(const BYTE A){ return A; }
inline DWORD GetTypeHash(const SBYTE A){ return A; }
inline DWORD GetTypeHash(const _WORD A){ return A; }
inline DWORD GetTypeHash(const SWORD A){ return A; }
inline DWORD GetTypeHash(const INT A){ return A; }
inline DWORD GetTypeHash(const DWORD A){ return A; }
inline DWORD GetTypeHash(const QWORD A){ return (DWORD)A + ((DWORD)(A >> 32) * 23); }
inline DWORD GetTypeHash(const SQWORD A){ return (DWORD)A + ((DWORD)(A >> 32) * 23); }
inline DWORD GetTypeHash(const TCHAR* S){ return appStrihash(S); }

template<typename T>
DWORD GetTypeHash(const T* P){ return (DWORD)P; }

#define ExchangeB(A,B) do{UBOOL T=A; A=B; B=T;}while(false);

/*----------------------------------------------------------------------------
	Standard macros.
----------------------------------------------------------------------------*/

// Number of elements in an array.
template <typename T, INT N>
char(&ArrayCountHelper(T (&array)[N]))[N];

#define ARRAY_COUNT(array) \
	(sizeof(ArrayCountHelper(array)))

// Offset of a struct member.
#define STRUCT_OFFSET(struc, member) \
	((INT)&((struc*)NULL)->member)

#define STR(x) #x
#define STRINGIFY(x) STR(x)

/*-----------------------------------------------------------------------------
	Dynamic array template.
-----------------------------------------------------------------------------*/

/*
 * Former dynamic array base.
 *
 * All functionality has been moved to TArray.
 * This class only exists for FTransactionBase::SaveArray
 */
class FArray{
protected:
	void* Data;
	INT ArrayNum     : 29;
	INT bIsReference : 1; // Array doesn't own the data it points to and thus is not allowed to free it
	INT bIsTemporary : 1; // This is a temporary object (e.g. returned from a function)
	INT bNoShrink    : 1; // Don't shrink allocation when elements are removed

	FArray(bool NoShrink = false) : Data(NULL), ArrayNum(0), bIsReference(0), bIsTemporary(0), bNoShrink(NoShrink){}
	FArray(void* Src, INT Count) : Data(Src), ArrayNum(Count), bIsReference(1), bIsTemporary(0), bNoShrink(0){}
	FArray(ENoInit){}
};

/*
 * Template dynamic array.
 * Should only be used with POD types.
 */
template<typename T>
class TArray : public FArray{
public:
	TArray() : FArray(){}
	TArray(T* Src, INT Count) : FArray(Src, Count){}
	TArray(ENoInit) : FArray(E_NoInit){}

	TArray(INT Size, bool NoShrink = false) : FArray(NoShrink){
		Set(Size);
	}

	TArray(const TArray<T>& other, bool NoShrink = false){
		bNoShrink = NoShrink;
		*this = other;
	}

	~TArray(){
		if(IsAllocated()){
			Deinit(0, ArrayNum);
			appFree(Data);
		}

		Data = NULL;
		ArrayNum = 0;
	}

	TArray<T>& operator=(const TArray<T>& Other){
		if(this != &Other){
			if(Other.bIsReference || Other.bIsTemporary){
				Transfer(const_cast<TArray<T>&>(Other));
			}else{
				Empty(Other.ArrayNum);

				// It is assumed that a type is not trivially copyable if it needs a destructor which should be true in pretty much any case
				if(TTypeInfo<T>::NeedsDestructor()){
					for(INT i = 0; i < Other.ArrayNum; ++i)
						new(*this)T(Other[i]);
				}else{
					appMemcpy(Data, Other.Data, Other.ArrayNum * sizeof(T));
					ArrayNum = Other.ArrayNum;
				}
			}
		}

		return *this;
	}

	TArray<T> operator+(const TArray<T>& Other){
		TArray<T> Result(*this);

		Result.Reserve(Result.Num() + Other.Num());

		if(TTypeInfo<T>::NeedsDestructor()){
			for(INT i = 0; i < Other.ArrayNum; ++i)
				new(Result)T(Other[i]);
		}else{
			appMemcpy(static_cast<T*>(Result.Data) + Result.Num(), Other.Data, Other.ArrayNum * sizeof(T));
			Result.ArrayNum += Other.ArrayNum;
		}

		Result.bIsTemporary = 1;

		return Result;
	}

	TArray<T>& operator+=(const TArray<T>& Other){
		Reserve(Num() + Other.Num());

		if(TTypeInfo<T>::NeedsDestructor()){
			for(INT i = 0; i < Other.ArrayNum; ++i)
				new(*this)T(Other[i]);
		}else{
			appMemcpy(GetData() + Num(), Other.Data, Other.ArrayNum * sizeof(T));
			ArrayNum += Other.ArrayNum;
		}

		return *this;
	}

	T& operator[](INT Index){
		checkSlow(IsValidIndex(Index));
		return GetData()[Index];
	}

	const T& operator[](INT Index) const{
		checkSlow(IsValidIndex(Index));
		return GetData()[Index];
	}

	void Transfer(TArray<T>& Src){
		Realloc(0, 0);

		Data = Src.Data;
		ArrayNum = Src.ArrayNum;
		bIsReference = Src.bIsReference;
		bNoShrink = Src.bNoShrink;

		Src.Unreference();
	}

	void Set(T* Src, INT Count){
		Data = Src;
		ArrayNum = Count;
	}

	void Set(INT NewSize, INT Slack){
		INT OldNum = Num();

		if(NewSize >= Num()){
			Realloc(NewSize, Slack);

			if(Num() > OldNum)
				appMemzero(static_cast<BYTE*>(Data) + OldNum, (Num() - OldNum) * sizeof(T));
		}else{
			Remove(NewSize, Num() - NewSize);
		}
	}

	void Set(INT NewSize){ 	Set(NewSize, NewSize); 	}
	T* GetData(){ 	return static_cast<T*>(Data); 	}
	const T* GetData() const{ 	return static_cast<T*>(Data); 	}
	bool IsValidIndex(INT Index) const{ 	return Index >= 0 && Index < Num(); 	}
	INT Num() const{ 	return ArrayNum; 	}
	INT Size() const{ 	return Num(); 	}
	bool IsAllocated() const{ 	return Data != NULL && !bIsReference; 	}
	T& Last(INT c = 0){ 	return (*this)[Num() - c - 1]; 	}
	const T& Last(INT c = 0) const{ 	return (*this)[Num() - c - 1]; 	}

	bool FindItem(const T& Item, INT& Index) const{
		for(Index = 0; Index < Num(); ++Index){
			if((*this)[Index] == Item)
				return true;
		}

		return false;
	}

	INT FindItemIndex(const T& Item) const{
		INT Index;

		if(FindItem(Item, Index))
			return Index;

		return INDEX_NONE;
	}

	void SetNoShrink(bool NoShrink){ 	bNoShrink = NoShrink; 	}
	void CountBytes(FArchive& Ar){ 	Ar.CountBytes(Data, Num() * sizeof(T)); 	}
	INT GetMaxSize() const{ 	return Capacity(); 	}

	void Serialize(FArchive& Ar){
		guard(MyTArray::Serialize);
		CountBytes(Ar);

		INT TempNum = Num();

		if(sizeof(T) == 1){
			// Serialize simple bytes which require no construction or destruction.
			if(Ar.IsLoading()){
				Ar << AR_INDEX(TempNum);
				Realloc(TempNum, 0);
			}else{
				Ar << AR_INDEX(TempNum);
			}

			Ar.Serialize(&(*this)[0], Num());
		}else if(Ar.IsLoading()){
			// Load array.

			Ar << AR_INDEX(TempNum);

			Empty(TempNum);

			for(INT i = 0; i < TempNum; ++i)
				Ar << *new(*this)T;
		}else{
			// Save array.
			Ar << AR_INDEX(TempNum);

			for(INT i = 0; i < Num(); ++i)
				Ar << (*this)[i];
		}

		unguard;
	}

	void Reserve(INT Size){
		INT CurrentSize = IsAllocated() ? 0 : Capacity();

		Realloc(Num(), CurrentSize < Size ? Size : CurrentSize);
	}

	void Insert(INT Index, INT Count = 1, bool bInit = true){
		Realloc(Num() + Count, 0);

		appMemmove(
			static_cast<BYTE*>(Data) + (Index + Count) * sizeof(T),
			static_cast<BYTE*>(Data) + Index * sizeof(T),
			(Num() - Index - Count) * sizeof(T)
		);

		if(bInit)
			Init(Index, Count);
	}

	INT Add(INT Count, bool bInit = true){
		Realloc(Num() + Count, 0);

		if(bInit)
			Init(Num() - Count, Count);

		return Num() - Count;
	}

	void InsertZeroed(INT Index, INT Count){
		Insert(Index, Count);
		appMemzero(static_cast<BYTE*>(Data) + Index * sizeof(T), Count * sizeof(T));
	}

	INT AddZeroed(INT Count = 1){
		INT Index = Add(Count);

		appMemzero(static_cast<BYTE*>(Data) + Index * sizeof(T), Count * sizeof(T));

		return Index;
	}

	void Shrink(){
		Realloc(Num(), Num());
	}

	void Empty(INT Slack = 0){
		Deinit(0, ArrayNum);
		Realloc(0, Slack);
	}

	void Remove(INT Index, INT Count = 1){
		Deinit(Index, Count);

		if(Count){
			appMemmove(
				static_cast<BYTE*>(Data) + Index * sizeof(T),
				static_cast<BYTE*>(Data) + (Index + Count) * sizeof(T),
				(Num() - Index - Count) * sizeof(T)
			);

			Realloc(Num() - Count, 0);
		}
	}

	INT RemoveItem(const T& Item){
		INT OldNum = Num();
		INT Index = FindItemIndex(Item);

		if(Index != INDEX_NONE)
			Remove(Index);

		return OldNum - Num();
	}

	T Pop(){
		T Result = (*this)[Num() - 1];

		Remove(Num() - 1);

		return Result;
	}

	INT AddItem(const T& Item){
		new(*this) T(Item);

		return Num() - 1;
	}

	INT AddUniqueItem(const T& Item){
		INT Index = FindItemIndex(Item);

		return Index != INDEX_NONE ? Index : AddItem(Item);
	}

	TArray<T> Segment(INT Index, INT Count){
		TArray<T> Result;

		Result.Data = &(*this)[Index];
		Result.ArrayNum = Count;

		Result.Unreference();

		return Result;
	}

	// Iterator
	class TIterator{
	public:
		TIterator(TArray<T>& InArray) : Array(InArray), Index(-1) { ++*this;           }
		void operator++()     { ++Index;                                               }
		void RemoveCurrent()  { Array.Remove(Index--);                                 }
		INT GetIndex()   const{ return Index;                                          }
		operator bool()  const{ return Index < Array.Num();                            }
		T& operator*()   const{ return Array[Index];                                   }
		T* operator->()  const{ return &Array[Index];                                  }
		T& GetCurrent()  const{ return Array[Index];                                   }
		T& GetPrev()     const{ return Array[Index ? Index - 1 : Array.Num() - 1 ];    }
		T& GetNext()     const{ return Array[Index < Array.Num() - 1 ? Index + 1 : 0]; }

	private:
		TArray<T>& Array;
		INT Index;
	};

protected:
	INT Capacity() const{
		if(IsAllocated())
			return GMalloc->GetAllocationSize(Data) / sizeof(T);

		return 0;
	}

	void Unreference() const{ // Unreference seems like a bad name choice but we want to use the original naming...
		const_cast<TArray<T>*>(this)->bIsReference = 1; // Need to do a const_cast because the function is const in the original code as well
	}

	void Init(INT Index, INT Count){
		// This assumes every type used with TArray can be zero initialized
		appMemzero(static_cast<BYTE*>(Data) + Index * sizeof(T), Count * sizeof(T));
	}

	void Deinit(INT Index, INT Count){
		check(ArrayNum == 0 || IsAllocated());

		if(TTypeInfo<T>::NeedsDestructor()){
			for(INT i = Index; i < Index + Count; ++i)
				(&(*this)[i])->~T();
		}
	}

	void Realloc(INT NewSize, INT Slack){
		guard(TArray::Realloc);

		if(IsAllocated()){
			if(NewSize <= ArrayNum){
				if(!bNoShrink && ArrayNum >= 0 && Align(NewSize * sizeof(T), 32) < Align(Num() * sizeof(T), 32))
					Data = appRealloc(Data, NewSize * sizeof(T), Max(NewSize, Slack) * sizeof(T));
			}else{
				Data = appRealloc(Data, NewSize * sizeof(T), Slack * sizeof(T));
			}
		}else{
			if(NewSize > 0 || Slack > 0){
				void* Temp = appMalloc(Max(NewSize, Slack) * sizeof(T));

				if(Data)
					appMemcpy(Temp, Data, Min(ArrayNum, NewSize) * sizeof(T));

				Data = Temp;
			}else{
				Data = NULL;
			}

			bIsReference = 0; // To make sure the memory isn't leaked.
							   // This doesn't exist in the original code which means that either
							   // the developers made a mistake or I'm missing something else about
							   // how these flags are used...
		}

		ArrayNum = NewSize;

		unguard;
	}

	friend FArchive& operator<<(FArchive& Ar, TArray<T>& Array){
		Array.Serialize(Ar);

		return Ar;
	}
};

template<typename T>
class TArrayNoInit : public TArray<T>{
public:
	TArrayNoInit() : TArray<T>(E_NoInit){}

	TArrayNoInit& operator=(const TArrayNoInit& Other){
		TArray<T>::operator=(Other);
		return *this;
	}
};

//
// Array operator news.
//
template<typename T>
void* operator new(size_t Size, TArray<T>& Array){
	INT Index = Array.Add(1);

	return &Array[Index];
}

template<typename T>
void* operator new(size_t Size, TArray<T>& Array, INT Index){
	Array.Insert(Index);

	return &Array[Index];
}

//
// Array exchanger.
//
template<typename T>
inline void ExchangeArray(TArray<T>& A, TArray<T>& B){
	guardSlow(ExchangeTArray);
	appMemswap(&A, &B, sizeof(FArray));
	unguardSlow;
}

/*-----------------------------------------------------------------------------
	Transactional array.
-----------------------------------------------------------------------------*/

template<typename T>
class TTransArray : public TArray<T>{
public:
	// Constructors.
	TTransArray(UObject* InOwner, INT InNum = 0) : TArray<T>(InNum),
												   Owner(InOwner){}

	TTransArray(UObject* InOwner, const TArray<T>& Other) : TArray<T>(Other),
															Owner(InOwner){}

	TTransArray& operator=(const TTransArray& Other){
		operator=((const TArray<T>&)Other);
		return *this;
	}

	// Add, Insert, Remove, Empty interface.

	INT Add(INT Count = 1){
		INT Index = TArray<T>::Add(Count);
		if(GUndo)
			GUndo->SaveArray(Owner, this, Index, Count, 1, sizeof(T), SerializeItem, DestructItem);
		return Index;
	}

	void Insert(INT Index, INT Count = 1){
		TArray<T>::Insert(Index, Count);

		if(GUndo)
			GUndo->SaveArray(Owner, this, Index, Count, 1, sizeof(T), SerializeItem, DestructItem);
	}

	void Remove(INT Index, INT Count = 1){
		if(GUndo)
			GUndo->SaveArray(Owner, this, Index, Count, -1, sizeof(T), SerializeItem, DestructItem);

		TArray<T>::Remove(Index, Count);
	}

	void Empty(INT Slack = 0){
		if(GUndo)
			GUndo->SaveArray(Owner, this, 0, ArrayNum, -1, sizeof(T), SerializeItem, DestructItem);

		TArray<T>::Empty(Slack);
	}

	// Functions dependent on Add, Remove.
	TTransArray& operator=(const TArray<T>& Other){
		if(this != &Other){
			Empty(Other.Num());

			for(INT i = 0; i<Other.Num(); i++)
				new(*this)T(Other(i));
		}

		return *this;
	}

	INT AddItem(const T& Item){
		new(*this) T(Item);
		return Num() - 1;
	}

	INT AddZeroed(INT n = 1){
		INT Index = Add(n);
		appMemzero(&(*this)[Index], n*sizeof(T));
		return Index;
	}

	INT AddUniqueItem(const T& Item){
		for(INT Index = 0; Index<ArrayNum; Index++)
			if((*this)[Index]==Item)
				return Index;
		return AddItem(Item);
	}

	INT RemoveItem(const T& Item){
		INT OriginalNum=ArrayNum;
		for(INT Index = 0; Index<ArrayNum; Index++)
			if((*this)[Index]==Item)
				Remove(Index--);
		return OriginalNum - ArrayNum;
	}

	// TTransArray interface.
	UObject* GetOwner(){
		return Owner;
	}

	void ModifyItem(INT Index){
		if(GUndo)
			GUndo->SaveArray(Owner, this, Index, 1, 0, sizeof(T), SerializeItem, DestructItem);
	}

	void ModifyAllItems(){
		if(GUndo)
			GUndo->SaveArray(Owner, this, 0, Num(), 0, sizeof(T), SerializeItem, DestructItem);
	}

	friend FArchive& operator<<(FArchive& Ar, TTransArray& A){
		if(!Ar.IsTrans())
			Ar << (TArray<T>&)A;
		return Ar;
	}
protected:
	static void SerializeItem(FArchive& Ar, void* TPtr){
		Ar << *(T*)TPtr;
	}

	static void DestructItem(void* TPtr){
		((T*)TPtr)->~T();
	}

	UObject* Owner;

private:

	// Disallow the copy constructor.
	TTransArray(const TArray<T>& Other){}
};

/*-----------------------------------------------------------------------------
	Lazy loading.
-----------------------------------------------------------------------------*/

/*
 * Lazy loader base class.
 */
class FLazyLoader{
	friend class ULinkerLoad;
protected:
	FArchive*	 SavedAr;
	INT          SavedPos;
public:
	FLazyLoader() : SavedAr(NULL),
					SavedPos(0){}

	virtual void Load() = 0;
	virtual void Unload() = 0;
	/*
	 * Returns the byte offset to the payload.
	 */
	virtual DWORD GetOffset(){
		return Abs(SavedPos);
	}
};

/*
 * Lazy-loadable dynamic array.
 */
template<typename T>
class TLazyArray : public TArray<T>, public FLazyLoader{
public:
	TLazyArray(INT InNum = 0)
	: TArray<T>(InNum)
	, FLazyLoader()
	{}
	~TLazyArray(){
		if(SavedAr)
			SavedAr->DetachLazyLoader(this);
	}
	/*
	 * Returns the byte offset to the payload, skipping the array size serialized
	 * by TArray's serializer.
	 */
	virtual DWORD GetOffset(){
		// Skips array size being serialized.
		return Abs(SavedPos) + sizeof(INT);
	}

	void Load(){
		// Make sure this array is loaded.
		if(SavedPos>0){
			// Lazy load it now.
			INT PushedPos = SavedAr->Tell();
			SavedAr->Seek(SavedPos);
			*SavedAr << (TArray<T>&)*this;
			SavedPos *= -1;
			SavedAr->Seek(PushedPos);
		}
	}

	void Unload(){
		// Make sure this array is unloaded.
		if(SavedPos < 0){
			// Unload it now.
			Empty(0);
			SavedPos *= -1;
		}
	}

	void Detach(){
		if(SavedAr)
			SavedAr->DetachLazyLoader(this);
	}

	friend FArchive& operator<<(FArchive& Ar, TLazyArray& This){
		if(Ar.IsLoading()){
			INT SeekPos = 0;
			Ar << SeekPos;
			if(GUglyHackFlags & 8){
				Ar << (TArray<T>&)This;
			}else{
				Ar.AttachLazyLoader(&This);

				if(!GLazyLoad)
					This.Load();
			}

			Ar.Seek(SeekPos);
		}else if(Ar.IsSaving()){
			// If there's unloaded array data, load it.
			if(This.SavedPos > 0)
				This.Load();

			// Save out count for skipping over this data.
			INT CountPos = Ar.Tell();
			Ar << CountPos << (TArray<T>&)This;
			INT EndPos = Ar.Tell();
			Ar.Seek(CountPos);
			Ar << EndPos;
			Ar.Seek(EndPos);
		}else {
			Ar << (TArray<T>&)This;
		}

		return Ar;
	}
};

/*-----------------------------------------------------------------------------
	Dynamic strings.
-----------------------------------------------------------------------------*/

class FStringTemp;

/*
 * A dynamically sizeable string.
 */
class CORE_API FString : protected TArray<TCHAR>{
public:
	FString();
	FString(const TCHAR* In, bool IsReference = false);
	FString(ENoInit);
	FString(const FString& Other);
	FString(const FStringTemp& Other);
	~FString();

	TCHAR& operator[](INT Index);
	const TCHAR& operator[](INT Index) const;
	TArray<TCHAR>& GetCharArray();
	const TCHAR* operator*() const;
	FString& operator*=(const TCHAR* Str);
	FString& operator*=(const FString& Str);
	bool operator<=(const TCHAR* Other) const;
	bool operator<=(const FString& Other) const;
	bool operator<(const TCHAR* Other) const;
	bool operator<(const FString& Other) const;
	bool operator>=(const TCHAR* Other) const;
	bool operator>=(const FString& Other) const;
	bool operator>(const TCHAR* Other) const;
	bool operator>(const FString& Other) const;
	bool operator==(const TCHAR* Other) const;
	bool operator==(const FString& Other) const;
	bool operator!=(const TCHAR* Other) const;
	bool operator!=(const FString& Other) const;
	INT Len() const;
	INT InStr(const TCHAR* SubStr, bool Right = false) const;
	INT InStr(const FString& SubStr, bool Right = false) const;
	TCHAR* MakeCharArray();
	FStringTemp Right(INT Count) const;
	void Empty();
	void Shrink();
	FString& operator+=(const TCHAR* Str);
	FString& operator+=(const FString& Str);
	FString& operator=(const TCHAR* Other);
	FString& operator=(const FString& Other);
	FString& operator=(const FStringTemp&);
	FStringTemp Left(INT Count) const;
	FStringTemp LeftChop(INT Count) const;
	FStringTemp Mid(INT Start, INT Count = MAXINT) const;
	FStringTemp operator+(const TCHAR* Other) const;
	FStringTemp operator+(const FString& Other) const;
	FStringTemp operator*(const TCHAR* Other) const;
	FStringTemp operator*(const FString& Other) const;
	INT InStr(TCHAR c, bool Right = false) const;
	void Serialize(FArchive& Ar);
	bool Split(const FString& InS, FString* LeftS, FString* RightS, bool Right = false) const;
	FStringTemp LeftPad(INT ChCount);
	FStringTemp RightPad(INT ChCount);
	FStringTemp Caps() const;
	FStringTemp Locs() const;
	INT ParseIntoArray(const TCHAR* pchDelim, TArray<FString>* InArray);
	FStringTemp Substitute(const FString&, const FString&) const;
	FStringTemp Substitute(const FString&) const;

	static FStringTemp VARARGS Printf(const TCHAR* fmt, ...);
	static FStringTemp Chr(TCHAR c);

protected:
	FString(INT Size);
	FString(INT InCount, const TCHAR* InSrc);

	friend FArchive& operator<<(FArchive& Ar, FString& String){
		String.Serialize(Ar);

		return Ar;
	}
};

/*
 * Temporary string used to avoid unnecessary copies with temporary objects (e.g. when chaining the + operator)
 * This should only be used as a return value.
 */
class CORE_API FStringTemp : public FString{
public:
	FStringTemp(const TCHAR* In, bool IsReference = false);
	FStringTemp(INT Count);
	FStringTemp(INT Count, const TCHAR* In);
	FStringTemp(const FStringTemp& Other);
	FStringTemp(const FString& Other);
	~FStringTemp();

	FStringTemp& operator=(const FStringTemp&);
};

inline FStringTemp operator+(const TCHAR* CStr, const FString& Str){
	return FString(CStr, true) + Str;
}

struct CORE_API FStringNoInit : public FString{
public:
	FStringNoInit(const FStringNoInit&);
	FStringNoInit();
	~FStringNoInit();

	FStringNoInit& operator=(const FStringNoInit&);
	FStringNoInit& operator=(const FString&);
	FStringNoInit& operator=(const TCHAR*);
};

/*
 * String that contains the value of a configuration variable.
 * Only intended to be used with FConfigSection.
 * It has an additional bool member that is set to true when the value of the config entry has changed.
 */
class FConfigString : public FString{
public:
	FConfigString() : bDirty(false){}
	FConfigString(const TCHAR* Src) : FString(Src),
	                                  bDirty(false){}
	FConfigString(const FString& Src) : FString(Src),
	                                    bDirty(false){}
	FConfigString(const FConfigString& Src) : FString(Src),
	                                          bDirty(Src.bDirty){}
	~FConfigString(){ FString::~FString(); }

	FConfigString& operator=(const FConfigString& Other){
		FString::operator=(Other);
		bDirty = Other.bDirty;

		return *this;
	}

	FConfigString& operator=(const FString& Other){
		FString::operator=(Other);

		return *this;
	}

	FConfigString& operator=(const TCHAR* Other){
		FString::operator=(Other);

		return *this;
	}

	void SetDirty(bool InDirty){ bDirty = InDirty; }
	bool Dirty() const{ return bDirty; }

private:
	bool bDirty;
};

inline DWORD GetTypeHash(const FString& S){
	return appStrihash(*S);
}

template<>
struct TTypeInfo<FString> : public TTypeInfoBase<FString>{
	typedef const TCHAR* ConstInitType;
	static const TCHAR* ToInit(const FString& In) {return *In;}
	static UBOOL DefinitelyNeedsDestructor(){ return 0; }
};

template<>
struct TTypeInfo<FConfigString> : public TTypeInfo<FString>{};

//
// String exchanger.
//
inline void ExchangeString(FString& A, FString& B){
	guardSlow(ExchangeTArray);
	appMemswap(&A, &B, sizeof(FString));
	unguardSlow;
}

/*-----------------------------------------------------------------------------
	FFilename.
-----------------------------------------------------------------------------*/

/*
 * Utility class for quick inquiries against filenames.
 */
struct FFilename : public FString{
public:
	FFilename() : FString(){}
	FFilename(const FString& Other) : FString(Other){}
	FFilename(const TCHAR* In) : FString(In){}
	FFilename(ENoInit) : FString(E_NoInit){}

	// Returns the text following the last period.
	FStringTemp GetExtension() const{
		INT Pos = InStr(".", true);

		if(Pos != -1)
			return Mid(Pos + 1, Len() - Pos - 1);

		return "";
	}

	// Returns the base filename, minus any path information.
	FStringTemp GetCleanFilename() const{
		INT Pos = GetLastPathSeparator();

		if(Pos != -1)
			return Mid(Pos + 1, Len() - Pos - 1);

		return *this;
	}

	// Returns the same thing as GetCleanFilename, but without the extension
	FStringTemp GetBaseFilename() const{
		FString Wk = GetCleanFilename();

		INT Pos = Wk.InStr(".", true);

		if(Pos != -1)
			return Wk.Left(Pos);

		return Wk;
	}

	// Returns the path and file name without the extension
	FStringTemp GetBaseFilePath() const{
		INT Pos = InStr(".", true);

		if(Pos != -1)
			return Left(Pos);

		return *this;
	}

	// Returns the path in front of the filename
	FStringTemp GetPath() const{
		INT Pos = GetLastPathSeparator();

		if(Pos != -1)
			return Left(Pos);

		return *this;
	}

private:
	INT GetLastPathSeparator() const{ return Max(InStr("\\", true), InStr("/", true)); }
};

/*----------------------------------------------------------------------------
	Special archivers.
----------------------------------------------------------------------------*/

/*
 * String output device.
 */
class FStringOutputDevice : public FString, public FOutputDevice{
public:
	FStringOutputDevice(const TCHAR* InStr = "") : FString(InStr){}

	void Serialize(const TCHAR* Data, EName Event){
		*this += Data;
	}
};

/*----------------------------------------------------------------------------
	TMap.
----------------------------------------------------------------------------*/

/*
 * Maps unique keys to values.
 */
template<typename TK, typename TI>
class TMapBase{
protected:
	class TPair{
	public:
		INT HashNext;
		TK  Key;
		TI  Value;

		TPair(typename TTypeInfo<TK>::ConstInitType InKey, typename TTypeInfo<TI>::ConstInitType InValue) : Key(InKey),
		                                                                                                    Value(InValue){}
		TPair(){}

		friend FArchive& operator<<(FArchive& Ar, TPair& F){
			return Ar << F.Key << F.Value;
		}
	};

	TArray<TPair> Pairs;
	INT*          Hash;
	INT           HashCount;

	void Rehash(){
		Hash = static_cast<INT*>(appRealloc(Hash, HashCount * sizeof(INT), 0));

		for(INT i = 0; i < HashCount; ++i)
			Hash[i] = INDEX_NONE;

		for(INT i = 0; i < Pairs.Num(); ++i){
			TPair& Pair   = Pairs[i];
			INT    iHash  = (GetTypeHash(Pair.Key) & (HashCount - 1));

			Pair.HashNext = Hash[iHash];
			Hash[iHash]   = i;
		}
	}

	void Relax(){
		while(HashCount > Pairs.Num() * 2 + 8)
			HashCount /= 2;

		Rehash();
	}

	TI& Add(typename TTypeInfo<TK>::ConstInitType InKey, typename TTypeInfo<TI>::ConstInitType InValue){
		TPair& Pair   = *new(Pairs)TPair(InKey, InValue);
		INT    iHash  = (GetTypeHash(Pair.Key) & (HashCount-1));

		Pair.HashNext = Hash[iHash];
		Hash[iHash]   = Pairs.Num()-1;
		if(HashCount * 2 + 8 < Pairs.Num()){
			HashCount *= 2;
			Rehash();
		}
		return Pair.Value;
	}

public:
	TMapBase() : Hash(NULL),
	             HashCount(8){
		Rehash();
	}

	TMapBase(const TMapBase& Other) : Pairs(Other.Pairs),
	                                  HashCount(Other.HashCount),
	                                  Hash(NULL){
		if(HashCount < 8) // Need to check this in case the other map was zero initialized without the constructor being called
			HashCount = 8;

		Rehash();
	}

	~TMapBase(){
		if(Hash)
			appFree(Hash);

		Hash = NULL;
		HashCount = 0;
	}

	TMapBase& operator=(const TMapBase& Other){
		Pairs     = Other.Pairs;
		HashCount = Other.HashCount;
		Rehash();

		return *this;
	}

	void Empty(){
		Pairs.Empty(0);
		HashCount = 8;
		Rehash();
	}

	TI& Set(typename TTypeInfo<TK>::ConstInitType InKey, typename TTypeInfo<TI>::ConstInitType InValue){
		for(INT i = Hash[(GetTypeHash(InKey) & (HashCount - 1))]; i != INDEX_NONE; i = Pairs[i].HashNext){
			if(Pairs[i].Key == InKey){
				Pairs[i].Value = InValue;

				return Pairs[i].Value;
			}
		}

		return Add(InKey, InValue);
	}

	TI& operator[](typename TTypeInfo<TK>::ConstInitType Key){
		TI* Entry = Find(Key);

		if(Entry)
			return *Entry;

		// Retarded hack to make this work with FString since its ConstInitType is char* and not const FString&
		static const BYTE ZeroedDefault[sizeof(TI)] = {0};

		return Add(Key, reinterpret_cast<typename TTypeInfo<TI>::ConstInitType>(ZeroedDefault));
	}

	INT Remove(typename TTypeInfo<TK>::ConstInitType InKey){
		INT Count = 0;

		for(INT i = Pairs.Num() - 1; i >= 0; --i){
			if(Pairs[i].Key == InKey){
				Pairs.Remove(i);
				++Count;
			}
		}

		if(Count)
			Relax();

		return Count;
	}

	TI* Find(const TK& Key){
		for(INT i = Hash[(GetTypeHash(Key) & (HashCount - 1))]; i != INDEX_NONE; i = Pairs[i].HashNext){
			if(Pairs[i].Key == Key)
				return &Pairs[i].Value;
		}

		return NULL;
	}

	TI FindRef(const TK& Key){
		for(INT i = Hash[(GetTypeHash(Key) & (HashCount - 1))]; i != INDEX_NONE; i = Pairs[i].HashNext){
			if(Pairs[i].Key == Key)
				return Pairs[i].Value;
		}

		return NULL;
	}

	const TI* Find(const TK& Key) const{
		for(INT i = Hash[(GetTypeHash(Key) & (HashCount - 1))]; i != INDEX_NONE; i = Pairs[i].HashNext){
			if(Pairs[i].Key == Key)
				return &Pairs[i].Value;
		}

		return NULL;
	}

	friend FArchive& operator<<(FArchive& Ar, TMapBase& M){
		Ar << M.Pairs;

		if(Ar.IsLoading())
			M.Rehash();

		Ar.CountBytes(M.Hash, M.HashCount * sizeof(M.Hash));

		return Ar;
	}

	void Dump(FOutputDevice& Ar){
		guard(TMapBase::Dump);

		Ar.Logf("TMapBase: %i items, %i hash slots", Pairs.Num(), HashCount);

		for(INT i = 0; i < HashCount; ++i){
			INT c = 0;

			for(INT j = Hash[i]; j != INDEX_NONE; j = Pairs[j].HashNext)
				c++;

			Ar.Logf("   Hash[%i] = %i", i, c);
		}

		unguard;
	}

	class TIterator{
	public:
		TIterator(TMapBase& InMap) : Pairs(InMap.Pairs), Index(0){}
		void operator++()          { ++Index; }
		void RemoveCurrent()       { Pairs.Remove(Index--); }
		operator UBOOL() const     { return Index<Pairs.Num(); }
		TK& Key() const            { return Pairs[Index].Key; }
		TI& Value() const          { return Pairs[Index].Value; }

	private:
		TArray<TPair>& Pairs;
		INT Index;
	};
};

template<typename TK, typename TI>
class TMap : public TMapBase<TK,TI>{
public:
	TMap& operator=(const TMap& Other){
		TMapBase<TK,TI>::operator=(Other);

		return *this;
	}

	int Num() const{
		return Pairs.Num();
	}
};

template<typename TK, typename TI>
class TMultiMap : public TMapBase<TK,TI>{
public:
	TMultiMap& operator=(const TMultiMap& Other){
		TMapBase<TK,TI>::operator=(Other);
		return *this;
	}

	void MultiFind(const TK& Key, TArray<TI>& Values) const{
		for(INT i = Hash[(GetTypeHash(Key) & (HashCount - 1))]; i != INDEX_NONE; i = Pairs[i].HashNext){
			if(Pairs[i].Key == Key)
				new(Values) TI(Pairs[i].Value);
		}
	}

	TI& Add(typename TTypeInfo<TK>::ConstInitType InKey, typename TTypeInfo<TI>::ConstInitType InValue){
		return TMapBase<TK,TI>::Add(InKey, InValue);
	}

	TI& AddUnique(typename TTypeInfo<TK>::ConstInitType InKey, typename TTypeInfo<TI>::ConstInitType InValue){
		for(INT i = Hash[(GetTypeHash(InKey) & (HashCount - 1))]; i != INDEX_NONE; i = Pairs[i].HashNext){
			if(Pairs[i].Key == InKey && Pairs[i].Value == InValue)
				return Pairs[i].Value;
		}

		return Add(InKey, InValue);
	}

	INT RemovePair(typename TTypeInfo<TK>::ConstInitType InKey, typename TTypeInfo<TI>::ConstInitType InValue){
		INT Count = 0;

		for(INT i = Pairs.Num() - 1; i >= 0; --i){
			if(Pairs[i].Key==InKey && Pairs[i].Value==InValue){
				Pairs.Remove(i);
				++Count;
			}
		}

		if(Count)
			Relax();

		return Count;
	}

	TI* FindPair(const TK& Key, const TK& Value){
		for(INT i = Hash[(GetTypeHash(Key) & (HashCount - 1))]; i != INDEX_NONE; i = Pairs[i].HashNext){
			if(Pairs[i].Key == Key && Pairs[i].Value == Value)
				return &Pairs[i].Value;
		}

		return NULL;
	}
};

/*----------------------------------------------------------------------------
	Sorting template.
----------------------------------------------------------------------------*/

template<typename T>
struct TStack{
	T* Min;
	T* Max;
};

template<typename T>
INT Compare(const T& T1, const T& T2){
	if(T1 < T2)
		return -1;

	if(T1 > T2)
		return 1;

	return 0;
}

/*
 * Sort elements.
 *
 * The sort is unstable, meaning that the ordering of equal
 * items is not necessarily preserved.
 */
template<typename T>
void Sort(T* First, INT Num){
	guard(Sort);

	if(Num < 2)
		return;

	TStack<T> RecursionStack[32]={{First,First+Num-1}}, Current, Inner;

	for(TStack<T>* StackTop=RecursionStack; StackTop>=RecursionStack; --StackTop){
		Current = *StackTop;
	Loop:
		INT Count = Current.Max - Current.Min + 1;
		if(Count <= 8){
			// Use simple bubble-sort.
			while(Current.Max > Current.Min){
				T *Max, *Item;

				for(Max=Current.Min, Item=Current.Min+1; Item<=Current.Max; Item++){
					if(Compare(*Item, *Max) > 0)
						Max = Item;
				}

				Exchange(*Max, *Current.Max--);
			}
		}else{
			// Grab middle element so sort doesn't exhibit worst-cast behaviour with presorted lists.
			Exchange(Current.Min[Count/2], Current.Min[0]);

			// Divide list into two halves, one with items <=Current.Min, the other with items >Current.Max.
			Inner.Min = Current.Min;
			Inner.Max = Current.Max+1;
			for(; ;)
			{
				while(++Inner.Min<=Current.Max && Compare(*Inner.Min, *Current.Min) <= 0);
				while(--Inner.Max> Current.Min && Compare(*Inner.Max, *Current.Min) >= 0);
				if(Inner.Min>Inner.Max)
					break;
				Exchange(*Inner.Min, *Inner.Max);
			}
			Exchange(*Current.Min, *Inner.Max);

			// Save big half and recurse with small half.
			if(Inner.Max-1-Current.Min >= Current.Max-Inner.Min){
				if(Current.Min+1 < Inner.Max){
					StackTop->Min = Current.Min;
					StackTop->Max = Inner.Max - 1;
					StackTop++;
				}

				if(Current.Max>Inner.Min){
					Current.Min = Inner.Min;
					goto Loop;
				}
			}else{
				if(Current.Max > Inner.Min){
					StackTop->Min = Inner  .Min;
					StackTop->Max = Current.Max;
					StackTop++;
				}

				if(Current.Min + 1 < Inner.Max){
					Current.Max = Inner.Max - 1;
					goto Loop;
				}
			}
		}
	}
	unguard;
}

/*----------------------------------------------------------------------------
	TList.
----------------------------------------------------------------------------*/

//
// Simple single-linked list template.
//
template<typename ElementType>
class TList{
public:
	ElementType         Element;
	TList<ElementType>* Next;

	// Constructor.

	TList(ElementType InElement,TList<ElementType>* InNext = NULL){
		Element = InElement;
		Next = InNext;
	}
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
