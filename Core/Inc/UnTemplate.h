/*=============================================================================
	UnTemplate.h: Unreal templates.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Type information.
-----------------------------------------------------------------------------*/

//
// Type information for initialization.
//
template<typename T>
struct TTypeInfoBase{
	typedef const T& ConstInitType;

	static UBOOL NeedsDestructor(){ return 1; }
	static UBOOL DefinitelyNeedsDestructor(){ return 0; }
	static const T& ToInit(const T& In){ return In; }
};

template<typename T>
struct TTypeInfo : public TTypeInfoBase<T>{};

template<>
struct TTypeInfo<BYTE> : public TTypeInfoBase<BYTE>{
	static UBOOL NeedsDestructor(){ return 0; }
};

template<>
struct TTypeInfo<SBYTE> : public TTypeInfoBase<SBYTE>{
	static UBOOL NeedsDestructor(){ return 0; }
};

template<>
struct TTypeInfo<ANSICHAR> : public TTypeInfoBase<ANSICHAR>{
	static UBOOL NeedsDestructor(){ return 0; }
};

template<>
struct TTypeInfo<INT> : public TTypeInfoBase<INT>{
	static UBOOL NeedsDestructor(){ return 0; }
};

template<>
struct TTypeInfo<DWORD> : public TTypeInfoBase<DWORD>{
	static UBOOL NeedsDestructor(){ return 0; }
};

template<>
struct TTypeInfo<_WORD> : public TTypeInfoBase<_WORD>{
	static UBOOL NeedsDestructor(){ return 0; }
};

template<>
struct TTypeInfo<SWORD> : public TTypeInfoBase<SWORD>{
	static UBOOL NeedsDestructor(){ return 0; }
};

template<>
struct TTypeInfo<QWORD> : public TTypeInfoBase<QWORD>{
	static UBOOL NeedsDestructor(){ return 0; }
};

template<>
struct TTypeInfo<SQWORD> : public TTypeInfoBase<SQWORD>{
	static UBOOL NeedsDestructor(){ return 0; }
};

template<>
struct TTypeInfo<FName> : public TTypeInfoBase<FName>{
	static UBOOL NeedsDestructor(){ return 0; }
};

template<>
struct TTypeInfo<UObject*> : public TTypeInfoBase<UObject*>{
	static UBOOL NeedsDestructor(){ return 0; }
};

/*-----------------------------------------------------------------------------
	Standard templates.
-----------------------------------------------------------------------------*/

template<typename T>
inline T Abs(const T A){
	return (A>=(T)0) ? A : -A;
}

template<typename T>
inline T Sgn(const T A){
	return (A>0) ? 1 : ((A<0) ? -1 : 0);
}

template<typename T>
inline T Max(const T A, const T B){
	return (A>=B) ? A : B;
}

template<typename T>
inline T Min(const T A, const T B){
	return (A<=B) ? A : B;
}

template<typename T>
inline T Square(const T A){
	return A*A;
}

template<typename T>
inline T Clamp(const T X, const T Min, const T Max){
	return X<Min ? Min : X<Max ? X : Max;
}

template<typename T>
inline T Align(const T Ptr, INT Alignment){
	return (T)(((DWORD)Ptr + Alignment - 1) & ~(Alignment-1));
}

template<typename T>
inline void Exchange(T& A, T& B){
	const T Temp = A;
	A = B;
	B = Temp;
}

template<typename T>
T Lerp(T& A, T& B, FLOAT Alpha){
	return A + Alpha * (B-A);
}

inline DWORD GetTypeHash(const BYTE A){
	return A;
}

inline DWORD GetTypeHash(const SBYTE A){
	return A;
}

inline DWORD GetTypeHash(const _WORD A){
	return A;
}

inline DWORD GetTypeHash(const SWORD A){
	return A;
}

inline DWORD GetTypeHash(const INT A){
	return A;
}

inline DWORD GetTypeHash(const DWORD A){
	return A;
}

inline DWORD GetTypeHash(const QWORD A){
	return (DWORD)A+((DWORD)(A>>32) * 23);
}

inline DWORD GetTypeHash(const SQWORD A){
	return (DWORD)A+((DWORD)(A>>32) * 23);
}

inline DWORD GetTypeHash(const TCHAR* S){
	return appStrihash(S);
}

#define ExchangeB(A,B) do{UBOOL T=A; A=B; B=T;}while(false);

/*----------------------------------------------------------------------------
	Standard macros.
----------------------------------------------------------------------------*/

// Number of elements in an array.
#define ARRAY_COUNT(array) \
	(sizeof(array) / sizeof((array)[0]))

// Offset of a struct member.
#define STRUCT_OFFSET(struc, member) \
	((INT)&((struc*)NULL)->member)

/*-----------------------------------------------------------------------------
	Allocators.
-----------------------------------------------------------------------------*/

template<typename T>
class TAllocator{};

/*-----------------------------------------------------------------------------
	Dynamic array template.
-----------------------------------------------------------------------------*/

//
// Former dynamic array base. All functionality has been moved to TArray.
// This class only exists for FTransactionBase::SaveArray
//
class FArray{
protected:
	void* Data;
	INT ArrayNum;
};

//
// Template dynamic array
//
template<typename T>
class TArray{
public:
	TArray() : Data(NULL),
			   ArrayNum(0){}

	TArray(T* Src, INT Count){
		Data = Src;
		ArrayNum = Count & 0x1FFFFFFF | 0x20000000;
	}

	TArray(ENoInit){}

	TArray(INT Size, bool What = false) : Data(NULL),
										  ArrayNum(What << 31){
		if(Size)
			Set(Size, Size);
	}

	TArray(const TArray<T>& Other, bool What) : Data(NULL),
												ArrayNum(What << 31){
		*this = Other;
	}

	~TArray(){
		if(IsAllocated())
			appFree(Data);

		Data = NULL;
		ArrayNum &= 0xE0000000;
	}

	void Set(T* Src, INT Count){
		Data = Src;
		ArrayNum = (Count ^ ArrayNum) & 0x1FFFFFFF ^ ArrayNum; //???
	}

	T* GetData(){
		return static_cast<T*>(Data);
	}

	const T* GetData() const{
		return static_cast<T*>(Data);
	}

	bool IsValidIndex(INT Index){
		return Index >= 0 && Index < Num();
	}

	INT Num() const{
		return 8 * ArrayNum >> 3;
	}

	INT Size() const{
		return Num();
	}

	bool IsAllocated() const{
		return Data != NULL && (ArrayNum & 0x20000000) == 0;
	}

	T& operator[](INT Index){
		return GetData()[Index];
	}

	const T& operator[](INT Index) const{
		return static_cast<T*>(Data)[Index];
	}

	T& Last(INT c = 0){
		return (*this)[Num() - c - 1];
	}

	const T& Last(INT c = 0) const{
		return (*this)[Num() - c - 1];
	}

	bool FindItem(const T& Item, INT& Index) const{
		for(Index = 0; Index < Num(); Index++){
			if((*this)[Index] == Item)
				return true;
		}

		return false;
	}

	INT FindItemIndex(const T& Item) const{
		for(INT Index = 0; Index < Num(); Index++){
			if((*this)[Index] == Item)
				return Index;
		}

		return INDEX_NONE;
	}

	void SetNoShrink(bool bNoShrink){
		ArrayNum &= 0x7FFFFFFF; //Clearing bit flag in case it is set
		ArrayNum |= bNoShrink << 31; //Setting it if bNoShrink is true
	}

	void CountBytes(FArchive& Ar){
		Ar.CountBytes(Data, Num() * sizeof(T));
	}

	INT GetMaxSize() const{
		if(IsAllocated())
			return GMalloc->GetAllocationSize(Data) / sizeof(T);
		else
			return 0;
	}

	void Serialize(FArchive& Ar){
		CountBytes(Ar);

		if(sizeof(T) == 1){
			//Serialize simple bytes which require no construction or destruction.
			if(Ar.IsLoading()){
				Ar << AR_INDEX(ArrayNum);
				Realloc(Num(), 0);
			}else{
				INT TempNum = Num();

				Ar << AR_INDEX(TempNum);
			}

			Ar.Serialize(&(*this)[0], Num());
		}else if(Ar.IsLoading()){
			//Load array.
			INT NewNum;

			Ar << AR_INDEX(NewNum);

			Empty(NewNum);

			for(INT i = 0; i < NewNum; i++)
				Ar << *new(*this)T;
		}else{
			//Save array.
			INT TempNum = Num();
			Ar << AR_INDEX(TempNum);

			for(INT i = 0; i < Num(); i++)
				Ar << (*this)[i];
		}
	}

	void Reserve(INT Size){
		INT Slack;

		Slack = Capacity();

		if(Size > Slack)
			Slack = Size;

		Realloc(Num(), Slack);
	}

	void Insert(INT Index, INT Count = 1, bool bInit = false){
		Realloc(Num() + Count, 0);

		appMemmove(
			static_cast<BYTE*>(Data) + (Index + Count) * sizeof(T),
			static_cast<BYTE*>(Data) + Index * sizeof(T),
			(Num() - Index - Count) * sizeof(T)
		);

		if(bInit)
			Init(Index, Count)
	}

	INT Add(INT Count, bool bInit = false){
		Realloc(Num() + Count, 0);

		if(bInit)
			Init(Num() - Count, Count);

		return Num() - Count;
	}

	void InsertZeroed(INT Index, INT Count){
		Insert(Index, Count);
		appMemZero(static_cast<BYTE*>(Data) + Index * sizeof(T), Count * sizeof(T));
	}

	INT AddZeroed(INT Count){
		INT Index = Add(Count);

		appMemzero(static_cast<BYTE*>(Data) + Index * sizeof(T), Count * sizeof(T));
	}

	void Shrink(){
		Realloc(Num(), Num());
	}

	void Empty(INT Slack){
		Realloc(0, Slack);
	}

	void Remove(INT Index, INT Count = 1){
		if(TTypeInfo<T>::NeedsDestructor()){
			for(INT i = Index; i < Index + Count; i++)
				(&(*this)[i])->~T();
		}

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

		for(INT Index = 0; Index < Num(); Index++){
			if((*this)[Index] == Item)
				Remove(Index--);
		}

		return OldNum - Num();
	}

	T Pop(){
		T Result = (*this)[Num() - 1];

		Remove(Num() - 1);

		return Result;
	}

	void Transfer(TArray<T>& Src){
		Realloc(0, 0);

		Data = Src.Data;
		ArrayNum = (ArrayNum ^ Src.Num()) & 0x1FFFFFFF ^ ArrayNum; //???
		ArrayNum = ArrayNum ^ (ArrayNum ^ (4 * Src.ArrayNum >> 2)) & 0x20000000; //???

		Src.Unreference();
	}

	void Set(INT NewSize, INT Slack){
		INT OldNum = Num();

		if(NewSize >= Num())
			Realloc(NewSize, Slack);
		else
			Remove(NewSize, Num() - NewSize);

		if(Num() > OldNum)
			appMemzero(static_cast<BYTE*>(Data) + OldNum, 4 * ((Num() - OldNum) >> 2));
	}

	void Set(INT NewSize){
		Set(NewSize, NewSize);
	}

	TArray<T>& operator+(const TArray<T>& Other){
		if(this != &Other){
			for(INT i = 0; i < Other.Num(); i++)
				new(*this) T(Other[i]);
		}

		return *this;
	}

	TArray<T>& operator+=(const TArray<T>& Other){
		if(this != &Other)
			*this = *this + Other;

		return *this;
	}

	INT AddItem(const T& Item){
		new(*this) T(Item);

		return Num() - 1;
	}

	INT AddUniqueItem(const T& Item){
		for(INT Index = 0; Index < Num(); Index++){
			if((*this)[Index] == Item)
				return Index;
		}

		return AddItem(Item);
	}

	TArray<T>& operator=(const TArray<T>& Other){
		if(this != &Other){
			if(!(Other.ArrayNum & 0x60000000)){
				Realloc(Other.Num(), 0);
				appMemcpy(Data, Other.Data, Other.Num());
			}else{
				Transfer(const_cast<TArray<T>&>(Other));
			}
		}

		return *this;
	}

	TArray<T> Segment(INT Index, INT Count);/*{
		//TODO: Implement
	}*/

	//Iterator
	class TIterator{
	public:
		TIterator(TArray<T>& InArray) : Array(InArray), Index(-1) { ++*this;         }
		void operator++()     { ++Index;                                             }
		void RemoveCurrent()  { Array.Remove(Index--);                               }
		INT GetIndex()   const{ return Index;                                        }
		operator UBOOL() const{ return Index < Array.Num();                          }
		T& operator*()   const{ return Array[Index];                                 }
		T* operator->()  const{ return &Array[Index];                                }
		T& GetCurrent()  const{ return Array[Index];                                 }
		T& GetPrev()     const{ return Array[Index ? Index - 1 : Array.Num() - 1 ];  }
		T& GetNext()     const{ return Array[Index<Array.Num() - 1 ? Index + 1 : 0]; }

	private:
		TArray<T>& Array;
		INT Index;
	};

protected:
	void* Data;
	INT ArrayNum;

	INT Capacity() const{
		return GetMaxSize();
	}

	void Unreference() const{
		const_cast<INT&>(ArrayNum) |= 0x20000000;
	}

	void Init(INT, INT){
		//TODO: Find out how this is implemented
		/*
			void *v3; // edi@1
			int result; // eax@1
			int v5; // edi@1
			int i; // ecx@1

			v3 = (void *)(a2 + *(_DWORD *)this);
			result = 0;
			memset(v3, 0, 4 * (a3 >> 2));
			v5 = (int)((char *)v3 + 4 * (a3 >> 2));
			for ( i = a3 & 3; i; --i )
			*(_BYTE *)v5++ = 0;
			return result;
		*/
	}

	void Realloc(INT NewSize, INT Slack){
		if(IsAllocated()){
			if(NewSize <= Num()){
				if(ArrayNum >= 0 && NewSize < Num())
					Data = appRealloc(Data, NewSize * sizeof(T), (NewSize >= Slack ? NewSize : Slack) * sizeof(T));
			}else{
				Data = appRealloc(Data, NewSize * sizeof(T), Slack * sizeof(T));
			}
		}else{
			if(NewSize > 0 || Slack > 0){
				void* Temp = appMalloc((NewSize >= Slack ? NewSize : Slack) * sizeof(T));

				if(Data)
					appMemcpy(Temp, Data, (Num() > NewSize ? Num() : NewSize) * sizeof(T));

				Data = Temp;
			}else{
				Data = NULL;
			}
		}

		ArrayNum ^= (NewSize ^ ArrayNum) & 0x1FFFFFFF; //???
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
			GUndo->SaveArray(Owner, reinterpret_cast<FArray*>(this), Index, Count, 1, sizeof(T), SerializeItem, DestructItem);
		return Index;
	}

	void Insert(INT Index, INT Count = 1){
		FArray::Insert(Index, Count, sizeof(T));
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
			GUndo->SaveArray(Owner, reinterpret_cast<FArray*>(this), 0, ArrayNum, -1, sizeof(T), SerializeItem, DestructItem);

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

//
// Lazy loader base class.
//
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
	/**
	 * Returns the byte offset to the payload.
	 *
	 * @return offset in bytes from beginning of file to beginning of data
	 */
	virtual DWORD GetOffset(){ 
		return Abs(SavedPos); 
	}
};

//
// Lazy-loadable dynamic array.
//
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
	/**
	 * Returns the byte offset to the payload, skipping the array size serialized
	 * by TArray's serializer.
	 *
	 * @return offset in bytes from beginning of file to beginning of data
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

//Forward declaration
class FStringTemp;

//
// A dynamically sizeable string.
//
class FString : protected TArray<TCHAR>{
public:
	FString() : TArray<TCHAR>(){}

	FString(const TCHAR* In, bool What = false){
		if(In && *In){
			if(!What){
				Realloc(appStrlen(In) + 1, 0);
				appMemcpy(Data, In, Num());
			}else{
				Data = const_cast<TCHAR*>(In);
				ArrayNum = (appStrlen(In) + 1) & 0x1FFFFFFF | 0x20000000;
			}
		}
	}

	FString(ENoInit) : TArray<TCHAR>(E_NoInit){}

	FString(const FString& Other){
		TArray<TCHAR>::operator=(Other);
	}

	FString(const FStringTemp& Other);

	~FString(){
		if(IsAllocated())
			appFree(Data);

		Data = NULL;
		ArrayNum &= 0xE0000000;
	}

	TCHAR& operator[](INT Index){
		return TArray<TCHAR>::operator[](Index);
	}

	const TCHAR& operator[](INT Index) const{
		return TArray<TCHAR>::operator[](Index);
	}

	TArray<TCHAR>& GetCharArray(){
		return static_cast<TArray<TCHAR>&>(*this);
	}

	const TCHAR* operator*() const{
		return Num() > 0 ? GetData() : "";
	}

	FString& operator*=(const TCHAR* Str);

	FString& operator*=(const FString& Str){
		if(Str.Num() > 0)
			return operator*=(*Str);

		return operator*=("");
	}

	bool operator<=(const TCHAR* Other) const{
		if(Num() > 0)
			return appStricmp(GetData(), Other) <= 0;

		return appStricmp("", Other) <= 0;
	}

	bool operator<=(const FString& Other) const{
		if(Other.Num() > 0)
			return operator<=(*Other);

		return operator<=("");
	}

	bool operator<(const TCHAR* Other) const{
		if(Num() > 0)
			return appStricmp(GetData(), Other) < 0;

		return appStricmp("", Other) < 0;
	}

	bool operator<(const FString& Other) const{
		if(Other.Num() > 0)
			return operator<(*Other);

		return operator<("");
	}

	bool operator>=(const TCHAR* Other) const{
		if(Num() > 0)
			return appStricmp(GetData(), Other) >= 0;

		return appStricmp("", Other) >= 0;
	}

	bool operator>=(const FString& Other) const{
		if(Other.Num() > 0)
			return operator>=(*Other);

		return operator>=("");
	}

	bool operator>(const TCHAR* Other) const{
		if(Num() > 0)
			return appStricmp(GetData(), Other) > 0;

		return appStricmp("", Other) > 0;
	}

	bool operator>(const FString& Other) const{
		if(Other.Num() > 0)
			return operator>(*Other);

		return operator>("");
	}

	bool operator==(const TCHAR* Other) const{
		if(Num() > 0)
			return appStricmp(GetData(), Other) == 0;

		return appStricmp("", Other);
	}

	bool operator==(const FString& Other) const{
		if(Other.Num() > 0)
			return operator==(*Other);

		return operator==("");
	}

	bool operator!=(const TCHAR* Other) const{
		if(Num() > 0)
			return appStricmp(GetData(), Other) != 0;

		return appStricmp("", Other) != 0;
	}

	bool operator!=(const FString& Other) const{
		if(Other.Num() > 0)
			return operator!=(Other);

		return operator!=("");
	}

	INT Len() const{
		return Num() ? Num() - 1 : 0;
	}

	INT InStr(const TCHAR* SubStr, bool Right = false) const;/*{
		//TODO: Implement
	}*/

	INT InStr(const FString& SubStr, bool Right = false) const{
		return InStr(*SubStr, Right);
	}

	TCHAR* MakeCharArray(){
		TCHAR* Result = static_cast<TCHAR*>(appMalloc(Len() + 1));

		for(int i = 0; i < Len(); i++){
			if(static_cast<_WORD>((*this)[i]) < 256)
				Result[i] = (*this)[i];
			else
				Result[i] = 127;
		}

		Result[Len()] = '\0';
	}

	FStringTemp Right(INT Count) const;

	void Empty(){
		TArray<TCHAR>::Empty(0);
	}

	void Shrink(){
		TArray<TCHAR>::Shrink();
	}

	FString& operator+=(const TCHAR* Str){
		if(*Str != '\0'){
		    if(Num()){
			    INT Index = Num() - 1;

			    Add(appStrlen(Str));
			    appStrcpy(&(*this)[Index], Str);
		    }else if(*Str){
			    Add(appStrlen(Str) + 1);
			    appStrcpy(&(*this)[0], Str);
		    }
		}

		return *this;
	}

	FString& operator+=(const FString& Str){
		if(Str.Num() > 0)
			return operator+=(*Str);

		return operator+=("");
	}

	FString& operator=(const TCHAR* Other){
		if(Other && *Other != '\0'){
			Realloc(appStrlen(Other) + 1, 0);
			appMemcpy(Data, Other, Num());
		}else{
			Realloc(0, 0);
		}

		return *this;
	}

	FString& operator=(const FString& Other){
		TArray<TCHAR>::operator=(Other);

		return *this;
	}

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

protected:
	FString(INT Size){
		if(Size)
			Set(Size, Size);
	}

	FString(INT InCount, const TCHAR* InSrc){
		INT Size = InCount ? InCount + 1 : 0;

		if(Size)
			Set(Size, Size);

		if(Num() > 0)
			appStrncpy(GetData(), InSrc, InCount + 1);
	}

	friend FArchive& operator<<(FArchive& Ar, FString& String){
		String.Serialize(Ar);

		return Ar;
	}
};

class FStringTemp : public FString{
public:
	FStringTemp(const TCHAR* In, bool What = false){
		if(*In){
			if(!What){
				Realloc(appStrlen(In) + 1, 0);
				appMemcpy(Data, In, Num());
				ArrayNum |= 0x40000000;
			}else{
				Data = const_cast<TCHAR*>(In);
				ArrayNum = (appStrlen(In) + 1) & 0x1FFFFFFF | 0x20000000;
			}
		}
	}

	FStringTemp(INT Count){
		if(Count)
			Set(Count, Count);

		ArrayNum |= 0x40000000;
	}

	FStringTemp(INT Count, const TCHAR* In) : FString(Count, In){
		ArrayNum |= 0x40000000;
	}

	FStringTemp(const FStringTemp& Other){
		TArray<TCHAR>::operator=(Other);
		ArrayNum |= 0x40000000;
	}

	FStringTemp(const FString& Other){
		TArray<TCHAR>::operator=(Other);
		ArrayNum |= 0x40000000;
	}

	~FStringTemp(){
		if(IsAllocated())
			appFree(Data);

		Data = NULL;
		ArrayNum &= 0xE0000000;
	}
};

struct FStringNoInit : public FString{
public:
	FStringNoInit(const FStringNoInit&);
	FStringNoInit();
	~FStringNoInit();

	FStringNoInit& operator=(const FStringNoInit&);
	FStringNoInit& operator=(const FString&);
	FStringNoInit& operator=(const TCHAR*);
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

//
// String exchanger.
//
inline void ExchangeString(FString& A, FString& B){
	guardSlow(ExchangeTArray);
	appMemswap(&A, &B, sizeof(FString));
	unguardSlow;
}

/*----------------------------------------------------------------------------
	Special archivers.
----------------------------------------------------------------------------*/

//
// String output device.
//
class FStringOutputDevice : public FString, public FOutputDevice{
public:
	FStringOutputDevice(const TCHAR* InStr=TEXT(""))
	: FString(InStr)
	{}
	void Serialize(const TCHAR* Data, EName Event)
	{
		*this += (TCHAR*)Data;
	}
};

//
// Buffer writer.
//
class FBufferWriter : public FArchive
{
public:
	FBufferWriter(TArray<BYTE>& InBytes)
	: Bytes(InBytes)
	, Pos(0)
	{
		ArIsSaving = 1;
	}
	void Serialize(void* InData, INT Length)
	{
		if(Pos+Length>Bytes.Num())
			Bytes.Add(Pos+Length-Bytes.Num());
		if(Length == 1)
			Bytes[Pos] = ((BYTE*)InData)[0];
		else
			appMemcpy(&Bytes[Pos], InData, Length);
		Pos += Length;
	}
	INT Tell()
	{
		return Pos;
	}
	void Seek(INT InPos)
	{
		Pos = InPos;
	}
	INT TotalSize()
	{
		return Bytes.Num();
	}
private:
	TArray<BYTE>& Bytes;
	INT Pos;
};

//
// Buffer archiver.
//
class FBufferArchive : public FBufferWriter, public TArray<BYTE>{
public:
	FBufferArchive() : FBufferWriter((TArray<BYTE>&)*this){}
};

//
// Buffer reader.
//
class CORE_API FBufferReader : public FArchive{
public:
	FBufferReader(const TArray<BYTE>& InBytes) : Bytes(InBytes),
												 Pos(0){
		ArIsLoading = ArIsTrans = 1;
	}
	
	void Serialize(void* Data, INT Num){
		check(Pos >= 0);
		check(Pos+Num<=Bytes.Num());
		if(Num == 1)
			((BYTE*)Data)[0] = Bytes[Pos];
		else
			appMemcpy(Data, &Bytes[Pos], Num);
		Pos += Num;
	}

	INT Tell(){
		return Pos;
	}

	INT TotalSize(){
		return Bytes.Num();
	}

	void Seek(INT InPos){
		check(InPos >= 0);
		check(InPos<=Bytes.Num());
		Pos = InPos;
	}

	UBOOL AtEnd(){
		return Pos>=Bytes.Num();
	}
private:
	const TArray<BYTE>& Bytes;
	INT Pos;
};

/*----------------------------------------------------------------------------
	TMap.
----------------------------------------------------------------------------*/

//
// Maps unique keys to values.
//
template<typename TK, typename TI>
class TMapBase{
protected:
	class TPair{
	public:
		INT HashNext;
		TK Key;
		TI Value;
		void* Null; //Seems to be a pointer but no idea what it stores

		TPair(typename TTypeInfo<TK>::ConstInitType InKey, typename TTypeInfo<TI>::ConstInitType InValue) : Key(InKey),
																											Value(InValue){}
		TPair(){}

		friend FArchive& operator<<(FArchive& Ar, TPair& F){
			return Ar << F.Key << F.Value;
		}
	};

	void Rehash(){
		INT* NewHash = new INT[HashCount];
		{for(INT i = 0; i<HashCount; i++)
		{
			NewHash[i] = INDEX_NONE;
		}}
		{for(INT i = 0; i<Pairs.Num(); i++)
		{
			TPair& Pair    = Pairs[i];
			INT    iHash   = (GetTypeHash(Pair.Key) & (HashCount-1));
			Pair.HashNext  = NewHash[iHash];
			NewHash[iHash] = i;
		}}
		if(Hash)
			appFree(Hash);
		Hash = NewHash;
	}

	void Relax(){
		while(HashCount>Pairs.Num()*2+8)
			HashCount /= 2;

		Rehash();
	}

	TI& Add(typename TTypeInfo<TK>::ConstInitType InKey, typename TTypeInfo<TI>::ConstInitType InValue){
		TPair& Pair   = *new(Pairs)TPair(InKey, InValue);
		INT    iHash  = (GetTypeHash(Pair.Key) & (HashCount-1));
		Pair.HashNext = Hash[iHash];
		Hash[iHash]   = Pairs.Num()-1;

		if(HashCount*2+8 < Pairs.Num()){
			HashCount *= 2;
			Rehash();
		}

		return Pair.Value;
	}

	TArray<TPair> Pairs;
	INT* Hash;
	INT HashCount;

public:
	TMapBase() : Hash(NULL),
				 HashCount(8){
		Rehash();
	}
	TMapBase(const TMapBase& Other) : Pairs(Other.Pairs),
									  HashCount(Other.HashCount),
									  Hash(NULL){
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
		for(INT i = Hash[(GetTypeHash(InKey) & (HashCount-1))]; i != INDEX_NONE; i = Pairs[i].HashNext)
			if(Pairs[i].Key == InKey)
				{Pairs[i].Value = InValue; return Pairs[i].Value;}

		return Add(InKey, InValue);
	}

	INT Remove(typename TTypeInfo<TK>::ConstInitType InKey){
		INT Count = 0;
		for(INT i=Pairs.Num()-1; i >= 0; i--){
			if(Pairs[i].Key==InKey)
				Pairs.Remove(i); Count++;
		}

		if(Count)
			Relax();

		return Count;
	}

	TI* Find(const TK& Key){
		for(INT i = Hash[(GetTypeHash(Key) & (HashCount-1))]; i != INDEX_NONE; i = Pairs[i].HashNext)
			if(Pairs[i].Key == Key)
				return &Pairs[i].Value;

		return NULL;
	}

	TI FindRef(const TK& Key){
		for(INT i = Hash[(GetTypeHash(Key) & (HashCount-1))]; i != INDEX_NONE; i = Pairs[i].HashNext){
			if(Pairs[i].Key == Key)
				return Pairs[i].Value;
		}

		return NULL;
	}

	const TI* Find(const TK& Key) const{
		for(INT i = Hash[(GetTypeHash(Key) & (HashCount-1))]; i != INDEX_NONE; i = Pairs[i].HashNext){
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
		Ar.Logf(TEXT("TMapBase: %i items, %i hash slots"), Pairs.Num(), HashCount);
		for(INT i = 0; i<HashCount; i++)
		{
			INT c = 0;
			for(INT j=Hash[i]; j!=INDEX_NONE; j=Pairs[j].HashNext)
				c++;
			Ar.Logf(TEXT("   Hash[%i] = %i"), i, c);
		}
		unguard;
	}

	class TIterator{
	public:
		TIterator(TMapBase& InMap) : Pairs(InMap.Pairs), Index(0) {}
		void operator++()          { ++Index; }
		void RemoveCurrent()       { Pairs.Remove(Index--); }
		operator UBOOL() const     { return Index<Pairs.Num(); }
		TK& Key() const            { return Pairs[Index].Key; }
		TI& Value() const          { return Pairs[Index].Value; }

	private:
		TArray<TPair>& Pairs;
		INT Index;
	};

	friend class TIterator;
};

template<typename TK, typename TI>
class TMap : public TMapBase<TK,TI>{
public:
	TMap& operator=(const TMap& Other){
		TMapBase<TK,TI>::operator=(Other);

		return *this;
	}

	int Num(){
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

	void MultiFind(const TK& Key, TArray<TI>& Values){
		guardSlow(TMap::MultiFind);
		for(INT i=Hash[(GetTypeHash(Key) & (HashCount-1))]; i!=INDEX_NONE; i=Pairs[i].HashNext)
			if(Pairs[i].Key==Key)
				new(Values)TI(Pairs[i].Value);
		unguardSlow;
	}

	TI& Add(typename TTypeInfo<TK>::ConstInitType InKey, typename TTypeInfo<TI>::ConstInitType InValue){
		return TMapBase<TK,TI>::Add(InKey, InValue);
	}

	TI& AddUnique(typename TTypeInfo<TK>::ConstInitType InKey, typename TTypeInfo<TI>::ConstInitType InValue){
		for(INT i=Hash[(GetTypeHash(InKey) & (HashCount-1))]; i!=INDEX_NONE; i=Pairs[i].HashNext)
			if(Pairs[i].Key==InKey && Pairs[i].Value==InValue)
				return Pairs[i].Value;
		return Add(InKey, InValue);
	}

	INT RemovePair(typename TTypeInfo<TK>::ConstInitType InKey, typename TTypeInfo<TI>::ConstInitType InValue){
		guardSlow(TMap::Remove);
		INT Count = 0;
		for(INT i=Pairs.Num()-1; i >= 0; i--)
			if(Pairs[i].Key==InKey && Pairs[i].Value==InValue)
				{Pairs.Remove(i); Count++;}
		if(Count)
			Relax();
		return Count;
		unguardSlow;
	}

	TI* FindPair(const TK& Key, const TK& Value){
		guardSlow(TMap::Find);
		for(INT i=Hash[(GetTypeHash(Key) & (HashCount-1))]; i!=INDEX_NONE; i=Pairs[i].HashNext)
			if(Pairs[i].Key==Key && Pairs[i].Value==Value)
				return &Pairs[i].Value;
		return NULL;
		unguardSlow;
	}
};

/*----------------------------------------------------------------------------
	Sorting template.
----------------------------------------------------------------------------*/

//
// Sort elements. The sort is unstable, meaning that the ordering of equal 
// items is not necessarily preserved.
//
template<typename T>
struct TStack{
	T* Min;
	T* Max;
};

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
	TDoubleLinkedList.
----------------------------------------------------------------------------*/

//
// Simple double-linked list template.
//
template<typename T>
class TDoubleLinkedList : public T{
public:
	TDoubleLinkedList* Next;
	TDoubleLinkedList** PrevLink;

	void Unlink(){
		if(Next)
			Next->PrevLink = PrevLink;
		*PrevLink = Next;
	}

	void Link(TDoubleLinkedList*& Before){
		if(Before)
			Before->PrevLink = &Next;
		Next     = Before;
		PrevLink = &Before;
		Before   = this;
	}
};

/*----------------------------------------------------------------------------
	FRainbowPtr.
----------------------------------------------------------------------------*/

//
// A union of pointers of all base types.
//
union CORE_API FRainbowPtr{
	// All pointers.
	void*  PtrVOID;
	BYTE*  PtrBYTE;
	_WORD* PtrWORD;
	DWORD* PtrDWORD;
	QWORD* PtrQWORD;
	FLOAT* PtrFLOAT;

	// Conversion constructors.
	FRainbowPtr(){}
	FRainbowPtr(void* Ptr) : PtrVOID(Ptr){};
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/