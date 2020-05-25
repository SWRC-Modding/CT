class AdminAuthentication extends AdminService;

var() config String AdminPassword;

function PostBeginPlay(){
	Super.PostBeginPlay();

	if(AdminPassword != "") // If AdminPassword is empty, the one from AccessControl is used
		Level.Game.AccessControl.SetAdminPassword(AdminPassword);
}

function bool ExecCmd(PlayerController Player, String Cmd){
	if(ParseCommand(Cmd, "LOGIN")){
		if(Player.PlayerReplicationInfo.bAdmin)
			Player.ClientMessage("You are already logged in!");
		else if(Level.Game.AccessControl.AdminLogin(Player, Cmd))
			Player.PlayerReplicationInfo.bAdmin = true;
		else
			Player.ClientMessage("Wrong password!");

		return true;
	}else if(ParseCommand(Cmd, "LOGOUT")){
		if(Player.PlayerReplicationInfo.bAdmin){
			Player.PlayerReplicationInfo.bAdmin = false;
			Player.ClientMessage("You are no longer an admin!");
		}else{
			Player.ClientMessage("You are not logged in!");
		}

		return true;
	}

	return false;
}

defaultproperties
{
	bRequiresLogin=false // This must be false or players won't be able to use the /login command
}
