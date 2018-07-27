class ACTION_ChangeSquadMember extends ScriptedAction;

var(Action) bool				bAddMember;
var(Action) name				MemberTag;
var(Action) name				SquadTag;

function bool InitActionFor(ScriptedController C)
{
	local Pawn Pawn;
	local PlayerController PC;
	local Squad Squad;

	if (SquadTag != '')
	{
		ForEach C.AllActors(class'Squad',Squad,SquadTag)
		{
		}
	}
	
	//if a different squad wasn't specified, use the player squad
	if (Squad == None)
	{
		PC = C.Level.GetLocalPlayerController();
		Squad = PC.Pawn.Squad;
	}

	if ( MemberTag != '' )
	{
		ForEach C.AllActors(class'Pawn',Pawn,MemberTag)
		{
			if (Pawn.Squad != None && !bAddMember)
			{
				Squad.RemoveFromSquad(Pawn);
			}
			if (Pawn.Squad == None && bAddMember)
			{
				Squad.AddToSquad(Pawn);
			}
		}
	}
	return false;	
}

function string GetActionString()
{
	if (!bAddMember)
		ActionString = "Remove squad member";
	if (Comment != "")
		return ActionString@MemberTag$" // "$Comment;
	return ActionString@MemberTag;
}


defaultproperties
{
     bAddMember=True
     ActionString="Add squad member"
}

