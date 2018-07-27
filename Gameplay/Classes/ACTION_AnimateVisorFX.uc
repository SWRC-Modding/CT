class ACTION_AnimateVisorFX extends ScriptedAction;

var(Action) color		AddColor;
var(Action) color		MultiplyColor;
var(Action) int			Blur;
var(Action) int			BloomFilter;

var(Action) float		InTime;
var(Action) float		SustainTime;
var(Action) float		OutTime;

var(Action) bool		AffectAddColor;
var(Action) bool		AffectMultiplyColor;
var(Action) bool		AffectBlur;
var(Action) bool		AffectBloomFilter;

function bool InitActionFor(ScriptedController C)
{
	local PlayerController Player;
	ForEach C.DynamicActors( class 'PlayerController', Player )
	{
		if( Player.myHUD != None )
		{
			if( AffectAddColor )
				Player.BlendInAddColor( AddColor, InTime, SustainTime, OutTime );

			if( AffectMultiplyColor )
				Player.BlendInMultColor( MultiplyColor, InTime, SustainTime, OutTime );

			if( AffectBlur )
				Player.AnimateBlur( Blur, InTime, SustainTime, OutTime );

			if( AffectBloomFilter )
				Player.AnimateBloomFilter( BloomFilter, InTime, SustainTime, OutTime );
		}
	}

	return false;	
}


defaultproperties
{
}

