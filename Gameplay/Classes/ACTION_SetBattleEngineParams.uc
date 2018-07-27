class ACTION_SetBattleEngineParams extends ScriptedAction;

var(Action) Actor.EBattleEngineBattleCriteria criteria;
var(Action) int baddieCount;
var(Action) float radiusInner;
var(Action) float radiusOuter;

function bool InitActionFor(ScriptedController C)
{
	local Controller A;

	For ( A=C.Level.ControllerList; A!=None; A=A.nextController )
		if ( A.IsA('PlayerController') && PlayerController(A).ViewTarget != None )
			PlayerController(A).ViewTarget.SetBattleEngineParams( criteria, baddieCount, radiusInner, radiusOuter  );
	return false;	
}


defaultproperties
{
     criteria=BATTLECRITERIA_AttackAndProximity
     baddieCount=1
     radiusInner=1250
     radiusOuter=2500
     ActionString="set battle criteria"
}

