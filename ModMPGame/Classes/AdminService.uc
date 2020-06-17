class AdminService extends Actor abstract native config(ModMPGame);

// Service will only be spawned if the current game mode is an object of this class or one of its subclasses
var() static config class<GameInfo> RelevantGameInfoClass;

var() bool bRequiresAdminPermissions; // Commands are only forwarded to the service if the issuer is logged in as an administrator

native static final function bool ParseCommand(out String Stream, String Match);
native static final function bool ParseIntParam(String Stream, String Match, out int Value);
native static final function bool ParseFloatParam(String Stream, String Match, out float Value);
native static final function bool ParseStringParam(String Stream, String Match, out String Value);

// Can be overridden to check for custom commands
native function bool ExecCmd(String Cmd, optional PlayerController PC);

static final function bool IsLocalPlayer(PlayerController PC){
	return PC != None && Viewport(PC.Player) != None;
}

cpptext
{
	virtual bool ExecCmd(const char* Cmd, class APlayerController* PC = NULL){ return false; }
}

defaultproperties
{
	bHidden=true
	RelevantGameInfoClass=class'GameInfo'
	bRequiresAdminPermissions=true
}
