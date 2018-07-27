//=============================================================================
// System.
//=============================================================================
class System extends Subsystem;

defaultproperties
{
     Suppress(0)="DevLoad"
     Suppress(1)="DevSave"
     Suppress(2)="DevNetTraffic"
     Suppress(3)="DevGarbage"
     Suppress(4)="DevKill"
     Suppress(5)="DevReplace"
     Suppress(6)="DevCompile"
     Suppress(7)="DevBind"
     Suppress(8)="DevBsp"
     Paths(0)="*.u"
     Paths(1)="../Properties/*.u"
     Paths(2)="../Maps/*.ctm"
     Paths(3)="../TestMaps/*.ctm"
     Paths(4)="../Textures/*.utx"
     Paths(5)="../Sounds/*.uax"
     Paths(6)="../Music/*.umx"
     Paths(7)="../StaticMeshes/*.usx"
     Paths(8)="../Animations/*.ukx"
     Paths(9)="../Saves/*.uvx"
     SourcePath="../../Code"
     CacheExt=".uxx"
     CachePath="../Cache"
     SavePath="../Save"
     PurgeCacheDays=30
}

