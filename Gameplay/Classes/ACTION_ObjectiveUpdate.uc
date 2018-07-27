class ACTION_ObjectiveUpdate extends ScriptedAction;

enum EUpdateAction
{
	UA_ObjCompleted,
	UA_ObjFailed,
	UA_ObjChanged
};

var(Action)		int				ObjectiveNum;
var(Action)		EUpdateAction	UpdateAction;
var(Action)		int				ChangeToObjNum;			// if UA_ObjChange, which objective number to change to?
var(Action)		localized String DisplayText;			// text to display in the critical text window
var(Action)		Sound			PlaySound;				// sound to play

function bool InitActionFor(ScriptedController C)
{
	local MissionObjectives Obj;
	local PlayerController Player;
	local bool result;

	ForEach C.AllActors(class'MissionObjectives', Obj)
		break;

	//Log("ObjNum "$ObjectiveNum$" Obj.CurrentObj "$Obj.CurrentObjective);
	switch(UpdateAction)
	{
	case UA_ObjFailed:
		result = Obj.ObjectiveFailed(ObjectiveNum);
		break;
	case UA_ObjChanged:
		result = Obj.ObjectiveChanged(ObjectiveNum, ChangeToObjNum);
		break;
	default:
		result = Obj.ObjectiveCompleted(ObjectiveNum);
		break;
	}

	if (DisplayText != "")
	{	
		ForEach C.DynamicActors( class 'PlayerController', Player )
		{
			if( Player.myHUD != None )
				Player.myHUD.AddCriticalMessage( DisplayText, Obj.FadeTime, Obj.CritTextColor );
		}
	}

	if (PlaySound != None)
		C.PlaySound(PlaySound);
	
	// return false for completion
	return !result;
}


defaultproperties
{
     ActionString="Objective Completed"
}

