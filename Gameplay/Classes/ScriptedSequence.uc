//=============================================================================
// ScriptedSequence
// used for setting up scripted sequences for pawns.
// A ScriptedController is spawned to carry out the scripted sequence.
//=============================================================================
class ScriptedSequence extends AIScript native;

var(AIScript) bool bDrawLinks;
var(AIScript) export editinline Array<ScriptedAction> Actions;
var class<ScriptedController>  ScriptControllerClass;


// (cpptext)
// (cpptext)
// (cpptext)
// (cpptext)

/* SpawnController()
Spawn and initialize an AI Controller (called by a non-player controlled Pawn at level startup)
*/

function SpawnControllerFor(Pawn P)
{
	local AIController C;
	//Do NOT call Super class SpawnControllerFor -- it's wrong	
	
	if (P.ControllerClass != None)
	{
		C = Spawn(P.ControllerClass,,,P.Location, P.Rotation);

		C.MyScript = self;
		C.Possess(P);
	}
	TakeOver(P);
}

/* TakeOver()
Spawn a scripted controller, which temporarily takes over the actions of the pawn,
unless pawn is currently controlled by a scripted controller - then just change its script
*/
function TakeOver(Pawn P)
{
	local ScriptedController S;

	if ( P.Controller.IsA( 'ScriptedController' ) )
	{
		S = ScriptedController(P.Controller);
		S.MyScript = self;
		S.TakeControlOf(P);
		S.SetNewScript(self);
	}
	else
		Warn( "Tried to use an action script on a non-scripted controller" );
}
		
//*****************************************************************************************
// Script Changes

function bool ValidAction(Int N)
{
	return true;
}

function SetActions(ScriptedController C)
{
	local ScriptedSequence NewScript;
	local bool bDone;

	if( C.CurrentAnimation != None )
		C.CurrentAnimation.SetCurrentAnimationFor(C);

	while( !bDone )
	{
		if( C.ActionNum < Actions.Length ) // we have to have a valid action number to proceed
		{
			// NathanM: I modified this code somewhat to robustly handle empty lines in the action script	
			
			// This first block is what used to be there
			if( Actions[C.ActionNum] != None )
			{
				if( ValidAction( C.ActionNum ) )
				{
					NewScript = Actions[C.ActionNum].GetScript( self );
				}
				else
				{
					NewScript = None;
					Warn(GetItemName(string(self))$" action "$C.ActionNum@Actions[C.ActionNum].GetActionString()$" NOT VALID!!!");
				}
			}
			else // this block handles empty actions
			{
				Warn( self$" has a NULL action in slot "$C.ActionNum$"!!!");

				// if we have more actions in the script, we'll move on and hope it's not NULL
				if( C.ActionNum < Actions.Length - 1 )
				{					
					C.ActionNum++;
					continue;
				}
				else // if not, then just end the script
					NewScript = None;
			}
		}		
		else // if the action number isn't valid, then this will effectively end the script
			NewScript = None;

		if( NewScript == None )
		{			
			C.CurrentAction = None;
			return;
		}
		else if( NewScript != self )
		{			
			C.SetNewScript(NewScript);
			return;
		}		

		bDone = !Actions[C.ActionNum].bDisabled && Actions[C.ActionNum].InitActionFor(C);
		if( !bDone )
  			Actions[C.ActionNum].ProceedToNextAction(C);  		
	}
}

// Editor uses these to determine if an event or tag is used in this script
function bool ContainsPartialEvent(string StartOfEventName)
{
	local int i;
	for (i = 0; i < Actions.Length; ++i)
	{
		if (Actions[i].ContainsPartialEvent(StartOfEventName))
			return true;
	}
	return super.ContainsPartialEvent(StartOfEventName);
}

function bool ContainsPartialTag(string StartOfTag)
{
	local int i;
	for (i = 0; i < Actions.Length; ++i)
	{
		if (Actions[i].ContainsPartialTag(StartOfTag))
			return true;
	}
	return super.ContainsPartialTag(StartOfTag);
}



cpptext
{
	void RenderEditorSelected(FLevelSceneNode* SceneNode,FRenderInterface* RI, FDynamicActor* FDA);

}

defaultproperties
{
     bDrawLinks=True
     ScriptControllerClass=Class'Gameplay.ScriptedController'
     bCollideWhenPlacing=True
     CollisionRadius=50
     CollisionHeight=100
     bDirectional=True
}

