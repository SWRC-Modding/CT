class CTMarker extends SquadMarker
	native;

native event MakeObsolete();
native function MemberKilled(Pawn Victim, Controller Killer, class<DamageType> damageType);

/*
function Trigger( Actor Other, Pawn EventInstigator )
{
	Log("CTMarker::Trigger");
	if (Tag != 'None')
	{
		//Let the squad know that the marker event got triggered
		Tag = 'None';
	}
}
*/

defaultproperties
{
     ActivatePromptButtonFuncs(0)="Use | onrelease StopUse"
     CancelPromptButtonFuncs(0)="Use | onrelease StopUse"
}

