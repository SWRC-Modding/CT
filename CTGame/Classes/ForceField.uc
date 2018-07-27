class ForceField extends Flashlight;

var()	float	ScaleSpeed;

function float TakeDamage( float Damage, Pawn EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional Name BoneName )
{
	if( EventInstigator != Owner )
		Owner.TakeDamage( Damage, EventInstigator, HitLocation, Momentum, DamageType, BoneName );

	return Damage;
}

function TurnOff()
{	
	bIsOn = false;	
	SetCollision( false, false, false );
	Enable('Tick');
}

function TurnOn()
{	
	bIsOn = true;
	bHidden = false;
	SetCollision( true, false, false );
	Enable('Tick');
}

function Tick( float DeltaTime )
{
	local float NewDrawScale;

    if( bIsOn )	
		NewDrawScale = DrawScale + ( DeltaTime * ScaleSpeed );	
	else
		NewDrawScale = DrawScale - ( DeltaTime * ScaleSpeed );
	
	NewDrawScale = FClamp( NewDrawScale, 0, 1 );
	SetDrawScale( NewDrawScale );		

	if( NewDrawScale == 0 || NewDrawScale == 1 )
		Disable('Tick');

	if( NewDrawScale == 0 )
		bHidden = true;
}


defaultproperties
{
     ScaleSpeed=5
     StaticMesh=StaticMesh'Weapons.DroidWeapons.DroidekaShield'
     bCollideActors=True
     bProjTarget=True
     bBlockZeroExtentTraces=True
     bBlockNonZeroExtentTraces=True
     TypeOfMaterial=MT_Shield
}

