//=============================================================================
// StanceEngageTarget, a particular class of SquadStance
//=============================================================================
class StanceEngageTarget extends SquadStance
	native
	abstract;


defaultproperties
{
     StanceName="Engage Target"
     ActivateCue=PAE_EngageTarget
     TetherLowPriority=0.45
     TetherHPActivateRadius=2048
     TetherLPActivateRadius=512
     TetherHPDeactivateRadius=1536
     TetherLPDeactivateRadius=128
     StanceChangeEvent="StanceEngageTarget"
     PreventRange=1536
     IdlePreventRange=512
     BactaPreventRange=512
     HUDIconX=2
     HUDIconY=7
     PreventType=STO_Objective
     TetherType=STO_Objective
     StanceType=STT_EngageTarget
}

