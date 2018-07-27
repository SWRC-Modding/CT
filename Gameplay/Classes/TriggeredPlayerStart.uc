class TriggeredPlayerStart extends PlayerStart;

function Trigger( actor Other, pawn EventInstigator )
{
	bEnabled = !bEnabled;
}


defaultproperties
{
     bStatic=False
}

