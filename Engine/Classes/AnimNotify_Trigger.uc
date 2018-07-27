class AnimNotify_Trigger extends AnimNotify_Scripted;

var() name EventName;
var() bool TriggerSelf;

event NotifyScript( Actor Owner )
{
	if( TriggerSelf )
		Owner.Trigger( Owner, Pawn(Owner) );
	else
		Owner.TriggerEvent( EventName, Owner, Pawn(Owner) );
}

defaultproperties
{
}

