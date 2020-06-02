class AdminAuthentication extends AdminService;

var() config String AdminPassword;

function PostBeginPlay(){
	Super.PostBeginPlay();

	if(AdminPassword != "") // If AdminPassword is empty, the one from AccessControl is used
		Level.Game.AccessControl.SetAdminPassword(AdminPassword);
}

function bool ExecCmd(PlayerController PC, String Cmd){
	if(ParseCommand(Cmd, "LOGIN")){
		if(PC.PlayerReplicationInfo.bAdmin)
			PC.ClientMessage("You are already logged in!");
		else if(Level.Game.AccessControl.AdminLogin(PC, Cmd))
			PC.PlayerReplicationInfo.bAdmin = true;
		else
			PC.ClientMessage("Wrong password!");

		return true;
	}else if(ParseCommand(Cmd, "LOGOUT")){
		if(PC.PlayerReplicationInfo.bAdmin){
			PC.PlayerReplicationInfo.bAdmin = false;
			PC.ClientMessage("You are no longer an admin!");
		}else{
			PC.ClientMessage("You are not logged in!");
		}

		return true;
	}

	return false;
}

defaultproperties
{
	bRequiresAdminPermissions=false // This must be false or players won't be able to use the /login command
}
