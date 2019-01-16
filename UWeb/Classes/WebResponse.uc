/*=============================================================================
	WebResponse is used by WebApplication to handle most aspects of sending
	http information to the client. It serves as a bridge between WebApplication
	and WebConnection.
=============================================================================*/

class WebResponse extends Object
	native
	noexport;

var private native const int ReplacementMap[4];	// TMap<FString, FString>!
var const config string IncludePath;
var WebConnection Connection;
var bool bSentText; // used to warn headers already sent
var bool bSentResponse;

// uhtm including
native final function Subst(string Variable, string Value, optional bool bClear);
native final function ClearSubst();
native final function IncludeUHTM(string Filename);
native final function IncludeBinaryFile(string Filename);
native final function string LoadParsedUHTM(string Filename);		// For templated web items, uses Subst too
native final function string GetHTTPExpiration(optional int OffsetSeconds);

event SendText(string Text, optional bool bNoCRLF)
{
	if(!bSentText)
	{
		SendStandardHeaders();
		bSentText = True;
	}	

	if(bNoCRLF)
		Connection.SendText(Text);
	else
		Connection.SendText(Text$Chr(13)$Chr(10));
}

event SendBinary(int Count, byte B[255])
{
	Connection.SendBinary(Count, B);
}

function SendCachedFile(string Filename, optional string ContentType)
{
	if(!bSentText)
	{
		SendStandardHeaders(ContentType, true);
		bSentText = True;
	}	
	IncludeUHTM(Filename);
}

function FailAuthentication(string Realm)
{
	HTTPError(401, Realm);
}

function HTTPResponse(string Header)
{
	HTTPHeader(Header);
	bSentResponse = True;
}

function HTTPHeader(string Header)
{
	if(bSentText)
		Log("Can't send headers - already called SendText()");

//	Log("Sent:"@Header);
	Connection.SendText(Header$Chr(13)$Chr(10));
}

function HTTPError(int ErrorNum, optional string Data)
{
	switch(ErrorNum)
	{
	case 400:
		HTTPResponse("HTTP/1.1 400 Bad Request");
		SendText("<TITLE>400 Bad Request</TITLE><H1>400 Bad Request</H1>If you got this error from a standard web browser, please mail jack@epicgames.com and submit a bug report.");
		break;
	case 401:
		HTTPResponse("HTTP/1.1 401 Unauthorized");
		HTTPHeader("WWW-authenticate: basic realm=\""$Data$"\"");
		SendText("<TITLE>401 Unauthorized</TITLE><H1>401 Unauthorized</H1>");
		break;
	case 404:
		HTTPResponse("HTTP/1.1 404 Object Not Found");
		SendText("<TITLE>404 File Not Found</TITLE><H1>404 File Not Found</H1>The URL you requested was not found.");
		break;
	default:
		break;
	}
}

function SendStandardHeaders( optional string ContentType, optional bool bCache )
{
	if(ContentType == "")
		ContentType = "text/html";
	if(!bSentResponse)
		HTTPResponse("HTTP/1.1 200 OK");
	HTTPHeader("Server: UnrealEngine UWeb Web Server Build "$Connection.Level.EngineVersion);
	HTTPHeader("Content-Type: "$ContentType);
	if (bCache)
	{
		HTTPHeader("Cache-Control: max-age="$Connection.WebServer.ExpirationSeconds);
		// Need to compute an Expires: tag .... arrgggghhh
		HTTPHeader("Expires:"@GetHTTPExpiration(Connection.WebServer.ExpirationSeconds));
	}
	HTTPHeader("Connection: Close");
	HTTPHeader("");
}

function Redirect(string URL)
{
	HTTPResponse("HTTP/1.1 302 Document Moved");
	HTTPHeader("Location: "$URL);
	SendText("<head><title>Document Moved</title></head>");
	SendText("<body><h1>Object Moved</h1>This document may be found <a HREF=\""$URL$"\">here</a>.");
}

defaultproperties
{
	IncludePath="../Web"
}