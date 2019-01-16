class WebRequest extends Object
	native
	noexport;

enum ERequestType
{
	Request_GET,
	Request_POST
};

var string URI;
var string Username;
var string Password;
var int ContentLength;
var string ContentType;
var ERequestType RequestType;
var private native const int VariableMap[4];	// TMultiMap<FString, FString>!

native final function string DecodeBase64(string Encoded);
native final function AddVariable(string VariableName, string Value);
native final function string GetVariable(string VariableName, optional string DefaultValue);
native final function int GetVariableCount(string VariableName);
native final function string GetVariableNumber(string VariableName, int Number, optional string DefaultValue);

function ProcessHeaderString(string S)
{
	local int i;

	if(Left(S, 21) ~= "Authorization: Basic ")
	{
		S = DecodeBase64(Mid(S, 21));
		i = InStr(S, ":");
		if(i != -1)
		{
			Username = Left(S, i);
			Password = Mid(S, i+1);
		}
	}
	else
	if(Left(S, 16) ~= "Content-Length: ")
		ContentLength = Int(Mid(S, 16));
	else
	if(Left(S, 14) ~= "Content-Type: ")
		ContentType = Mid(S, 14);
}

function DecodeFormData(string Data)
{
	local string Token[2], ch;
	local int i;
	local int t;

	t = 0;
	for(i=0;i<Len(Data);i++)
	{
		ch = mid(Data, i, 1);
		switch(ch)
		{
		case "+":
			Token[t] = Token[t]$" ";
			break;
		case "&":
		case "?":
			if(Token[0] != "")
				AddVariable(Token[0], Token[1]);			
			Token[0] = "";
			Token[1] = "";
			t = 0;
			break;
		case "=":
			if(t == 0)
				t = 1;
			else
				Token[t] = Token[t]$"=";
			break;
		case "%":
			Token[t] = Token[t]$Chr(16 * GetHexDigit(mid(Data, ++i, 1)) + GetHexDigit(mid(Data, ++i, 1)));
			break;
		default:
			Token[t] = Token[t]$ch;
		}
	}

	if(Token[0] != "")
		AddVariable(Token[0], Token[1]);
}

function int GetHexDigit(string D)
{
	switch(caps(D))
	{
	case "0": return 0;
	case "1": return 1;
	case "2": return 2;
	case "3": return 3;
	case "4": return 4;
	case "5": return 5; 
	case "6": return 6; 
	case "7": return 7; 
	case "8": return 8; 
	case "9": return 9; 
	case "A": return 10; 
	case "B": return 11; 
	case "C": return 12; 
	case "D": return 13; 
	case "E": return 14; 
	case "F": return 15; 
	}

	return 0;
}
