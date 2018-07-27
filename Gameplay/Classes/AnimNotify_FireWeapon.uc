class AnimNotify_FireWeapon extends AnimNotify_Scripted;

var() bool ContinuousFire;
var() bool StopFire;
var() bool FireSecondary;
var() bool DisableFireAnim;

event NotifyScript( Actor Owner )
{
	local Pawn OwnerPawn;
	local Controller OwnerController;

	if( Owner.IsA('Pawn') )
	{
		OwnerPawn = Pawn(Owner);
		OwnerController = OwnerPawn.Controller;

        if( OwnerPawn.Weapon != None )
		{
			if( StopFire )
			{
				OwnerController.bFire = 0;
				OwnerPawn.Weapon.ServerStopFire();
				OwnerPawn.bDisableFireAnims = false;
				return;
			}
			else
			{
				if( ContinuousFire )				
					OwnerController.bFire = 1;

				OwnerPawn.bDisableFireAnims = DisableFireAnim;

				if( FireSecondary && OwnerPawn.CurrentGrenade != None )				
					OwnerPawn.CurrentGrenade.ServerFire();				
				else				
					OwnerPawn.Weapon.ServerFire();					
				
				if( !ContinuousFire )				
					OwnerPawn.bDisableFireAnims = false;
			}
		}
	}
	if (Owner.IsA('Weapon'))
		Weapon(Owner).FireWeapon();
}

defaultproperties
{
}

