//
// FOutputDevice
//

inline void FOutputDevice::Log(EName Event, const TCHAR* Str){
	if(!FName::SafeSuppressed(Event))
		Serialize(Str, Event);
}

inline void FOutputDevice::Log(const TCHAR* Str){
	if(!FName::SafeSuppressed(NAME_Log))
		Serialize(Str, NAME_Log);
}

inline void FOutputDevice::Log(const FString& S){
	if(!FName::SafeSuppressed(NAME_Log))
		Serialize(*S, NAME_Log);
}

inline void FOutputDevice::Log(enum EName Type, const FString& S){
	if(!FName::SafeSuppressed(Type))
		Serialize(*S, Type);
}

inline void FOutputDevice::Logf(EName Event, const TCHAR* Fmt, ...){
	if(!FName::SafeSuppressed(Event)){
		TCHAR Buffer[1024];

		GET_VARARGS(Buffer, sizeof(Buffer), Fmt);
		Serialize(Buffer, Event);
	}
}

inline void FOutputDevice::Logf(const TCHAR* Fmt, ...){
	if(!FName::SafeSuppressed(NAME_Log)){
		TCHAR Buffer[1024];

		GET_VARARGS(Buffer, sizeof(Buffer), Fmt);
		Serialize(Buffer, NAME_Log);
	}
}