class AnimNotify_TossWeapon extends AnimNotify_Scripted;

event NotifyScript( Actor Owner )
{
	local Pawn OwnerPawn;
	
	if( Owner.IsA('Pawn') )
	{
		OwnerPawn = Pawn(Owner);
        if( OwnerPawn.Weapon != None )		
            OwnerPawn.TossWeapon( OwnerPawn.Weapon, vect(0,0,0) );
	}
}

defaultproperties
{
}

