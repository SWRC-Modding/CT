//=============================================================================
// Emitter: An Unreal Sprite Particle Emitter.
//=============================================================================
class SpriteEmitter extends ParticleEmitter
	native;


enum EParticleDirectionUsage
{
	PTDU_None,
	PTDU_Up,	
	PTDU_Forward,
	PTDU_Normal,
	PTDU_UpAndNormal,	
};


var (Sprite)		EParticleDirectionUsage		UseDirectionAs;
var (Sprite)		vector						ProjectionNormal;

var transient		vector						RealProjectionNormal;


defaultproperties
{
     ProjectionNormal=(Z=1)
}

