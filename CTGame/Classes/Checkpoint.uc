
class Checkpoint extends Actor
	placeable
	hidecategories(AI,Collision,CollisionAdvanced,Karma,LightColor,Lighting,marker,Sound);


function Trigger( actor Other, pawn EventInstigator )
{
	GotoState('Saving');	
}

state Saving
{
Begin:
	Sleep(0.1);
	if (Level.GetLocalPlayerController().Pawn == None || Level.GetLocalPlayerController().Pawn.bIncapacitated)
		Goto('Begin');
	if ( Level.GetLocalPlayerController().CanAutoSave() )
		Level.GetLocalPlayerController().CheckpointSaveStarted();
	Sleep(0.1);
	Level.GetLocalPlayerController().AutoSave();
	GotoState('');
}


defaultproperties
{
     bHidden=True
}

