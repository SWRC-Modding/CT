//=============================================================================
// SquadStance, base class of all squad stances
//=============================================================================
class StanceFormUpTight extends StanceFormUp
	native
	abstract;

//Icon representation - texture?
//A bunch of variables about how squad behaves in this stance

static function string GetStanceName()
{
	return Default.StanceName;
}

//a tighter form up

defaultproperties
{
     TetherHPActivateRadius=512
     TetherLPActivateRadius=384
     TetherHPDeactivateRadius=256
     TetherLPDeactivateRadius=128
     NextStance=Class'CTGame.StancePreviousOrder'
     NextStanceTime=10
     PreventRange=256
}

