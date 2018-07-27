// ====================================================================
//  Class:  CTGame.SquadMarker
//
//  Test Marker Object
// ====================================================================

class SquadMarker extends MarkerIconActor
	native
	abstract
	placeable
	hidecategories(Collision, CollisionAdvanced, DisplayAdvanced, Force,Karma,LightColor,Lighting,MovementAdvanced,Sound);

enum EMarkerAction
{
	MA_None,
	MA_Activate,
	MA_Cancel
};

enum EMarkerUsability
{
	MU_Invalid, //bad setup
	MU_Irrelevant,
	MU_Incapacitated,
	MU_Occupied,
	MU_Usable
};

struct MarkerParticipant
{
	var StaticMesh	IconMesh;
	var MarkerIconActor	IconActor;
	var Pawn		Member;		//the actual clone (NULL if not participating?)
	var bool		bIconOnAnchorPt;
	//var bool		bParticipating;
	var vector		IconOffset;
};


var(Marker) Volume						EnableVolume;
var(Marker)	array<Actor>				Actors;
var(Marker)	Actor						Anchor;
var(Marker) name						EventUnderway;	//this event is triggered when the marker is considered "underway"
var(Marker) name						EventCompleted; //this event is triggered when the marker is completed
var			Squad						Squad;
var			BYTE						MaxMembers; //this is the max members that the marker is written to accomodate
var(Marker) BYTE						MembersRequired; //the number of capable squad members required to activate this marker
var			BYTE						MarkerGoals;
var			BYTE						TotalParticipants;	//the number actually included
var static	BYTE						HUDIconX;
var static  BYTE						HUDIconY;

var(Marker)	Array<Name>					MarkerAnims;
var(Marker) SquadMarker					AlternateMarkers[3];	//assume no more than 3 alternates
var(Marker) Sound						CustomInitiateSound;
var			static String				ActionString;
var			float						LastActivateTime;
var			int							MarkerStage;

var			MarkerParticipant			Participants[3];
var			bool						bSetupPhase;
var			bool						bCanCancelAfterUnderway;
var			bool						bComplete;	//whether the marker can be considered complete (affects how auto-cancel works)
var			bool						bMarkerActive;
var			bool						bDisabledByAlternate;
var			bool						bValidSetup;	//the marker was set up properly
var			bool						AlwaysVisible;
var			bool						bFailed;		//whether a goal for the marker has failed
var			bool						bCancelIfMembersNeeded;
//var			bool						bWantsToCancel;	//the player has asked to cancel this marker
var	static  class<Pawn>					PreferredPawnClass;

var static	class<SquadStance>				InitiateStance;
var static  class<SquadStance>				FinishStance;
var	static	PawnAudioTable.EPawnAudioEvent	InitiateCue;
var	static	PawnAudioTable.EPawnAudioEvent	ConfirmCue;
var	static	PawnAudioTable.EPawnAudioEvent	CancelCue;
var static  PawnAudioTable.EPawnAudioEvent	CancelConfirmCue;
var static	PawnAudioTable.EPawnAudioEvent	IrrelevantCue; //if player tries to acti

// Marker prompt vars
var (Marker) static localized String ActivatePromptText;
var (Marker) Array<String> ActivatePromptButtonFuncs;
var (Marker) static localized String CancelPromptText;
var (Marker) Array<String> CancelPromptButtonFuncs;
var (Marker) static localized String IrrelevantPromptText;	//text for when the marker is irrelevant for some reason

// Display Vars
var	(Marker)	float TargetingRadius;	// Size of the highlight sphere
var	(Marker)	float TargetingDistance;	// Min distance to highlight

// Available State Colors
// should make these constants
var	(Display) const Color EnabledColor;
var (Display) const Color OmitColor;
var (Display) const Color OccupiedColor;
var (Display) const Color SetupColor;

replication
{
	reliable if( Role == ROLE_Authority )
		bSetupPhase;
}

simulated native function EMarkerUsability GetMarkerUsability(Pawn Instigator);
native function bool Initiate(Pawn Instigator); //we want this to run on server
native function bool CancelMarker(Pawn Instigator);
simulated native function vector GetMarkerSpotLocation(int index);
function MemberKilled(Pawn Victim, Controller Killer, class<DamageType> damageType);
function MakeObsolete();
native event bool IsSetUpProperly();
native function UnderwayCallback();

simulated event bool MarkersActive()
{
	local int i;

	if (bMarkerActive)
		return true;
	for (i=0; i < 3; ++i)
	{
		if (AlternateMarkers[i] != None && AlternateMarkers[i].bMarkerActive)
			return true;
	}
	return false;
}

simulated event EMarkerAction CurrentMarkerAction()
{
	if (bDisabledByAlternate)
		return MA_None;
	return MA_Activate;
}

function PostBeginPlay()
{
	local vector loc;
	local rotator rot;
	local int i;

	//the first icon actor is the marker itself
	Participants[0].IconActor = self;

	//TotalParticipants = MaxMembers;

	for (i=0; i < MaxMembers; ++i)
	{
		if (Participants[i].IconMesh != None)
		{
			if (Participants[i].bIconOnAnchorPt)
			{
				if (Anchor != None && i < Anchor.NavPts.Length)
				{
					loc = Anchor.NavPts[i].Location;//GetMarkerSpotLocation(i);
					loc.Z -= Anchor.NavPts[i].CollisionHeight;
					rot = Anchor.NavPts[i].Rotation;
				}
			}
			else
			{
				loc = Location + (Participants[i].IconOffset >> Rotation);//.RotateAngleAxis(Rotation.Yaw, FVector(0,0,1));
				rot = Rotation;
			}

			if (i == 0)
			{
				if (Participants[i].bIconOnAnchorPt)
				{
					SetLocation(loc);
					SetRotation(rot);
				}
			}
			else
			{
				Participants[i].IconActor = Spawn(class'MarkerIconActor',,,loc,rot);
				Participants[i].IconActor.SetStaticMesh(Participants[i].IconMesh);
				Participants[i].IconActor.SquadMarker = self;
			}
		}
	}
	bValidSetup = IsSetUpProperly();
}

simulated function ShowIcons ( bool Show, int Start, int Stop )
{
	local int i;
	for (i=Start; i < Stop; ++i)
	{
		// TimR: For now participants should not be hidded as they need to fade away
		//		 The draw code will make sure they are not rendered if alpha is 0
		Participants[i].IconActor.bHidden = !Show;
/*
		Participants[i].IconActor.bHidden = !show;
		if( show )
		{
			Participants[i].IconActor.CurrentAlpha = 0;
			Participants[i].IconActor.CurrentMorph = 0;
		}
*/
	}
}

simulated function UpdateIcons( PlayerController Player )
{
	//local bool bUsable;
	local EMarkerUsability Usability;

	//don't update icons if we're already in the setup phase
	if (!bSetupPhase)
	{
		/*
		if (!bMarkerActive)
		{
			Squad = Player.Pawn.Squad;
			//if the marker wasn't set up properly, don't even show the icons
			if (Squad == None || Anchor == None || Anchor.bDeleteMe || !bValidSetup)
			{
				ShowIcons(false, 0, MaxMembers);
				return;
			}
		}
		*/
		Usability = GetMarkerUsability(Player.Pawn);
		switch(Usability)
		{
			case MU_Invalid:
			case MU_Irrelevant:
				ShowIcons(false, 0, MaxMembers);
				break;
			case MU_Occupied:
			case MU_Incapacitated:
				ShowIcons(true, 0, MaxMembers);
				//ShowIcons(true, 0, MembersRequired);
				//ShowIcons(false, MembersRequired, MaxMembers);
				break;
			case MU_Usable:
				ShowIcons(true, 0, MaxMembers);
				break;
		}
		//show all the icons regardless
		/*
		bUsable = IsUsable(Player.Pawn);
		if (!bUsable)
		{
			ShowIcons(true, 0, 1);
			ShowIcons(false, 1, MaxMembers);
		}
		else
		{
			ShowIcons(true, 0, MaxMembers);	 //if usable, show all the icons		
		}
		*/
	}
}

simulated function NotifyTargeted( PlayerController Player )
{
	UpdateIcons(Player);
}

simulated function NotifyUnTargeted( PlayerController Player )
{
//	if (!bSetupPhase)
//		ShowIcons(false, 1, MaxMembers);
}

event bool ExecuteMarker( Pawn Instigator )
{
	switch (CurrentMarkerAction())
	{
	case MA_None:
		break;
	case MA_Activate:
		return Initiate(Instigator);
		break;
	case MA_Cancel:
		CancelMarker(Instigator);
		break;
	}
	return true;
}

function bool IsEnableVolumeEmptyOfEnemies()
{
	local Pawn P;
	if (EnableVolume == None)
		return false;
	ForEach EnableVolume.TouchingActors(class'Pawn', P)
	{
		if (!P.IsDeadOrIncapacitated() && P.GetTeam().IsEnemy(1)) //if there are any enemies to the player in the volume
			return false;
	}
	return true;
}

// This function is used by the editor to indicate to the search function
// that an event is important to this guy
function bool ContainsPartialEvent(string StartOfEventName)
{
	local string EventString;
	EventString = string(EventUnderWay);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}

	EventString = string(EventCompleted);
	if ( Caps(StartOfEventName) == Left(Caps(EventString), Len(StartOfEventName) ) )
	{
		return true;
	}

	return super.ContainsPartialEvent(StartOfEventName);
}

State() Disabled
{
	ignores NotifyTargeted;

	function BeginState()
	{
		//Log("SquadMarker::Disabled::BeginState "$self);
		if (bMarkerActive && bCanCancelAfterUnderway)
		{
			CancelMarker(None);
		}
		ShowIcons(false, 0, MaxMembers);
	}

	simulated event EMarkerAction CurrentMarkerAction()
	{
		if (bMarkerActive)
			return MA_Cancel;
		return MA_None;
	}

	function Trigger( actor Other, pawn EventInstigator )
	{		
		GotoState('Enabled');	
	}

Begin:
	bBlockZeroExtentTraces=false;
	bBlockNonZeroExtentTraces=false;
	bProjTarget=false;
Loop:
	Sleep(1.0f);
	if (IsEnableVolumeEmptyOfEnemies())
		GotoState('Enabled');
	if (EnableVolume != None)
		Goto('Loop');
}

auto State() Enabled
{
	function BeginState()
	{
		if (Anchor != None)
			Anchor.bHidden = false;
		bHidden = false;
		ShowIcons(false, 1, MaxMembers);
		CurrentColor=EnabledColor;
		//bProjTarget=true;
	}

Begin:

}


State Setup
{
	simulated event EMarkerAction CurrentMarkerAction()
	{
		return MA_Cancel;
	}

	simulated function BeginState()
	{
		local int i;		
		for (i=0; i < TotalParticipants; ++i)
		{
			Participants[i].IconActor.CurrentColor = SetupColor;
		}
		ShowIcons(false, TotalParticipants, MaxMembers);
		bSetupPhase = true;
		CurrentColor=SetupColor;
	}
	simulated function EndState()
	{
		bSetupPhase = false;
	}


Begin:

}

State Executed
{
	simulated function UpdateIcons( PlayerController Player){}
	simulated function NotifyTargeted( PlayerController Player )
	{
		//do nothing in this state when the marker/marker trigger is targeted
	}
	simulated event EMarkerAction CurrentMarkerAction()
	{
		if (bCanCancelAfterUnderway && !bComplete)
			return MA_Cancel;
		return MA_None;
	}
	function BeginState()
	{	
		UnderwayCallback();
		TriggerEvent(EventUnderway, self, Participants[0].Member );
		if (Anchor.IsA('ActivateItem'))
			TriggerEvent(ActivateItem(Anchor).EventUnderway, Anchor, Participants[0].Member );
		bSetupPhase = false;
		//CurrentIconCount = 0;
		AlwaysVisible=false;
		ShowIcons(false, 0, MaxMembers);
		bBlockZeroExtentTraces=false;
		bBlockNonZeroExtentTraces=false;
		bProjTarget=false;
	}
Begin:	
}


State Completed
{
	simulated event EMarkerAction CurrentMarkerAction()
	{
		return MA_None;
	}
	simulated function UpdateIcons( PlayerController Player)
	{
	}
	simulated function NotifyTargeted( PlayerController Player )
	{
		//do nothing in this state when the marker/marker trigger is targeted
	}
	function BeginState()
	{		
		TriggerEvent(EventCompleted, self, Instigator ); //trigger our EventCompleted upon Marker completion
		ShowIcons(false, 0, MaxMembers);
	}
}



defaultproperties
{
     MaxMembers=1
     MembersRequired=1
     LastActivateTime=-1e+010
     ActivatePromptText="PRESS @ TO ENGAGE MANEUVER"
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptText="PRESS @ TO CANCEL MANEUVER"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
     TargetingDistance=3000
     EnabledColor=(B=255,G=206,R=122,A=50)
     OmitColor=(B=122,G=126,R=254,A=50)
     OccupiedColor=(B=122,G=180,R=230,A=50)
     SetupColor=(B=122,G=255,R=143,A=255)
     bCollideActors=True
     bBlockZeroExtentTraces=False
     bBlockNonZeroExtentTraces=False
     InitialState="Enabled"
     CollisionRadius=0
     CollisionHeight=0
}

