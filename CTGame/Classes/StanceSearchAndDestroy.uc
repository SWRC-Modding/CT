//=============================================================================
// StanceOffensive, a particular class of SquadStance
//=============================================================================
class StanceSearchAndDestroy extends SquadStance
	native
	abstract;


defaultproperties
{
     StanceName="Search and Destroy"
     ActivateCue=PAE_SearchAndDestroy
     TetherHighPriority=0.45
     TetherLowPriority=0.2
     TetherHPActivateRadius=2048
     TetherLPActivateRadius=512
     TetherHPDeactivateRadius=1280
     TetherLPDeactivateRadius=256
     StanceChangeEvent="StanceSearchAndDestroy"
     BactaPreventRange=1280
     TimeBeforeSeekingCover=0
     HUDIconX=1
     HUDIconY=3
     PreventType=STO_Objective
     TetherType=STO_Objective
     StanceType=STT_SearchAndDestroy
     bLeapFrog=True
     bCheckCorpses=True
}

