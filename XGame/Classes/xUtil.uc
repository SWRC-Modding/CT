class xUtil extends Object
    /* SBD Removed *** exportstructs*/
    native;

struct GameTypeRecord
{
    var() String GameName;
    var() String ClassName;
    var() String MapPrefix;
    var() String Acronym;
    var() String MapListType;
    var() String ScreenShotName;
    var() Material Screenshot;
    var() String DecoTextName;
    var() int DefaultGoalScore;
    var() int MinGoalScore;
    var() int DefaultTimeLimit;    
    var() int DefaultRemainingRounds;
    var() int bTeamGame;
};

struct MapRecord
{
    var() String LongName;
    var() String ScreenShotName;
    var() Material Screenshot;
    var() String TextName;
    var() String Filename;
    var() int IdealPlayerCountMin;
    var() int IdealPlayerCountMax;
    var() int GoalScore;
    var() int TimeLimit;
    var() int FragLimit;
    var() int MapIndex;
    var() String SupportedGames;
	var() byte bSplitScreenOK;
	var() byte bNetworkOK;
};

struct WeaponRecord
{
    var() string WeaponClassName;
    var() string FriendlyName;
    var() string AttachmentMeshName;
    var() float  AttachmentDrawScale;
    var() string PickupMeshName;
    var() byte   Priority;
    var() byte   ExchangeFireModes;
};

struct MutatorRecord
{
    var() string            ClassName;
    var() class<Mutator>    MutClass; // not filled in by GetMutatorList()
    var() string            IconMaterialName;
    var() Material          IconMaterial; // not filled in by GetMutatorList()
    var() string            ConfigMenuClassName;
    var() string            GroupName;
    var() int               SinglePlayerOnly;
    var() int               OnByDefault;
    var() localized string  FriendlyName;
    var() localized string  Description;
    var() byte              bActivated;
};

//enum EWepAffinityType
//{
//    WepAff_Damage,
//    WepAff_Ammo,
//    WepAff_FireRate,
//    WepAff_Accuracy,
//};
//
//struct WepAffinityData
//{
//    var() string            WepString;
//    var() class<Weapon>     WepClass;
//    var() EWepAffinityType  Type;
//    var() float             Value;
//};

//enum ESpecies   // character species, used for triggering race-specific combos
//{
//    SPECIES_Alien,
//    SPECIES_Bot,
//	SPECIES_Egypt,
//    SPECIES_Jugg,
//	SPECIES_Merc,
//    SPECIES_Night,
//    SPECIES_None
//};

//TODO: comment on how to add data to this
struct PlayerRecord
{
    var() String                    DefaultName;            // Character's name, also used as selection tag
//    var() ESpecies                  Species;                // Species
    var() String                    MeshName;               // Mesh type
    var() String                    BodySkinName;           // Body texture name
    var() String                    FaceSkinName;           // Face texture name
    var() String                    SoundGroupClassName;    // Sound Group name
    var() String                    GibGroupClassName;      // Gib Group name
//    var() WepAffinityData           WepAffinity;            // Weapon affinity
    var() String                    PortraitName;           // Menu picture file-name.
    var() Material                  Portrait;
    var() String                    TextName;               // Decotext reference
    var() String                    ClassName;              // Not used, but might be of use to others
    var() String                    SkeletonMeshName;       // Mesh to swap in when skeletized by the ion cannon blast
    var() int                       Source;                 // L1=0x01, L2=0x02, L3=0x04, extra=0x08
    var() String                    VoiceClassName;         // voice pack class name
    var() const int                 RecordIndex;
    var() const byte                bLoaded;
};

//enum EINTPresets
//{
//    EIP_L1,         // League 1
//    EIP_L123,       // Leagues 1, 2 & 3
//    EIP_L1AndExt,   // League 1 & any extra
//    EIP_L123AndExt, // Leagues 1, 2 & 3 & any extra
//};

//var() string LeaguePlayerRecords[3];

var Array<string> DamagedContentNames;

native		final simulated static function int		GetDamagedContentCount();
native		final simulated static function string	GetDamagedContentName(int index);
native		final simulated static function			ClearDamagedContent();

native(560) final simulated static function GetGameTypeList(out array<GameTypeRecord> GameTypeRecords);
native      final simulated static function GetGameTypeRecord(out GameTypeRecord Record, String GameTypeName, optional bool LoadResources);
native(561) final simulated static function GetMapList(out array<MapRecord> MapRecords, optional String MapPrefix, optional bool SplitscreenOnly, optional int MapIndex );
native      final simulated static function GetMapRecord(out MapRecord Record, String MapName, optional bool LoadResources);
native(562) final simulated static function GetPlayerList(out array<PlayerRecord> PlayerRecords);
native(563) final simulated static function PlayerRecord GetPlayerRecord(int index);
native(575) final simulated static function PlayerRecord GetRandPlayerRecord(optional bool bL1Only);
native(564) final simulated static function PlayerRecord FindPlayerRecord(string charName);
native final simulated static function PlayerRecord CheckLoadLimits(LevelInfo Info, int index);
native(569) final simulated static function LoadPlayerRecordResources(int index);
//native(570) final simulated static function GetWeaponList(out array<WeaponRecord> WeaponRecords);
//native(571) final simulated static function WeaponRecord FindWeaponRecord(string weaponName);
//native(572) final simulated static function UpdateWeaponRecord(WeaponRecord record);
native(573) final simulated static function GetMutatorList(out array<MutatorRecord> MutatorRecords);

//native(565) final static function bool CorrectAutoAim(out vector newAim, vector start, vector fireDir, Pawn target, 
//                                                      vector aimSpot, float maxHoffset, float maxVoffset, float hitOffsetRatio);
// 
//native(574) final static function bool LoadUserINI(PlayerController pc, string iniFileName, out string playerName, out string characterName);

//simulated static function int AllowedINTs(EINTPresets preset)
//{
//    local int allow;
//
//    switch(preset)
//    {
//        case EIP_L1:
//            allow = 0x1;
//            break;
//        case EIP_L123:
//            allow = 0x7;
//            break;
//        case EIP_L1AndExt:
//            allow = 0x9;
//            break;
//        case EIP_L123AndExt:
//        default:
//            allow = 0xf;
//    }
//
//    return allow;
//}

//simulated static function FilterPlayerRecords(out array<PlayerRecord> filteredRecords, EINTPresets filter)
//{
//    local array<PlayerRecord> allRecords;
//    local int i;
//
//    GetPlayerList(allRecords);
//
//    for (i=0; i<allRecords.Length; i++)
//    {
//        if ((allRecords[i].Source & AllowedINTs(filter)) == 0)
//            continue;
//
//        filteredRecords[filteredRecords.Length] = allRecords[i];
//    }
//}

//simulated static function string GetFavoriteWeaponName(int playerRecordIndex)
//{
//    local PlayerRecord pr;
//    local WeaponRecord wr; 
//    
//    pr = GetPlayerRecord(playerRecordIndex);
//    wr = FindWeaponRecord(pr.WepAffinity.WepString);
//    
//    return wr.FriendlyName;
//}


defaultproperties
{
}

