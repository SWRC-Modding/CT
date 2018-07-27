//=============================================================================
// AccessControl.
//
// AccessControl is a helper class for GameInfo.
// The AccessControl class determines whether or not the player is allowed to 
// login in the PreLogin() function, and also controls whether or not a player 
// can enter as a spectator or a game administrator.
//
//=============================================================================
class AccessControl extends Info;

var globalconfig string     IPPolicies[50];
var	localized string          IPBanned;
var	localized string	      WrongPassword;
var	localized string          NeedPassword;
var class<Admin> AdminClass;

var private globalconfig string AdminPassword;	    // Password to receive bAdmin privileges.
var private globalconfig string GamePassword;		    // Password to enter game.

var bool bDontAddDefaultAdmin;


function SetAdminPassword(string P)
{
	AdminPassword = P;
}

function SetGamePassword(string P)
{
	GamePassword = P;
}

function bool RequiresPassword()
{
	return GamePassword != "";
}

function Kick( string S ) 
{
	local PlayerController P;

	ForEach DynamicActors(class'PlayerController', P)
		if ( P.PlayerReplicationInfo.PlayerName~=S 
			&&	(NetConnection(P.Player)!=None) )
		{
			P.Destroy();
			return;
		}
}

function KickBan( string S ) 
{
	local PlayerController P;
	local string IP;
	local int j;

	ForEach DynamicActors(class'PlayerController', P)
		if ( P.PlayerReplicationInfo.PlayerName~=S 
			&&	(NetConnection(P.Player)!=None) )
		{
			IP = P.GetPlayerNetworkAddress();
			if( CheckIPPolicy(IP) )
			{
				IP = Left(IP, InStr(IP, ":"));
				Log("Adding IP Ban for: "$IP);
				for(j=0;j<50;j++)
					if( IPPolicies[j] == "" )
						break;
				if(j < 50)
					IPPolicies[j] = "DENY,"$IP;
				SaveConfig();
			}
			P.Destroy();
			return;
		}
}

function bool AdminLogin( PlayerController P, string Password )
{
	if (AdminPassword == "")
		return false;

	if (Password == AdminPassword)
	{
		Log("Administrator logged in.");
		Level.Game.Broadcast( P, P.PlayerReplicationInfo.PlayerName$"logged in as a server administrator." );
		return true;
	}
	return false;
}

//
// Accept or reject a player on the server.
// Fails login if you set the Error to a non-empty string.
//
event PreLogin
(
	string Options,
	string Address,
	out string Error,
	out string FailCode,
	bool bSpectator
)

{
	// Do any name or password or name validation here.
	local string InPassword;
	local bool   bAdmin;
	local bool   bInvited;

	Error="";
	InPassword = Level.Game.ParseOption( Options, "Password" );

//--Live FIXME
//	bAdmin = CheckOptionsAdmin(Options);
	bAdmin = false;
    bInvited = bAdmin || bool(Level.Game.ParseOption( Options, "WasInvited" ) );
	if( (Level.NetMode != NM_Standalone) && !bAdmin && Level.Game.AtCapacity(bSpectator,bInvited) )
	{
		Error=Level.Game.GameMessageClass.Default.MaxedOutMessage;
	}
	else if
	(	GamePassword!=""
	&&	caps(InPassword)!=caps(GamePassword)
	&&	(AdminPassword=="" || caps(InPassword)!=caps(AdminPassword)) )
	{
		if( InPassword == "" )
		{
			Error = NeedPassword;
			FailCode = "NEEDPW";
		}
		else
		{
			Error = WrongPassword;
			FailCode = "WRONGPW";
		}
	}

	if(!CheckIPPolicy(Address))
		Error = IPBanned;
}


function bool CheckIPPolicy(string Address)
{
	local int i, j, LastMatchingPolicy;
	local string Policy, Mask;
	local bool bAcceptAddress, bAcceptPolicy;
	
	// strip port number
	j = InStr(Address, ":");
	if(j != -1)
		Address = Left(Address, j);

	bAcceptAddress = True;
	for(i=0; i<50 && IPPolicies[i] != ""; i++)
	{
		j = InStr(IPPolicies[i], ",");
		if(j==-1)
			continue;
		Policy = Left(IPPolicies[i], j);
		Mask = Mid(IPPolicies[i], j+1);
		if(Policy ~= "ACCEPT") 
			bAcceptPolicy = True;
		else
		if(Policy ~= "DENY") 
			bAcceptPolicy = False;
		else
			continue;

		j = InStr(Mask, "*");
		if(j != -1)
		{
			if(Left(Mask, j) == Left(Address, j))
			{
				bAcceptAddress = bAcceptPolicy;
				LastMatchingPolicy = i;
			}
		}
		else
		{
			if(Mask == Address)
			{
				bAcceptAddress = bAcceptPolicy;
				LastMatchingPolicy = i;
			}
		}
	}

	if(!bAcceptAddress)
		Log("Denied connection for "$Address$" with IP policy "$IPPolicies[LastMatchingPolicy]);
		
	return bAcceptAddress;
}


defaultproperties
{
     IPPolicies(0)="ACCEPT,*"
     IPBanned="Your IP address has been banned on this server."
     WrongPassword="The password you entered is incorrect."
     NeedPassword="You need to enter a password to join this game."
     AdminClass=Class'Engine.Admin'
}

