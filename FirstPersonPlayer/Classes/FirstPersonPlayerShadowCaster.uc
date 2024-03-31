class FirstPersonPlayerShadowCaster extends Pawn;

var ShadowProjector Shadow;

function PostBeginPlay(){
	Shadow = Spawn(class'ShadowProjector',self,'',Location);
	Shadow.ShadowActor = self;
	Shadow.LightDirection = Normal(vect(1,1,3));
	Shadow.LightDistance = 380;
	Shadow.MaxTraceDistance = 350;
	Shadow.InitShadow();
}

function Destroyed(){
	Shadow.Destroy();
	Super.Destroyed();
}

function Tick(float DeltaTime){}
function SetAnimAction(name NewAction){}
function PawnPlayFiring(name animName, bool bLoop){}
function PawnStopFiring(){}
function PlayTakeHit(vector HitLoc, int Damage, class<DamageType> damageType){}
function PlayBreathe(optional name animName){}
function PlayFalling(){}
function RelaxedStanding(){}
function AnimateStanding(){}
function AnimateCrouching(){}
function PlayDodging(bool bRight){}
function PlayDyingAnim(class<DamageType> DamageType, vector HitLoc){}

/* Skins(0)=FinalBlend'HudTextures.Effects.DefaultInvisible' */
defaultproperties
{
	DrawType=DT_Mesh
	Mesh=SkeletalMesh'Clone.CloneCommando'
	bActorShadows=True
	bCollideActors=False
	bCollideWorld=False
	bBlockActors=False
	bBlockPlayers=False
	bProjTarget=False
	bBlockZeroExtentTraces=False
	bBlockNonZeroExtentTraces=False
	bUseCylinderCollision=False
	bBlockKarma=False
	ControllerClass=None
}
