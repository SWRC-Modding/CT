//=============================================================================
// StanceOffensive, a particular class of SquadStance
//=============================================================================
class StanceHold extends SquadStance
	native
	abstract;


defaultproperties
{
     StanceName="Hold"
     TetherHPActivateRadius=768
     TetherLPActivateRadius=512
     TetherHPDeactivateRadius=384
     TetherLPDeactivateRadius=256
     PreventRange=256
     HUDIconX=1
     HUDIconY=2
     PreventType=STO_Self
     TetherType=STO_Self
     StanceType=STT_Hold
}

