#pragma once

#include "Engine.h"

#ifndef GAMESPYMGR_API
#define GAMESPYMGR_API DLL_IMPORT
LINK_LIB(GameSpyMgr)
#endif

#if SUPPORTS_PRAGMA_PACK
#pragma pack(push,4)
#endif

/*
 * GameSpyCDKeyResponseInterface
 */

class GAMESPYMGR_API GameSpyCDKeyResponseInterface : public CDKeyResponseInterface{
public:
	GameSpyCDKeyResponseInterface();
	GameSpyCDKeyResponseInterface(const GameSpyCDKeyResponseInterface& Other);
	~GameSpyCDKeyResponseInterface();
	GameSpyCDKeyResponseInterface& operator=(const GameSpyCDKeyResponseInterface& Other);

	// CDKeyResponseInterface interface
	virtual void GetResponse(char* Challenge, char* CDKey, char* Response);
};

/*
 * GameSpyMgr
 */

class GAMESPYMGR_API GameSpyMgr{
public:
	GameSpyMgr(const GameSpyMgr& Other);
	GameSpyMgr(UEngine*, int);
	~GameSpyMgr();
	GameSpyMgr& operator=(const GameSpyMgr& Other);

	void AuthCDKey(int, unsigned int, FString, FString);
	void BeginServerQuery();
	void CancelServerQuery();
	void ClearServerQueryResults();
	bool CreateServerBrowser();
	void DeleteServerBrowser();
	int Exec(const char*, FOutputDevice&);
	FString GetExtPlayerQueryResult(int, int);
	FString GetExtServerQueryResult(int);
	int GetLocalID();
	int GetServerQueryNumResults() const;
	FString GetServerQueryResult(int, int);
	bool IsBackEndAvailable();
	void RegisterCustomKeys();
	bool RegisterGame(int);
	void ReleaseAllCDKey();
	void ReleaseCDKey(int);
	void __cdecl SetUplinkCB(void(__cdecl*)(int, bool, void*), void*);
	void Tick(float);
	void UnRegisterGame();
	void UpdateState();

	static void __cdecl CDAuthCallback(int, int, int, char*, void*);
	static void __cdecl SBCallback(struct _ServerBrowser*, enum SBCallbackReason, struct _SBServer*, void*);
	static void __cdecl adderror_callback(enum qr2_error_t, char*, void*);
	static void __cdecl cm_callback(char*, int, void*);
	static int __cdecl count_callback(enum qr2_key_type, void*);
	static void __cdecl keylist_callback(enum qr2_key_type, struct qr2_keybuffer_s*, void*);
	static void __cdecl nn_callback(int, void*);
	static void __cdecl playerkey_callback(int, int, struct qr2_buffer_s*, void*);
	static void __cdecl serverkey_callback(int, struct qr2_buffer_s*, void*);
	static void __cdecl teamkey_callback(int, int, struct qr2_buffer_s*, void*);

protected:
	GameSpyMgr();

private:
	char Padding1[24]; // Padding
	FString Idk;
	char Padding2[10]; // Padding
};

inline void SetGameSpyMasterServerAddress(const TCHAR* Address)
{
	guardFunc;
	check(Address);

	static TCHAR Buffer[64]; // Max size of the buffer in GameSpyMgr.dll
	const INT Len = appStrlen(Address);

	if(Len >= ARRAY_COUNT(Buffer))
		appErrorf("Master server address must not be longer than %d characters", ARRAY_COUNT(Buffer));

	appStrcpy(Buffer, Address);

	const DWORD GameSpyMgrDllBase = (DWORD)GetModuleHandleA("GameSpyMgr.dll");
	check(GameSpyMgrDllBase);

	// SBOverrideMasterServer: Used by the client to fetch the server list
	*reinterpret_cast<const char**>(GameSpyMgrDllBase + 0x00011F58) = Buffer;
	// qr2_hostname: Used by the server to register itself
	appStrcpy(reinterpret_cast<char*>(GameSpyMgrDllBase + 0x0011F60), Buffer);

	unguard;
}

#if SUPPORTS_PRAGMA_PACK
#pragma pack(pop)
#endif
