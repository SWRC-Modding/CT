class AdminService extends Actor abstract native config(ModMPGame);

// Service will only be spawned if the current game mode is an object of this class or one of its subclasses
var() static config class<GameInfo> RelevantGameInfoClass;

var() bool bRequiresAdminPermissions; // Commands are only forwarded to the service if the issuer is logged in as an administrator

native static final function bool ParseCommand(out string Stream, string Match);
native static final function string ParseToken(out string Stream);

native static final function bool ParseIntParam(string Stream, string Match, out int Value);
native static final function bool ParseFloatParam(string Stream, string Match, out float Value);
native static final function bool ParseStringParam(string Stream, string Match, out string Value);

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
