//=============================================================================
// StanceSecurePosition, a particular class of SquadStance
//=============================================================================
class StanceSecurePosition extends SquadStance
	native
	abstract;


defaultproperties
{
     StanceName="Secure Area"
     ActivateCue=PAE_SecurePosition
     TetherLowPriority=0.35
     TetherLPDeactivateRadius=128
     StanceChangeEvent="StanceSecurePosition"
     PreventRange=640
     BactaPreventRange=768
     TimeBeforeSeekingCover=0
     HUDIconX=1
     HUDIconY=5
     PreventType=STO_Objective
     TetherType=STO_Objective
     StanceType=STT_EngagePosition
     bCheckCorpses=True
     FavorCombatNearestTether=True
}

