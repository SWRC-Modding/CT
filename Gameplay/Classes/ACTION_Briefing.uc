class ACTION_Briefing extends ScriptedAction;

// variables for player rotation while disarmed
var(Action) float MaxPitch;
var(Action) float MinPitch;
var(Action) float MaxYaw;
var(Action) float MinYaw;
var(Action) bool  bEndBriefing;
var(Action)	bool  bSetBriefingWeapon;
var(Action)	autoload string BriefingWeapon;
var(Action) bool  bDrawHUD;
var(Action) bool  bOkToChangeWeapon;

function bool InitActionFor(ScriptedController C)
{
	local PlayerController PC;
	local Pawn PCPawn;
	local Inventory Inv;
	local int InvGrp;

	ForEach C.AllActors(class'PlayerController', PC)
		break;

	if ( PC != None && PC.Pawn != None )
	{
		PCPawn = PC.Pawn;
		if (bEndBriefing)
			PC.GotoState('PlayerWalking');
		else
		{
			PC.bDuck = 0;
			PC.LimitRotation(MaxPitch, MinPitch, MaxYaw, MinYaw);
			if (!PC.IsInState('Briefing'))
				PC.GotoState('Briefing');
			if (bSetBriefingWeapon)
			{
				Inv = PCPawn.CreateInventory(BriefingWeapon);
				if (Weapon(Inv) != None)
				{
					InvGrp = Weapon(Inv).GetInventoryGroup();
					PC.ForceWeaponSwitch(InvGrp);			// SwitchWeapon has a check for bBriefing which we want to bypass
				}
			}
			else 
			{
				if (PCPawn.Weapon != None)
				{
					PCPawn.Weapon.GotoState('');
					PCPawn.Disarm();
					if (bOkToChangeWeapon)
						PCPawn.ChangedWeapon();
				}
			}
		}
		PC.DrawHUD(bDrawHUD);
   	}

	return false;	
}


defaultproperties
{
     MaxPitch=32768
     MinPitch=-32768
     MaxYaw=32768
     MinYaw=-32768
     bDrawHUD=True
     ActionString="Briefing"
}

