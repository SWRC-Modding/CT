//=============================================================================
// ZoneInfo, the built-in Unreal class for defining properties
// of zones.  If you place one ZoneInfo actor in a
// zone you have partioned, the ZoneInfo defines the 
// properties of the zone.
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class ZoneInfo extends Info
	native
	hidecategories(Sound)
	placeable;

#exec Texture Import File=Textures\ZoneInfo.pcx Name=S_ZoneInfo Mips=Off MASKED=1

//-----------------------------------------------------------------------------
// Zone properties.

var() SkyZoneInfo SkyZone; // Optional sky zone containing this zone's sky.
var() name ZoneTag;
var() localized String LocationName; 

var() EMaterialType ZoneDefaultMaterial;
var() float KillZ;		// any actor falling below this level gets destroyed
var() eKillZType KillZType;	// passed by FellOutOfWorldEvent(), to allow different KillZ effects
var() bool bSoftKillZ;	// 2000 units of grace unless land

//-----------------------------------------------------------------------------
// Zone flags.
var()		bool   bTerrainZone;		// There is terrain in this zone.
var()		bool   bDistanceFog;		// There is distance fog in this zone.
var()		bool   bClearToFogColor;	// Clear to fog color if distance fog is enabled.
var()		bool   bUseSkyDome;

var const array<TerrainInfo> Terrains;

//-----------------------------------------------------------------------------
// Zone light.
var            vector AmbientVector;
var(ZoneLight) byte AmbientBrightness, AmbientHue, AmbientSaturation;

var(ZoneLight) color DistanceFogColor;
var(ZoneLight) float DistanceFogStart;
var(ZoneLight) float DistanceFogEnd;
var(ZoneLight) float DistanceFogBlendTime;

// Vision mode FX
var(ZoneFX) bool			BloomEnable;
var(ZoneFX) byte			Bloom;
var(ZoneFX) byte			BloomFilter;
var(ZoneFX) VisionMode		ZoneVisionMode;

// Splatter FX
var(ZoneFX)	float			SplattersPerMinute;
var(ZoneFX)	float			SplatterWiperDelay;
var(ZoneFX) array<Material> SplatterTextures;
var(ZoneFX) class<Emitter>	SplatterEffect;
var(ZoneFX) float			SplatterScaleMin;
var(ZoneFX) float			SplatterScaleMax;
var(ZoneFX) Sound			SplatterSound;

var(ZoneLight) const texture EnvironmentMap;
var(ZoneLight) float TexUPanSpeed, TexVPanSpeed;

var(ZoneSound) class<AudioEnvironment> ZoneAudioClass;
//------------------------------------------------------------------------------

var(ZoneVisibility) bool bLonelyZone;								// This zone is the only one to see or never seen
var(ZoneVisibility) editinline array<ZoneInfo> ManualExcludes;		// No Idea.. just sounded cool

//=============================================================================
// Iterator functions.

// Iterate through all actors in this zone.
native(308) final iterator function ZoneActors( class<actor> BaseClass, out actor Actor );

simulated function LinkToSkybox()
{
	local skyzoneinfo TempSkyZone;

	if( bUseSkyDome )
	{
		if( SkyZone == None )
		{
			// SkyZone.
			foreach AllActors( class 'SkyZoneInfo', TempSkyZone, '' )
				SkyZone = TempSkyZone;
			if(Level.DetailMode == DM_Low)
			{
				foreach AllActors( class 'SkyZoneInfo', TempSkyZone, '' )
					if( !TempSkyZone.bHighDetail && !TempSkyZone.bSuperHighDetail )
						SkyZone = TempSkyZone;
			}
			else if(Level.DetailMode == DM_High)
			{
			foreach AllActors( class 'SkyZoneInfo', TempSkyZone, '' )
					if( !TempSkyZone.bSuperHighDetail )
						SkyZone = TempSkyZone;
			}
			else if(Level.DetailMode == DM_SuperHigh)
			{
				foreach AllActors( class 'SkyZoneInfo', TempSkyZone, '' )
					SkyZone = TempSkyZone;
			}
		}
	}
	else
		SkyZone = None;
}

//=============================================================================
// Engine notification functions.

simulated function PreBeginPlay()
{
	Super.PreBeginPlay();

	// call overridable function to link this ZoneInfo actor to a skybox
	LinkToSkybox();
}

// When an actor enters this zone.
event ActorEntered( actor Other );

// When an actor leaves this zone.
event ActorLeaving( actor Other );


// The following function used as part of the editor for search functionality
function bool ContainsPartialTag(string StartOfTag)
{
	local string TagString;
	TagString = string(ZoneTag);
	if ( Caps(StartOfTag) == Left(Caps(TagString), Len(StartOfTag) ) )
	{
		return true;
	}

	return super.ContainsPartialTag(StartOfTag);
}




defaultproperties
{
     KillZ=-10000
     bUseSkyDome=True
     AmbientSaturation=255
     DistanceFogColor=(B=128,G=128,R=128)
     DistanceFogStart=3000
     DistanceFogEnd=8000
     DistanceFogBlendTime=1
     Bloom=255
     BloomFilter=200
     SplatterWiperDelay=1.5
     SplatterScaleMin=1
     SplatterScaleMax=1
     TexUPanSpeed=1
     TexVPanSpeed=1
     bStatic=True
     bNoDelete=True
     Texture=Texture'Engine.S_ZoneInfo'
}

