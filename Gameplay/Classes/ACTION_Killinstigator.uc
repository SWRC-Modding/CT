class ACTION_KillInstigator extends ScriptedAction;

var() class<DamageType>		 DamageType;

function bool InitActionFor(ScriptedController C)
{
	C.GetInstigator().Died( None, DamageType, C.Instigator.Location );
	return false;	
}

function string GetActionString()
{
	if (Len(Comment) > 0)
		return ActionString@DamageType$" // "$Comment;
	return ActionString@DamageType;
}


defaultproperties
{
     DamageType=Class'Engine.Crushed'
     ActionString="Damage instigator"
}

