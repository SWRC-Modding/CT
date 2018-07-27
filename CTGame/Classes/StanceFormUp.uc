//=============================================================================
// SquadStance, base class of all squad stances
//=============================================================================
class StanceFormUp extends SquadStance
	native
	abstract;

//Icon representation - texture?
//A bunch of variables about how squad behaves in this stance

static function string GetStanceName()
{
	return Default.StanceName;
}


defaultproperties
{
     StanceName="Form Up"
     ActivateCue=PAE_FormUp
     TetherHPActivateRadius=1280
     TetherHPDeactivateRadius=768
     TetherLPDeactivateRadius=192
     StanceChangeEvent="StanceFormUp"
     BattleRestrictionTime=10
     TimeBeforeSeekingCover=2
     HUDIconX=1
     HUDIconY=4
     PreventType=STO_Leader
     TetherType=STO_Leader
     StanceType=STT_FormUp
     FavorCombatNearestTether=True
}

