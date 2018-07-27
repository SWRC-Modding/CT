//=============================================================================
// StanceOffensive, a particular class of SquadStance
//=============================================================================
class StanceHoldLoose extends StanceHold
	native
	abstract;


defaultproperties
{
     TetherLowPriority=0.2
     TetherHPActivateRadius=1280
     TetherLPActivateRadius=768
     NextStance=Class'CTGame.StancePreviousOrder'
     NextStanceTime=5
     PreventRange=1024
     TimeBeforeSeekingCover=0
}

