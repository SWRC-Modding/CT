// ====================================================================
//  Class:  GamePlay.Action_ConsoleCommand
//  Parent: GamePlay.ScriptedAction
//
//  Executes a console command
// ====================================================================

class Action_ConsoleCommand extends ScriptedAction;

var(Action) string		CommandStr;	// The console command to execute

function bool InitActionFor(ScriptedController C)
{
	if (CommandStr!="")
	{
		C.Level.GetLocalPlayerController().ConsoleCommand(CommandStr);
	}
		
	return false;	
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@CommandStr$" // "$Comment;
	return ActionString@CommandStr;
}


defaultproperties
{
     ActionString="console command"
}

