//=============================================================================
// AIScript - used by Level Designers to specify special AI scripts for pawns 
// placed in a level, and to change which type of AI controller to use for a pawn.
// AIScripts can be shared by one or many pawns. 
// Game specific subclasses of AIScript will have editable properties defining game specific behavior and AI
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class AIScript extends Actor
	abstract
	native	
	placeable
	hidecategories(Force,Karma,LightColor,Lighting,Sound,Collision,CollisionAdvanced,DisplayAdvanced,Movement,MovementAdvanced);

#exec Texture Import File=Textures\AIScript.dds Name=S_AIScript Mips=Off MASKED=1

var()	class<AIController> ControllerClass;
var		bool		bLoggingEnabled;	


/* SpawnController()
Spawn and initialize an AI Controller (called by a non-player controlled Pawn at level startup)
*/
function SpawnControllerFor(Pawn P)
{
	local AIController C;

	if ( ControllerClass == None )
	{
		if ( P.ControllerClass == None )
			return;
		C = Spawn(P.ControllerClass,,,P.Location, P.Rotation);
	}
	else
		C = Spawn(ControllerClass,,,P.Location, P.Rotation);
	C.MyScript = self;
	C.Possess(P);
}

function Actor GetMoveTarget()
{
	return self;
}

function TakeOver(Pawn P);


defaultproperties
{
     bStatic=True
     bHidden=True
     Texture=Texture'Engine.S_AIScript'
     CollisionRadius=10
     CollisionHeight=10
}

