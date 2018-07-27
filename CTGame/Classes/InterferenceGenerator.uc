class InterferenceGenerator extends Actor
	placeable;

var() bool bIsOn;

function Trigger( Actor Other, Pawn EventInstigator )
{
	if( bIsOn )
		TurnOff();
	else
		TurnOn();
}

function TurnOn()
{
	bIsOn = true;
}

function TurnOff()
{
	bIsOn = false;
}


defaultproperties
{
     bIsOn=True
     bHidden=True
     bCollideActors=True
     CollisionRadius=0.01
     CollisionHeight=0.01
}

