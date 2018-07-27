//=============================================================================
// Clone Troooper HUD
//
// Created: 2003 Feb 2, Nathan Martz
//=============================================================================

class CTHUD extends HUD;

var float ScaleX, ScaleY;
var float DisplayFOV;		// FOV for drawing the HUD mesh in first person view
var CTFontInfo MyFont;

// Slip vars.
var() float MaxSlipDist;		// Max units slippage
var() float MaxSlipDeg;			// Max degrees slippage.
var() float UnslipRate;			// Rate of slip restoration.
var() float	ScavDroidMaxRange;	// Max Dist that Scav Droids can affect the HUD
var() float	ScavStaticGamma;	// Gamma for fading in static
var rotator RotVelocity;		// Retained rotational velocity.
var float LastTick;

const MAX_SCAV_STATIC = 128;


function RenderToScale(Canvas C, Texture T)
{
    C.DrawTile(T, T.USize * ScaleX, T.VSize * ScaleY, 0, 0, T.USize, T.VSize);

    return;
}

simulated function PostBeginPlay()
{
	MyFont = CTFontInfo(spawn(Class<Actor>(DynamicLoadObject("CTGame.CTFontInfo", class'Class'))));
	Super.PostBeginPlay();

	// Create the default camera effect
	//PlayerOwner.CreateCameraEffect(class'FrameFX');
}

event Tick(float DeltaTime)
{
	local InterferenceGenerator TestGenerator;	
	local float DistSqr;
	local float SmallestDistSqr, SmallestDist;
	local float test;

	if( bScavangerHead )
	{
		StaticAlpha = MAX_SCAV_STATIC;
	}
	else
	{
		// NathanM:
		// The following double iteration though the collison hash really stinks, but it's the lesser of two evils
		// I tried attaching an Interference Generator to the ScavDroids, but that prevents it from showing up in
		// the CollidingActors list. I figured it was better to do two radius checks w/ the Hash than to do one
		// check against all of the actors in the world.

		SmallestDistSqr = Square(10000);
		ForEach CollidingActors( class'InterferenceGenerator', TestGenerator, ScavDroidMaxRange, Location )
		{
			if( TestGenerator.bIsOn )
			{
				DistSqr = VDistSq( TestGenerator.Location, Location );
				if( DistSqr < SmallestDistSqr )
				{												
					SmallestDistSqr = DistSqr;				
				}
			}
		}

		/* NathanM: I new spawn an interference generator w/ each ScavDroid
		ForEach CollidingActors( class'CTPawn', TestPawn, ScavDroidMaxRange, Location )
		{
			if( TestPawn.IsA('TrandoshanScav') && TestPawn.Health > 0 )
			{
				DistSqr = VDistSq( TestPawn.Location,Location );
				if( DistSqr < SmallestDistSqr )
				{												
					SmallestDistSqr = DistSqr;				
				}
			}
		}
		*/

		// TimR: Make sure that 10000 is not used
		if( SmallestDistSqr < Square(ScavDroidMaxRange) )
		{
			SmallestDist = Sqrt(SmallestDistSqr);
			Test = ( 1.0 - ( SmallestDist / ScavDroidMaxRange ) );		
			Test = Test ** ScavStaticGamma;		
			StaticAlpha = int( test * MAX_SCAV_STATIC );			
		}
		else
			StaticAlpha = 0;
	}

	// To do: this must be stored in the level somewhere, no?
	LastTick = DeltaTime;

	Super.Tick( DeltaTime );
}

function PreRender(Canvas C)
{
	local vector NewLoc, Slip;
	local rotator NewRot, SlipRot;
	local float SlipRotDeg;
	local Pawn PawnOwner;
	local Actor Temp;

	// Draw Helmet if PawnOwner is one
	
    ScaleX = C.SizeX / 800.0;
    ScaleY = C.SizeY / 600.0;

    //C.Reset();

	// CL: Get the player's current pawn
	PawnOwner = PlayerOwner.Pawn;
    
	// ------- Advance inertially toward location -------
	if( PawnOwner != None )
	{
		PlayerOwner.PlayerCalcView( Temp, NewLoc, NewRot );
		//NewLoc = PawnOwner.Location + PawnOwner.EyePosition();
		if( MaxSlipDist > 0.0 && !PlayerOwner.bBriefing )
		{
			Slip = Location + Velocity * LastTick - NewLoc;

			// Decay the slip, then limit it to asymptotically approach MaxSlipDist.
			Slip *= exp(-LastTick * UnslipRate);
			Slip *= MaxSlipDist / (MaxSlipDist + VSize(Slip));

			NewLoc += Slip;
		}

		if(LastTick > 0.0f)
			Velocity = (Velocity + (NewLoc - Location) / LastTick) * 0.5;

		SetLocation(NewLoc);

		// Apply a similar formula to rotation.	
		//NewRot = PawnOwner.GetViewRotation();

		if( MaxSlipDeg > 0.0 && !PlayerOwner.bBriefing )
		{
			SlipRot = Rotation + RotVelocity * LastTick - NewRot;
			SlipRot *= exp(-LastTick * UnslipRate);
			SlipRotDeg = sqrt( square(SlipRot.Pitch) + square(SlipRot.Yaw) + square(SlipRot.roll) ) * 360.0 / 65536.0;
			SlipRot *= MaxSlipDeg / (MaxSlipDeg + SlipRotDeg);
			NewRot += SlipRot;
		}
		RotVelocity = (RotVelocity + (NewRot - Rotation) / LastTick) * 0.6;
		setRotation(NewRot);

		// ------- Draw 3D Helmet --------
		// This is now drawn in UnHelmet.cpp
		//C.DrawActor(self, false, false, DisplayFOV);
	}

}

simulated function PostRender(Canvas C)
{	
	//gdr Preserve this behavior in single player but ignore it in multiplayer
	if( Level.NetMode == NM_Standalone && (PlayerOwner == None || PlayerOwner.bBehindView ))
		return;
	
    Super.PostRender(C);

	return;
}


defaultproperties
{
     DisplayFOV=84
     MaxSlipDist=0.25
     MaxSlipDeg=0.5
     UnslipRate=15
     ScavDroidMaxRange=1000
     ScavStaticGamma=1.5
     FlashlightTexture=Texture'HUDTextures.Helmet.HudFlashlightProjector'
     DrawType=DT_StaticMesh
     StaticMesh=StaticMesh'HUDMeshes.HelmetSurface.Helmet'
     bAcceptsProjectors=False
     AmbientGlow=20
}

