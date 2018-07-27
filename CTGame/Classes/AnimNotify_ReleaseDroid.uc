class AnimNotify_ReleaseDroid extends AnimNotify_Scripted;

event NotifyScript( Actor Owner )
{
	local DroidDispenser Dispenser;

	if( Owner.IsA('DroidDispenser') )
	{
		Dispenser = DroidDispenser(Owner);
        if( Dispenser != None )
		{
            Dispenser.ReleaseDroid();
		}
	}
}

defaultproperties
{
}

