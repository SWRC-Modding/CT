/*=============================================================================
	WebServer.cpp: Unreal Web Server
	Copyright 1997-2002 Epic Games, Inc. All Rights Reserved.

Revision history:
	* Created by Jack Porter
	* Modified by Michel Comeau
=============================================================================*/

#include "../Inc/UWeb.h"

// To help ANSI out.
#undef clock
#undef unclock

#include <time.h>
/*-----------------------------------------------------------------------------
	Declarations.
-----------------------------------------------------------------------------*/

static TMap<FString,FString>& CachedFileContent() // Caching unparsed special files - sjs modified for static linking
{
    static TMap<FString, FString>	CachedFileContent;
    return CachedFileContent;
}
FString		WebRootRealPath;				// Full path on system of web root
/*-----------------------------------------------------------------------------
	UWebRequest functions.
-----------------------------------------------------------------------------*/

//
// Decode a base64 encoded string - used for HTTP authentication
//
void UWebRequest::execDecodeBase64( FFrame& Stack, RESULT_DECL )
{
	guard(UWebRequest::execDecodeBase64);
	P_GET_STR(Encoded);
	P_FINISH;

	TCHAR *Decoded = (TCHAR *)appAlloca((Encoded.Len() / 4 * 3 + 1) * sizeof(TCHAR));
	check(Decoded);

	FString Base64Map(TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"));
	INT ch, i=0, j=0;
	TCHAR Junk[2] = {0, 0};
	TCHAR *Current = (TCHAR *)*Encoded;

    while((ch = (INT)(*Current++)) != '\0')
	{
		if (ch == '=')
			break;

		Junk[0] = ch;
		ch = Base64Map.InStr(FString(Junk));
		if( ch == -1 )
		{
			*(FString*)Result = FString(TEXT(""));
			return;
		}

		switch(i % 4) {
		case 0:
			Decoded[j] = ch << 2;
			break;
		case 1:
			Decoded[j++] |= ch >> 4;
			Decoded[j] = (ch & 0x0f) << 4;
			break;
		case 2:
			Decoded[j++] |= ch >>2;
			Decoded[j] = (ch & 0x03) << 6;
			break;
		case 3:
			Decoded[j++] |= ch;
			break;
		}
		i++;
	}

    /* clean up if we ended on a boundary */
    if (ch == '=') 
	{
		switch(i % 4)
		{
		case 0:
		case 1:
			*(FString*)Result = FString(TEXT(""));
			return;
		case 2:
			j++;
		case 3:
			Decoded[j++] = 0;
		}
	}
	Decoded[j] = '\0';
	*(FString*)Result = FString(Decoded);
	unguard;
}

void UWebRequest::execAddVariable( FFrame& Stack, RESULT_DECL )
{
	guard(WebRequest::execAddVariable);
	P_GET_STR(VariableName);
	P_GET_STR(Value);	
	P_FINISH;
	VariableMap.Add(*(VariableName.Caps()), *Value);
	unguard;
}

void UWebRequest::execGetVariable( FFrame& Stack, RESULT_DECL )
{
	guard(WebRequest::execGetVariable);
	P_GET_STR(VariableName);
	P_GET_STR_OPTX(DefaultValue, TEXT(""));
	P_FINISH;
	FString *S = VariableMap.Find(VariableName.Caps());
	if(S)
		*(FString*)Result = *S;
	else
		*(FString*)Result = DefaultValue;
	unguard;
}

void UWebRequest::execGetVariableCount( FFrame& Stack, RESULT_DECL )
{
	guard(WebRequest::execGetVariableCount);
	P_GET_STR(VariableName);
	P_FINISH;

	TArray<FString> List;
	VariableMap.MultiFind( VariableName.Caps(), List );
	*(INT *)Result = List.Num();
	unguard;
}

void UWebRequest::execGetVariableNumber( FFrame& Stack, RESULT_DECL )
{
	guard(UWebRequest::execGetVariableNumber);
	P_GET_STR(VariableName);
	P_GET_INT(Number);
	P_GET_STR_OPTX(DefaultValue, TEXT(""));
	P_FINISH;

	TArray<FString> List;
	VariableMap.MultiFind( VariableName.Caps(), List );
	if(Number >= List.Num())
		*(FString*)Result = DefaultValue;
	else
		*(FString*)Result = List[Number];
	unguard;
}

/*-----------------------------------------------------------------------------
	UWebResponse functions.
-----------------------------------------------------------------------------*/

#define UHTMPACKETSIZE 512
void UWebResponse::SendInParts( const FString &S )
{
	guard(UWebResponse::SendInParts);
	INT Pos = 0, L;
	L = S.Len();

	if(L <= UHTMPACKETSIZE)
	{
		if(L > 0)
			eventSendText(S, 1);
		return;
	}
	while( L - Pos > UHTMPACKETSIZE )
	{
		eventSendText(S.Mid(Pos, UHTMPACKETSIZE), 1);
		Pos += UHTMPACKETSIZE;
	}
	if(Pos > 0)
		eventSendText(S.Mid(Pos), 1);
	unguard;
}

// ValidWebFile takes a relative path and makes sure it is located under the WebRoot
// it makes use of / and .. legal.
bool UWebResponse::ValidWebFile(const FString &Filename)
{
FString olddir(GFileManager->GetDefaultDirectory());

	if( IncludePath == TEXT("") )
	{
		debugf( NAME_Log, TEXT("WebServer: Bad IncludePath: %s"), *IncludePath);//!!localize!!
		return false;
	}
	if ( WebRootRealPath == TEXT("") )
	{
		if (!GFileManager->SetDefaultDirectory(*IncludePath))
		{
			debugf( NAME_Log, TEXT("WebServer: Bad IncludePath: %s"), *IncludePath);
			return false;
		}
		WebRootRealPath = GFileManager->GetDefaultDirectory();
		GFileManager->SetDefaultDirectory(*olddir);
	}

	if (!GFileManager->SetDefaultDirectory(*Filename))
	{
	int p1, p2;

		p1 = Filename.InStr(TEXT("/"), true);
		p2 = Filename.InStr(TEXT("\\"), true);
		if (p1 < p2)
			p1 = p2;

		if (p1 == -1 || !GFileManager->SetDefaultDirectory(*(Filename.Left(p1))))
		{
			debugf( NAME_Log, TEXT("WebServer: Invalid Filename: %s"), *Filename);
			return false;
		}
	}

	FString fullpath(GFileManager->GetDefaultDirectory());
	GFileManager->SetDefaultDirectory(*olddir);

	if (fullpath.Len() < WebRootRealPath.Len() || fullpath.Left(WebRootRealPath.Len()) != WebRootRealPath)
	{
		debugf( NAME_Log, TEXT("WebServer: Filename not under web root: %s <-> %s"), *fullpath, *WebRootRealPath );
		return false;
	}
	return true;
}

void UWebResponse::execIncludeBinaryFile( FFrame& Stack, RESULT_DECL )
{
	guard(UWebResponse::execIncludeBinaryFile);
	P_GET_STR(Filename);
	P_FINISH;
	
	if ( !ValidWebFile(*(IncludePath * Filename)) )
		return;

	TArray<BYTE> Data;
	if( !appLoadFileToArray( Data, *(IncludePath * Filename)) )
	{
		debugf( NAME_Log, TEXT("WebServer: Unable to open include file %s%s%s"), *IncludePath, PATH_SEPARATOR, *Filename );//!!localize!!
		return;
	}
	for( INT i=0; i<Data.Num(); i += 255)
		eventSendBinary( Min<INT>(Data.Num()-i, 255), &Data[i] );

	unguard;
}

bool UWebResponse::IncludeTextFile(const FString &Root, const FString &Filename, bool bCache, FString *Result)
{
	guard(UWebResponse::IncludeTextFile);

	FString IncludeFile;
	if( Filename.Left(1) == TEXT("\\") || Filename.Left(1) == TEXT("/") )
		IncludeFile = *(IncludePath * Filename);
	else
		IncludeFile = *(IncludePath * Root * Filename);

	if ( !ValidWebFile(IncludeFile) )
		return false;

	FString Text, *PrevText = NULL;
	if (bCache)
	{
		PrevText = CachedFileContent().Find(IncludeFile);
		if (PrevText != NULL)
			Text = *PrevText;
	}

	if( PrevText == NULL && !appLoadFileToString( Text, *IncludeFile ) )
	{
		debugf( NAME_Log, TEXT("WebServer: Unable to open include file %s"), *IncludeFile );//!!localize!!
		return false;
	}

	// Add to Cache if it wasnt
	if (bCache && PrevText == NULL)
		CachedFileContent().Set( *IncludeFile, *Text);

	INT Pos = 0;
	TCHAR* T = const_cast<TCHAR*>( *Text );
	TCHAR* P;
	TCHAR* I;
	while( true )
	{
		P = appStrstr(T, TEXT("<%"));
		I = appStrstr(T, TEXT("<!--"));

		if (P == NULL && I == NULL)
			break;

		if (I == NULL || (P != NULL && P<I))
		{
			if (Result == NULL)
				SendInParts( Text.Mid(Pos, (P - T)) );
			else
				(*Result) += Text.Mid(Pos, (P - T));

			Pos += (P - T);
			T = P;

			guard(FindClosing);
			// Find the close percentage
			TCHAR *PEnd = appStrstr(P+2, TEXT("%>"));
			if(PEnd)
			{
				guard(PerformReplacement);
				FString Key = Text.Mid(Pos + (P - T) + 2, (PEnd - P) - 2);
				FString *V, Value;
				if(Key.Len() > 0)
				{
					V = ReplacementMap.Find(Key);
					if(V)
						Value = *V;
					else
						Value = TEXT("");
				}
  				if (Result == NULL)
					SendInParts(Value);
				else
					(*Result) += Value;

				Pos += (PEnd - P) + 2;
				T = PEnd + 2;
				unguard;
			}
			else
			{
				Pos++;
				T++;
			}
			unguard;
		}
		else
		{
			if (Result == NULL)
				SendInParts( Text.Mid(Pos, (I - T)) );
			else
				(*Result) += Text.Mid(Pos, (I - T));

			Pos += (I - T);
			T = I;

			//debugf( NAME_Log, TEXT("Found Comment Marker"));//!!localize!!

			guard(FindClosing);
			// Find the close tag
			TCHAR *IEnd = appStrstr(I+4, TEXT("-->"));
			if(IEnd)
			{
				bool bIncluded = false;
				guard(checkInclude);
				//debugf( NAME_Log, TEXT("Found End Comment Marker"));//!!localize!!
				T += 4;
				// the code is <!-- #include file="filename.ext" -->
				// Skip any leading white space
				while (*T == ' ' || *T == '\t' || *T == '\r' || *T == '\n') T++;

				//debugf( NAME_Log, TEXT("Next Marker: '%s'"), *(Text.Mid((Pos), 9)));//!!localize!!

				if (Text.Mid((Pos + T - I), 9) == TEXT("#include "))
				{
					T += 9;

					//debugf( NAME_Log, TEXT("Found #include"));//!!localize!!

					// Skip any leading white space
					while (*T == ' ' || *T == '\t' || *T == '\r' || *T == '\n') T++;

					if (Text.Mid((Pos + T - I), 5) == TEXT("file="))
					{
						T += 5;
						//debugf( NAME_Log, TEXT("Found 'file='"));
						while (*T == ' ' || *T == '\t' || *T == '\r' || *T == '\n') T++;
						
						if (*T == '\'' || *T == '"')
						{
							TCHAR c = *T;
							TCHAR *U = appStrchr(T+1, c);
							if (U != NULL && (U-T-1) > 0)
							{
								//debugf( NAME_Log, TEXT("WebServer: Including File '%s'"), *(Root * Text.Mid((Pos + T - I + 1), U-T-1)) );//!!localize!!
								bIncluded = IncludeTextFile(Root, Text.Mid((Pos + T - I + 1), U-T-1));
							}
						}
					}
				}
				// Send the text if it the include file was not found
				if (!bIncluded)
				{
					if (Result == NULL)
						SendInParts(Text.Mid(Pos, (IEnd - I) + 3));
					else
						(*Result) += Text.Mid(Pos, (IEnd - I) + 3);
				}

				Pos += (IEnd - I) + 3;
				T = IEnd + 3;
				
				unguard;
			}
			unguard;
		}
	}
	if (Result == NULL)
		SendInParts(Text.Mid(Pos));
	else
		(*Result) += Text.Mid(Pos);

	return true;
	unguard;
}

void UWebResponse::execIncludeUHTM( FFrame& Stack, RESULT_DECL )
{
	guard(UWebResponse::execIncludeUHTM);
	P_GET_STR(Filename);
	P_FINISH;

	// Find the root path of filename
	FString Root;
	int p1, p2;

	p1 = Filename.InStr(TEXT("/"), true);
	p2 = Filename.InStr(TEXT("\\"), true);
	if (p1<p2)
		p1 = p2;

	if (p1 != -1)
	{
	  Root = Filename.Left(p1);
	  Filename = Filename.Mid(p1 + 1);
	}
	else
	  Root = TEXT("");

//	debugf( NAME_Log, TEXT("WebServer: Root of File is '%s'"), *Root );//!!localize!!

	IncludeTextFile(Root, Filename);

	unguard;
}

void UWebResponse::execLoadParsedUHTM( FFrame& Stack, RESULT_DECL )
{
	guard(UWebResponse::execLoadParsedUHTM);
	P_GET_STR(Filename);
	P_FINISH;

	// Find the root path of filename
	FString Root;
	int p1, p2;

	p1 = Filename.InStr(TEXT("/"), true);
	p2 = Filename.InStr(TEXT("\\"), true);
	if (p1<p2)
		p1 = p2;

	if (p1 != -1)
	{
	  Root = Filename.Left(p1);
	  Filename = Filename.Mid(p1 + 1);
	}
	else
	  Root = TEXT("");

	IncludeTextFile(Root, Filename, false, (FString*)Result);

	unguard;
}

void UWebResponse::execClearSubst( FFrame& Stack, RESULT_DECL )
{
	guard(UWebResponse::execClearSubst);
	P_FINISH;
	ReplacementMap.Empty();
	unguard;
}

void UWebResponse::execSubst( FFrame& Stack, RESULT_DECL )
{
	guard(UWebResponse::execSubst);
	P_GET_STR(Variable);
	P_GET_STR(Value);
	P_GET_UBOOL_OPTX(bClear, 0);
	P_FINISH;

	if(bClear)
		ReplacementMap.Empty();
	ReplacementMap.Set( *Variable, *Value );
	unguard;
}

void UWebResponse::execGetHTTPExpiration( FFrame& Stack, RESULT_DECL )
{
	guard(UWebResponse::execGetHTTPExpiration);
	P_GET_INT_OPTX(OffsetSeconds, 86400);
	P_FINISH;
	// Format GMT Time as "dd mmm yyyy hh:mm:ss GMT";
	struct tm *newtime;
	time_t ltime;

	TCHAR GMTRef[100];
	TCHAR *Months[12] = { TEXT("Jan"), TEXT("Feb"), TEXT("Mar"), TEXT("Apr"),
						  TEXT("May"), TEXT("Jun"), TEXT("Jul"), TEXT("Aug"),
						  TEXT("Sep"), TEXT("Oct"), TEXT("Nov"), TEXT("Dec") };

	time( &ltime );
	ltime += OffsetSeconds;
	newtime = gmtime( &ltime );
	appSprintf(GMTRef, TEXT("%02d %3s %04d %02d:%02d:%02d GMT"),
						newtime->tm_mday, Months[newtime->tm_mon], newtime->tm_year + 1900,
						newtime->tm_hour, newtime->tm_min, newtime->tm_sec);

	*(FString *) Result = FString(GMTRef);
	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

