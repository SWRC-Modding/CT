class AdminAccessControl extends AccessControl config(ModMPGame);

var protected config string AdminPassword; // Override because the base class property is private

function PostBeginPlay(){
	SaveConfig(); // Generating ini entry if it wasn't there already for convenience
}

function SetAdminPassword(string NewPassword){
	AdminPassword = NewPassword;
	super.SetAdminPassword(NewPassword);
	SaveConfig();
}

function bool AdminLogin(PlayerController PC, string Password){
	if(AdminPassword == "")
		return false;

	if(Password == AdminPassword){
		PC.PlayerReplicationInfo.bAdmin = true;
		Log("Administrator logged in (" $ PC.PlayerReplicationInfo.PlayerName $ ")");
		Level.Game.Broadcast(PC, PC.PlayerReplicationInfo.PlayerName $ " logged in as a server administrator" );

		return true;
	}

	return false;
}


function KickPlayerController(PlayerController PC, optional string Reason){
	local string Msg;

	PC.Destroy();

	Msg = PC.PlayerReplicationInfo.PlayerName $ " was kicked from the server";

	if(Reason != "")
		Msg = Msg $ " for " $ Reason;

	Log(Msg);
	Level.Game.Broadcast(self, Msg);
}

function BanPlayerController(PlayerController PC, optional string Reason){
	local string IP;
	local string Msg;
	local int i;

	IP = PC.GetPlayerNetworkAddress();

	if(CheckIPPolicy(IP)){
		IP = Left(IP, InStr(IP, ":"));

		Log("Adding IP Ban for: " $ IP);

		for(i = 0; i < 50; i++){
			if(IPPolicies[i] == "")
				break;
		}

		if(i < 50)
			IPPolicies[i] = "DENY," $ IP;

		SaveConfig();
	}

	PC.Destroy();

	Msg = PC.PlayerReplicationInfo.PlayerName $ " was banned from the server";

	if(Reason != "")
		Msg = Msg $ " for " $ Reason;

	Log(Msg);
	Level.Game.Broadcast(self, Msg);
}
