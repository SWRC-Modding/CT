// ====================================================================
//  Class:  CTGame.Prop
//  Parent: Engine.Ammunition
//
//  Base class for Clone Trooper Turrets
// ====================================================================

class AnimProp extends Prop
	native
	placeable;

struct PropAnimInfo
{
	var() name Anim;
	var() bool bLoop;	
};

var(Prop) PropAnimInfo AnimHealthy;
var(Prop) PropAnimInfo AnimDamaged;
var(Prop) PropAnimInfo AnimDestroyed;
var(Prop) PropAnimInfo AnimTriggered;

function Trigger( Actor Other, Pawn EventInstigator )
{
	if( AnimTriggered.Anim != '' )
	{
		if( IsPlayingAnim( AnimTriggered.Anim ) )
		{
			if( AnimTriggered.bLoop )
				LoopAnim( '' );
			else
				PlayAnim( '' );
		}
		else
		{
			if( AnimTriggered.bLoop )
				LoopAnim( AnimTriggered.Anim );
			else
				PlayAnim( AnimTriggered.Anim );
		}
	}
}

function bool OkayToDestroyAnimProp()
{
	return (AnimDestroyed.Anim == '');
}

auto state Invulnerable
{
	function BeginState()
	{
		super.BeginState();
		if( AnimHealthy.Anim != '' )
		{
			if( AnimHealthy.bLoop )
				LoopAnim( AnimHealthy.Anim );
			else
				PlayAnim( AnimHealthy.Anim );					
		}
	}
}

state() PropDamaged
{
	function BeginState()
	{
		super.BeginState();
		if( AnimDamaged.Anim != '' )
		{
			if( AnimDamaged.bLoop )
				LoopAnim( AnimDamaged.Anim );
			else
				PlayAnim( AnimDamaged.Anim );					
		}
	}
}

state() PropDestroyed
{
	function BeginState()
	{
		super.BeginState();
		if( AnimDestroyed.Anim != '' )
		{
			if( AnimDestroyed.bLoop )
				LoopAnim( AnimDestroyed.Anim );
			else
				PlayAnim( AnimDestroyed.Anim );					
		}
	}
}


defaultproperties
{
     DrawType=DT_Mesh
     bAlignBottom=True
     CollisionRadius=22
     CollisionHeight=22
}

