/*=============================================================================
	UWeb.cpp: Unreal Webserver Implementation
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

Revision history:
	* Created by Jack Porter.
=============================================================================*/

#include "../Inc/UWeb.h"

/*-----------------------------------------------------------------------------
	Declarations.
-----------------------------------------------------------------------------*/

// Package.
IMPLEMENT_PACKAGE(UWeb);

// Register things.

IMPLEMENT_CLASS(UWebResponse);
FNativeEntry<UWebResponse> UWebResponse::StaticNativeMap[] =
{
    MAP_NATIVE(Subst, 0)
    MAP_NATIVE(ClearSubst, 0)
    MAP_NATIVE(IncludeUHTM, 0)
	MAP_NATIVE(IncludeBinaryFile, 0)
    MAP_NATIVE(LoadParsedUHTM, 0)
    MAP_NATIVE(GetHTTPExpiration, 0)
	{NULL, NULL}
};
LINK_NATIVES(UWebResponse);

IMPLEMENT_CLASS(UWebRequest);
FNativeEntry<UWebRequest> UWebRequest::StaticNativeMap[] =
{
	MAP_NATIVE(GetVariableNumber, 0)
	MAP_NATIVE(GetVariableCount, 0)
	MAP_NATIVE(GetVariable, 0)
	MAP_NATIVE(AddVariable, 0)
	MAP_NATIVE(DecodeBase64, 0)
	{NULL, NULL}
};
LINK_NATIVES(UWebRequest);

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
