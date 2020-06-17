/*
 * Base c l a s s (Can't use the word c l a s s here or the compiler will parse it despite this being a comment. WTF???)
 * for all services that can be instanciated by the AdminControl.
 * When an admin enters a command it is dispatched to all existing admin services.
 */
class AdminService extends Actor abstract native config(ModMPGame);

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
	bRequiresAdminPermissions=true
}
