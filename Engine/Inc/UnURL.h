/*=============================================================================
	UnURL.h: Unreal URL class.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	FURL.
-----------------------------------------------------------------------------*/

//
// A uniform resource locator.
//
class ENGINE_API FURL
{
public:
	// URL components.
	FString Protocol;	// Protocol, i.e. "unreal" or "http".
	FString Host;		// Optional hostname, i.e. "204.157.115.40" or "unreal.epicgames.com", blank if local.
	INT		Port;       // Optional host port.
	FString Map;		// Map name, i.e. "SkyCity", default is "Index".
	TArray<FString> Op;	// Options.
	FString Portal;		// Portal to enter through, default is "".

	// Status.
	UBOOL	Valid;		// Whether parsed successfully.

	// Statics.
	static FString DefaultProtocol;
	static FString DefaultProtocolDescription;
	static FString DefaultName;
	static FString DefaultMap;
	static FString DefaultLocalMap;
	static FString DefaultHost;
	static FString DefaultPortal;
	static FString DefaultMapExt;
	static FString DefaultSaveExt;
	static INT DefaultPort;

	// Constructors.
	FURL( const TCHAR* Filename=NULL );
	FURL( FURL* Base, const TCHAR* TextURL, ETravelType Type );
	static void StaticInit();
	static void StaticExit();

	// Functions.
	UBOOL IsInternal() const;
	UBOOL IsLocalInternal() const;
	UBOOL HasOption( const TCHAR* Test ) const;
	const TCHAR* GetOption( const TCHAR* Match, const TCHAR* Default ) const;
	void LoadURLConfig( const TCHAR* Section, const TCHAR* Filename=NULL );
	void SaveURLConfig( const TCHAR* Section, const TCHAR* Item, const TCHAR* Filename=NULL ) const;
	void AddOption( const TCHAR* Str );
	FString String( UBOOL FullyQualified=0 ) const;
	ENGINE_API friend FArchive& operator<<( FArchive& Ar, FURL& U );

	// Operators.
	UBOOL operator==( const FURL& Other ) const;
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
