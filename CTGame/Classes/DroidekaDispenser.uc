//=============================================================================
// Droideka Dispensor
//
// Created: 2004, February 25, Jenny Huang
//=============================================================================

class DroidekaDispenser extends DroidDispenser
	placeable;



defaultproperties
{
     WarmUpTime=2
     WarmUpInfo=(TransitionMaterial=Shader'DispensersTextures.DroidekaDispenser.DroidekaDispenser_Warm')
     OnlineInfo=(TransitionMaterial=Shader'DispensersTextures.DroidekaDispenser.DroidekaDispenser_On')
     ShutdownInfo=(TransitionMaterial=Shader'DispensersTextures.DroidekaDispenser.DroidekaDispenser_Off')
     DamagedInfo=(TransitionEffect=Class'CTEffects.Dispenser_Explosion',TransitionMaterial=Combiner'DispensersTextures.DroidekaDispenser.DroidekaDispenser_Dest')
     BlockerMesh=StaticMesh'Weapons.Accessories.DroidekaDispLeg'
     BlockerBones(0)="knee_L"
     BlockerBones(1)="knee_R"
     BlockerBones(2)="knee_B"
     AttachBones(0)="droidAttach1"
     AttachBones(1)="droidAttach2"
     AttachBones(2)="droidAttach3"
     AttachBones(3)="droidAttach4"
     DispenseAnims(0)="Dispense1"
     DispenseAnims(1)="Dispense2"
     DispenseAnims(2)="Dispense3"
     DispenseAnims(3)="Dispense4"
     Mesh=SkeletalMesh'Dispensers.DroidekaDispenser'
     CollisionRadius=170
     CollisionHeight=220
}

