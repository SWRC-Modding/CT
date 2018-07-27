class ACTION_ChangeLevel extends ScriptedAction;

var(Action) string URL;
var(Action) bool bShowLoadingMessage;

function bool InitActionFor(ScriptedController C)
{
	local PlayerController PC;

	if (C.Level.NetMode == NM_Standalone)
	{
		ForEach C.AllActors(class'PlayerController', PC)
			break;
		if ( (PC != None) && (PC.Pawn != None) && (PC.Pawn.Squad != None) )
			PC.Pawn.SaveSquadTravelInfo();
		C.Level.Game.SendPlayer(PC, URL);
	}
	else
	{
		if( bShowLoadingMessage )
			C.Level.ServerTravel(URL, false);
		else
			C.Level.ServerTravel(URL$"?quiet", false);
	}
	return true;	
}


defaultproperties
{
     ActionString="Change level"
}

