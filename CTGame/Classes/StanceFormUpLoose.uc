//=============================================================================
// SquadStance, base class of all squad stances
//=============================================================================
class StanceFormUpLoose extends StanceFormUp
	abstract;

//Icon representation - texture?
//A bunch of variables about how squad behaves in this stance

static function string GetStanceName()
{
	return Default.StanceName;
}

//same low priority tether as FormUp, but a looser high-pri tether

defaultproperties
{
     TetherHighPriority=0.4
     TetherHPActivateRadius=2048
     NextStance=Class'CTGame.StancePreviousOrder'
     NextStanceTime=10
     PreventRange=1280
}

