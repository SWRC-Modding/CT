//=============================================================================
// Mission Objective System
// 
// Created August 18, 2003 by Jenny Huang
//=============================================================================

class MissionObjectives extends Actor
	native
	placeable;

enum EObjectiveMode
{
	OM_Hidden,								// not revealed yet
	OM_Incomplete,							// revealed but not completed yet
	OM_Completed,							// revealed and completed
	OM_Failed,								// revealed and failed
	OM_Removed								// revealed and removed after an objective change
};

struct SingleObjective
{
	var()	localized	String			Objective;		// what's the obejctive
	var()	EObjectiveMode				Stat;			// Removed, Incomplete, Complete, Failed
//	var()	bool			Completed;		// has the objective been completed yet?
//	var()	bool			Revealed;		// has the objective been revealed yet?
};

var(Objectives)	editinline	localized Array<SingleObjective>	MissionObj;
var(Objectives)	float		FadeTime;			// amount of time before it fades from the critical text space
var(Objectives)	color		CritTextColor;		// color of the critical text
var(Objectives)	color		IncompleteColor;	// color on the pause screen for incomplete tasks
var(Objectives)	color		CompletedColor;		// color on the pause screen for the completed tasks
var(Objectives)	color		FailedColor;		// color on the pause screen for the failed tasks
var				int			CurrentObjective;	// pointer to the position in the array
var				int			NumObjectives;		// total number of objectives

function PostBeginPlay()
{
	local int i;

	NumObjectives = MissionObj.Length;
	//Log("NumObjectives "$NumObjectives);

	// Start the first objective as having been revealed if the bool is set
	for ( i=0; i < NumObjectives; i++ )
		if (MissionObj[i].Stat == OM_Incomplete)
			CurrentObjective = i;

	Super.PostBeginPlay();


}

// LD triggers when the next objective gets revealed to the player.
// Objective displays in the critical text region of the HUD.
function Trigger( Actor Other, Pawn EventInstigator )
{
	//Log("Triggered!  CurrentObjective "$CurrentObjective$" MissionObj.Length "$MissionObj.Length);
	CurrentObjective++;
	if (CurrentObjective >= MissionObj.Length)
		CurrentObjective = MissionObj.Length - 1;
	// find the next hidden objective to reveal
	while (MissionObj[CurrentObjective].Stat != OM_Hidden)
	{
		CurrentObjective++;
		if ( (CurrentObjective >= MissionObj.Length) || (CurrentObjective < 0) )
			return;
	}
	DisplayObjective(CurrentObjective);
}

function DisplayObjective(int CurrObj)
{
	//Log("DisplayObjective "$CurrObj$" MissionObj Length "$MissionObj.Length);
	if ( (CurrObj >= 0) && (CurrObj < MissionObj.Length) )
	{
		//Level.GetLocalPlayerController().myHUD.AddCriticalMessage(MissionObj[CurrObj].Objective, FadeTime, CritTextColor);
		MissionObj[CurrObj].Stat = OM_Incomplete;
	}
}

function bool ObjectiveCompleted( int ObjNum )
{
	if ( (ObjNum >= 0) && (ObjNum <= NumObjectives) )
	{
		MissionObj[ObjNum].Stat = OM_Completed;
		return true;
	}
	else
		return false;
}

function bool ObjectiveFailed( int ObjNum )
{
	if ( (ObjNum >= 0) && (ObjNum <= NumObjectives) )
	{
		MissionObj[ObjNum].Stat = OM_Failed;
		return true;
	}
	else
		return false;
}

function bool ObjectiveChanged( int ObjNum, int ChangeToObjNum )
{
	//Log("ObjectiveChanged ObjNum "$ObjNum$" ChangeToObjNum "$ChangeToObjNum);
	if ( (ObjNum >= 0) && (ObjNum <= NumObjectives) && (ChangeToObjNum >= 0) && (ChangeToObjNum <= NumObjectives) )
	{
		MissionObj[ObjNum].Stat = OM_Removed;
		DisplayObjective(ChangeToObjNum);
		return true;
	}
	else
		return false;
}



defaultproperties
{
     FadeTime=3
     CritTextColor=(B=255,G=255,R=255,A=255)
     IncompleteColor=(B=255,G=255,R=255,A=255)
     CompletedColor=(B=255,G=255,R=255,A=255)
     CurrentObjective=-1
}

