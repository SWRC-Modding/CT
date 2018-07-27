// ====================================================================
//  Class:  WarfareGame.MuzzleFlashAttachment
//
//	These are attached to WeaponAttachments and are flashed
//	when a shot is fired
//
// (c) 2001, Epic Games, Inc - All Rights Reserved
// ====================================================================

class MuzzleFlashAttachment extends InventoryAttachment;

var int TickCount;	// How long to display it

var(MuzzleFlash) ELightType FlashLightType;
var(MuzzleFlash) ELightEffect FlashLightEffect;

simulated event Timer()
{
	bHidden=true;
}

simulated function Flash()
{
	GotoState('Visible');
}

simulated state Visible
{
	simulated event Tick(float Delta)
	{
		if (TickCount>2)
			gotoState('');
			
		TickCount++;
	}
	
	simulated function EndState()
	{
		bHidden=true;
		LightEffect=LE_None;
		LightType=LT_None;
		if( WeaponAttachment( Pawn(Owner).Weapon.ThirdPersonActor ).MuzzleLight != None )
		{
			WeaponAttachment( Pawn(Owner).Weapon.ThirdPersonActor ).MuzzleLight.LightEffect = LE_None;
			WeaponAttachment( Pawn(Owner).Weapon.ThirdPersonActor ).MuzzleLight.LightType = LT_None;
		}
	}
	
	simulated function BeginState()
	{
		local Rotator R;
		local vector V;
	
		TickCount=0;
		R = RelativeRotation;
		R.Pitch = Rand(65535);
		SetRelativeRotation(R);
	
		V=Default.DrawScale3D;
//		V.Z += frand() - 0.5;
		V.X += frand() - 0.5;
		V.Y += frand() - 0.5;
	
		SetDrawScale3D(v);
		bHidden=false;
		
		//LightEffect=FlashLightEffect;
		//LightType=FlashLightType;
		if( WeaponAttachment( Pawn(Owner).Weapon.ThirdPersonActor ).MuzzleLight != None )
		{
			WeaponAttachment( Pawn(Owner).Weapon.ThirdPersonActor ).MuzzleLight.LightEffect = FlashLightEffect;
			WeaponAttachment( Pawn(Owner).Weapon.ThirdPersonActor ).MuzzleLight.LightType = FlashLightType;
		}
	}
		
}		
		


defaultproperties
{
     bHidden=True
     bDynamicLight=True
     bAcceptsProjectors=False
     bUnlit=True
}

