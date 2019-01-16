/*=============================================================================
	WebConnection is the bridge that will handle all communication between
	the web server and the client's browser.
=============================================================================*/

class WebConnection extends TcpLink;

var WebServer WebServer;
var string ReceivedData;

var WebRequest Request;
var WebResponse Response;
var WebApplication Application;

var int RawBytesExpecting;
// MC: Debug 
// var int ConnId;

event Accepted()
{
	WebServer = WebServer(Owner);
	SetTimer(30, False);
//	ConnId = WebServer.ConnId++;
//	Log("Connection"@ConnId@"Accepted");
}

event Closed()
{
//	Log("Connection"@ConnId@"Closed");
	Destroy();
}

event Timer()
{
	Close();
}

event ReceivedText( string Text )
{
	local int i;
	local string S;

	ReceivedData = ReceivedData $ Text;
	if(RawBytesExpecting > 0)
	{
		RawBytesExpecting -= Len(Text);
		CheckRawBytes();

		return;
	}

	// remove a LF which arrived in a new packet
	// and thus didn't get cleaned up by the code below
	if(Left(ReceivedData, 1) == Chr(10))
		ReceivedData = Mid(ReceivedData, 1);
	i = InStr(ReceivedData, Chr(13));
	while(i != -1)
	{
		S = Left(ReceivedData, i);
		i++;
		// check for any LF following the CR.
		if(Mid(ReceivedData, i, 1) == Chr(10))
			i++;

		ReceivedData = Mid(ReceivedData, i);

		ReceivedLine(S);
		
		if(LinkState != STATE_Connected)
			return;
		if(RawBytesExpecting > 0)
		{	
			CheckRawBytes();
			return;
		}

		i = InStr(ReceivedData, Chr(13));
	}
}

function ReceivedLine(string S)
{
	if (S == "") EndOfHeaders();
	else
	{
//		Log(S);
		if(Left(S, 4) ~= "GET ")		ProcessGet(S);
		else if(Left(S, 5) ~= "POST ")	ProcessPost(S);
		else if(Left(S, 5) ~= "HEAD ")  ProcessHead(S);
		else if(Request != None)
		{
			Request.ProcessHeaderString(S);
		}
	}
}

function ProcessHead(string S)
{
//	Log("Received: "$S);
}

function ProcessGet(string S)
{
	local int i;

	if(Request == None)
		CreateResponseObject();

	Request.RequestType = Request_GET;
	S = Mid(S, 4);
	while(Left(S, 1) == " ")
		S = Mid(S, 1);

	i = InStr(S, " ");

	if(i != -1)
		S = Left(S, i);

	i = InStr(S, "?");
	if(i != -1)
	{
		Request.DecodeFormData(Mid(S, i+1));
		S = Left(S, i);
	}

	Application = WebServer.GetApplication(S, Request.URI);
	if(Application != None && Request.URI == "")
	{
		Response.Redirect(WebServer.ServerURL$S$"/");
		Cleanup();
	}
	else
	if(Application == None && Webserver.DefaultApplication != -1)
	{
		Response.Redirect(WebServer.ServerURL$Webserver.ApplicationPaths[Webserver.DefaultApplication]$"/");
		Cleanup();
	}
}

function ProcessPost(string S)
{
	local int i;

	if(Request == None)
		CreateResponseObject();

	Request.RequestType = Request_POST;

	S = Mid(S, 5);
	while(Left(S, 1) == " ")
		S = Mid(S, 1);

	i = InStr(S, " ");

	if(i != -1)
		S = Left(S, i);

	i = InStr(S, "?");
	if(i != -1)
	{
		Request.DecodeFormData(Mid(S, i+1));
		S = Left(S, i);
	}
	Application = WebServer.GetApplication(S, Request.URI);
	if(Application != None && Request.URI == "")
	{
		Response.Redirect(WebServer.ServerURL$S$"/");
		Cleanup();
	}
}

function CreateResponseObject()
{
	Request = new(None) class'WebRequest';

	Response = new(None) class'WebResponse';
	Response.Connection = Self;
}

function EndOfHeaders()
{
	if(Response == None)
	{
		CreateResponseObject();
		Response.HTTPError(400); // Bad Request
		Cleanup();
		return;
	}

	if(Application == None)
	{
		Response.HTTPError(404); // FNF
		Cleanup();
		return;
	}

	if(Request.ContentLength != 0 && Request.RequestType == Request_POST)
	{
		RawBytesExpecting = Request.ContentLength;
		RawBytesExpecting -= Len(ReceivedData);
		CheckRawBytes();
	}
	else
	{
		if (Application.PreQuery(Request, Response))
		{
			Application.Query(Request, Response);
			Application.PostQuery(Request, Response);
		}
		Cleanup();
	}
}

function CheckRawBytes()
{
	if(RawBytesExpecting <= 0)
	{
		if(!(Request.ContentType ~= "application/x-www-form-urlencoded"))
		{
			Log("WebConnection: Unknown form data content-type: "$Request.ContentType);
			Response.HTTPError(400); // Can't deal with this type of form data
		}
		else
		{
			Request.DecodeFormData(ReceivedData);
			if (Application.PreQuery(Request, Response))
			{
			  Application.Query(Request, Response);
			  Application.PostQuery(Request, Response);
			}
			ReceivedData = "";
		}
		Cleanup();
	}
}

function Cleanup()
{
	if(Request != None)
		Request = None;

	if(Response != None)
	{
		Response.Connection = None;
		Response = None;
	}

	if(Application != None)
		Application = None;

	Close();
}
