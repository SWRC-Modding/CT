#pragma once

#include "Engine.h"
#include "GameSpyMgr.h"

#ifndef IPDRV_API
#define IPDRV_API DLL_IMPORT
LINK_LIB(IpDrv)
#endif

#if SUPPORTS_PRAGMA_PACK
#pragma pack(push,4)
#endif

/*
 * MasterServerLink
 */

class IPDRV_API AMasterServerLink : public AInfo{
	DECLARE_CLASS(AMasterServerLink,AInfo,CLASS_Transient|CLASS_Config,IpDrv)
	NO_DEFAULT_CONSTRUCTOR(AMasterServerLink)

	INT LinkPtr;
	INT LANPort;
	INT LANServerPort;
	INT CurrentMasterServer;
	INT MasterServerPort[5];
	FStringNoInit MasterServerAddress[5];

	virtual UBOOL Poll( INT WaitTime ) { return 0; }

	void GetMasterServer(FString& OutAddress, INT& OutPort);
};

/*
 * MasterServerUplink
 */

class IPDRV_API AMasterServerUplink : public AMasterServerLink{
	DECLARE_CLASS(AMasterServerUplink,AMasterServerLink,CLASS_Transient|CLASS_Config,IpDrv)
	NO_DEFAULT_CONSTRUCTOR(AMasterServerUplink)

	FServerResponseLine ServerState;
	class AMasterServerGameStats* GameStats;
	INT CheckCDKey;
	INT AdvertiseServer;

	void Destroy();
	void PostScriptDestroyed();
	UBOOL Poll(INT WaitTime);
};

inline void SetDefaultMasterServerAddress()
{
	debugf("Getting default master server address from IpDrv.MasterServerLink");

	UClass*                  Class  = LoadClass<AMasterServerLink>(NULL, "IpDrv.MasterServerUplink", NULL, LOAD_NoFail, NULL);
	const AMasterServerLink* MSLink = static_cast<AMasterServerLink*>(Class->GetDefaultActor());
	const INT                Index  = MSLink->CurrentMasterServer;

	if(Index >= 0 && Index < ARRAY_COUNT(MSLink->MasterServerAddress) && MSLink->MasterServerAddress[Index].Len() > 0)
		SetGameSpyMasterServerAddress(*MSLink->MasterServerAddress[MSLink->CurrentMasterServer]);
	else
		debugf("No master server address set at IpDrv.MasterServerLink[%i]", MSLink->CurrentMasterServer);
}

inline GameSpyMgr* GetGameSpyMgr()
{
	const DWORD IpDrvBase = (DWORD)GetModuleHandleA("IpDrv.dll");
	if(IpDrvBase)
		return (GameSpyMgr*)(IpDrvBase + 0x0002AF2C);

	return NULL;
}

#if SUPPORTS_PRAGMA_PACK
#pragma pack(pop)
#endif
