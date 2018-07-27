/*=============================================================================
	UnPlayer.h: Unreal player class.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	UPlayer.
-----------------------------------------------------------------------------*/

//
// A player, the base class of UViewport (local players) and UNetConnection (remote players).
//
class ENGINE_API UPlayer : public UObject, public FOutputDevice, public FExec{
	DECLARE_ABSTRACT_CLASS(UPlayer,UObject,CLASS_Transient|CLASS_Config,Engine)

	//Objects
	class APlayerController* Actor;
	class UConsole* Console;

	BITFIELD bWindowsMouseAvailable:1;
	BITFIELD bShowWindowsMouse:1;
	BITFIELD bSuspendPrecaching:1;
	FLOAT WindowsMouseX;
	FLOAT WindowsMouseY;
	INT CurrentNetSpeed;
	INT ConfiguredInternetSpeed;
	INT ConfiguredLanSpeed;
	BYTE GamePadIndex;
	BYTE SplitIndex;
	BYTE SelectedCursor;
	class UInteractionMaster* InteractionMaster;
	TArrayNoInit<class UInteraction*> LocalInteractions;

	//Constructor
	UPlayer();

	//Overrides
	virtual void Destroy();
	virtual int Exec(const char*, FOutputDevice&);
	virtual void Serialize(FArchive&);
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/