/*=============================================================================
	UWebPrivate.h: Unreal Web server
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

Revision history:
	* Created by Jack Porter
	* Modified by Michel Comeau
=============================================================================*/

#include "../../Engine/Inc/Engine.h"

#ifndef UWEB_API
#define UWEB_API DLL_IMPORT
#endif // !UWEB_API

/*-----------------------------------------------------------------------------
	Public includes.
-----------------------------------------------------------------------------*/

struct UWebResponse_eventSendText_Parms
{
    FString Text;
    BITFIELD bNoCRLF;
};
struct UWebResponse_eventSendBinary_Parms
{
	INT Count;
	BYTE B[255];
};
class UWEB_API UWebResponse : public UObject
{
private:
	void SendInParts(const FString &S);
	bool IncludeTextFile(const FString &Root, const FString &Filename, bool bCache=false, FString *Result = NULL);
	bool ValidWebFile(const FString &Filename);
public:
	TMap<FString, FString>	ReplacementMap;
	FStringNoInit			IncludePath;
    class AWebConnection*	Connection;
    BITFIELD				bSentText:1;
    BITFIELD				bSentResponse:1;
	DECLARE_FUNCTION(execIncludeBinaryFile);
    DECLARE_FUNCTION(execIncludeUHTM);
    DECLARE_FUNCTION(execLoadParsedUHTM);
    DECLARE_FUNCTION(execClearSubst);
    DECLARE_FUNCTION(execSubst);
	// MC: Temporary ?
	DECLARE_FUNCTION(execGetHTTPExpiration);
	// ----
    void eventSendText(const FString& Text, BITFIELD bNoCRLF)
    {
        UWebResponse_eventSendText_Parms Parms;
        Parms.Text=Text;
        Parms.bNoCRLF=bNoCRLF;
        ProcessEvent(FindFunctionChecked(NAME_SendText),&Parms);
    }
    void eventSendBinary(INT Count, BYTE* B)
    {
        UWebResponse_eventSendBinary_Parms Parms;
        Parms.Count=Count;
        appMemcpy(&Parms.B[0], B, Count);
        ProcessEvent(FindFunctionChecked(NAME_SendBinary),&Parms);
    }
    DECLARE_CLASS(UWebResponse,UObject,0,UWeb)
    NO_DEFAULT_CONSTRUCTOR(UWebResponse)
	DECLARE_NATIVES(UWebResponse)
};

/*-----------------------------------------------------------------------------
	UWebRequest.
-----------------------------------------------------------------------------*/

enum ERequestType
{
    Request_GET             =0,
    Request_POST            =1,
    Request_MAX             =2,
};

class UWEB_API UWebRequest : public UObject
{
public:
    FStringNoInit	URI;
    FStringNoInit	Username;
    FStringNoInit	Password;
    INT				ContentLength;
    FStringNoInit	ContentType;
    BYTE			RequestType;
    TMultiMap<FString, FString>	VariableMap;
    DECLARE_FUNCTION(execGetVariableNumber);
    DECLARE_FUNCTION(execGetVariableCount);
    DECLARE_FUNCTION(execGetVariable);
    DECLARE_FUNCTION(execAddVariable);
    DECLARE_FUNCTION(execDecodeBase64);
    DECLARE_CLASS(UWebRequest,UObject,0,UWeb)
    NO_DEFAULT_CONSTRUCTOR(UWebRequest)
	DECLARE_NATIVES(UWebRequest)
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
