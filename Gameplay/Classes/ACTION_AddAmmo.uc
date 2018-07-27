class ACTION_AddAmmo extends ScriptedAction;

var(Action) class<Weapon>	WhichWeapon;
var(Action)	int				AmmoAmount;

function bool InitActionFor(ScriptedController C)
{
	local PlayerController PC;
	local Weapon Weap;

	//Log("ACTION_AddAmmo C "$C);
	
	ForEach C.AllActors(class'PlayerController', PC)
		break;

	//Log("ACTION_AddAmmo PC "$PC$" WhichWeapon "$WhichWeapon$" AmmoAmount "$AmmoAmount);

	if ( (WhichWeapon != None) && (PC.Pawn != None) )
	{
		Weap = Weapon(PC.Pawn.FindInventoryType(WhichWeapon));
		//Log("Weap "$Weap$" AmmoType "$Weap.AmmoType);
		if (Weap != None && Weap.AmmoType != None)
		{
			Weap.AmmoType.AddAmmo(AmmoAmount);
			//Log("Added Ammo.  Amount "$Weap.AmmoType.AmmoAmount);
		}
	}
	return false;	
}

defaultproperties
{
}

