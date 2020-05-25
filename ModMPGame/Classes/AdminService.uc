/*
 * Base c l a s s (Can't use the word c l a s s here or the compiler will parse it despite this being a comment. WTF???)
 * for all services that can be instanciated by the AdminControl.
 * When an admin enters a command it is dispatched to all existing admin services.
 */
class AdminService extends Actor abstract native config(ModMPGame);

var bool bRequiresLogin; // Commands are only forwarded to the service if the issuer is logged in as an administrator

native final function bool ParseCommand(out String Stream, String Match);
native final function bool ParseIntParam(String Stream, String Match, out int Value);
native final function bool ParseFloatParam(String Stream, String Match, out float Value);
native final function bool ParseStringParam(String Stream, String Match, out String Value);

native function bool ExecCmd(PlayerController Player, String Cmd);

cpptext
{
	virtual bool ExecCmd(class APlayerController* Player, const char* Cmd){ return false; }
}

defaultproperties
{
	bRequiresLogin=true
}
