//=============================================================================
// BattleDroid Dispensor
//
// Created: 2004, February 12, Jenny Huang
//=============================================================================

class BDDispenser extends DroidDispenser
	placeable;



defaultproperties
{
     NumDispensers=3
     NumInitDroids=3
     DamagedInfo=(TransitionEffect=Class'CTEffects.Dispenser_Explosion',TransitionMaterial=Combiner'DispensersTextures.BDDispenser.BDDispenser_Dest')
     IntroAnim="entranceSky"
     AttachBones(0)="droidAttach1"
     AttachBones(1)="droidAttach2"
     AttachBones(2)="droidAttach3"
     AttachBones(3)="droidAttach4"
     DispenseAnims(0)="Dispense1"
     DispenseAnims(1)="Dispense2"
     DispenseAnims(2)="Dispense3"
     DispenseAnims(3)="Dispense4"
     Mesh=SkeletalMesh'Dispensers.BDDispenser'
     CollisionRadius=170
     CollisionHeight=125
}

