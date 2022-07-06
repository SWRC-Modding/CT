class SkinChanger extends AdminService native;

var() config bool   RandomizeBotSkins;

var() array<Shader> CloneSkins;
var() array<Shader> TrandoSkins;

var float           NextSkinUpdateTime;

var int             NextBotCloneSkin;
var int             NextBotTrandoSkin;

native final function SetCloneSkin(Controller C, int SkinIndex);
native final function SetTrandoSkin(Controller C, int SkinIndex);

function bool ExecCmd(String Cmd, optional PlayerController PC){
	local int SkinIndex;
	local bool IsClone;
	local bool IsTrando;

	if(PC != None){
		if(ParseCommand(Cmd, "CHANGESKIN")){
			if(Len(Cmd) > 0){
				SkinIndex = int(Cmd);

				if(PC.Pawn != None){
					if(PC.Pawn.IsA('MPClone'))
						SetCloneSkin(PC, SkinIndex);
					else if(PC.Pawn.IsA('MPTrandoshan'))
						SetTrandoSkin(PC, SkinIndex);
				}else{
					SetCloneSkin(PC, SkinIndex);
					SetTrandoSkin(PC, SkinIndex);
				}
			}else{
				CommandFeedback(PC, "Expected skin number");
			}

			return true;
		}else if(ParseCommand(Cmd, "SHOWSKINS")){
			if(PC != None && PC.Pawn != None){
				IsClone = PC.Pawn.IsA('MPClone');
				IsTrando = PC.Pawn.IsA('MPTrandoshan');
			}

			if(IsClone || !IsTrando){
				CommandFeedback(PC, "Clone Skins:");

				for(SkinIndex = 0; SkinIndex < CloneSkins.Length; ++SkinIndex)
					CommandFeedback(PC, "- " $ SkinIndex $ ": " $ CloneSkins[SkinIndex].Diffuse.Name);
			}

			if(IsTrando || !IsClone){
				CommandFeedback(PC, "Trando Skins:");

				for(SkinIndex = 0; SkinIndex < TrandoSkins.Length; ++SkinIndex)
					CommandFeedback(PC, "- " $ SkinIndex $ ": " $ TrandoSkins[SkinIndex].Diffuse.Name);
			}

			CommandFeedback(PC, "Check console for full list of skins");

			return true;
		}
	}

	return false;
}

cpptext
{
	// Overrides
	virtual INT Tick(FLOAT DeltaTime, ELevelTick TickType);
	virtual void Spawned();

	struct FSkinEntry{
		INT NumSeenBy;
		INT CloneIndex;
		INT TrandoIndex;
	};

	static TMap<FString, FSkinEntry> SkinsByPlayerID;
	static INT                       LastSkinResetDay;
}

defaultproperties
{
	bRequiresAdminPermissions=false
	RandomizeBotSkins=true
	CloneSkins(0)=Shader'CloneTextures.CloneTextures.CloneCommando38_Shader'
	CloneSkins(1)=Shader'CloneTextures.CloneTextures.CloneCommando40_Shader'
	CloneSkins(2)=Shader'CloneTextures.CloneTextures.CloneCommando62_Shader'
	CloneSkins(3)=Shader'CloneTextures.CloneTextures.CloneCommando07_Shader'
	CloneSkins(4)=Shader'CloneTextures.CloneTextures.CloneCommandoWhite_Shader'
	CloneSkins(5)=Shader'CloneTextures.CloneTextures.MP_CloneCommandoA_Shader'
	CloneSkins(6)=Shader'CloneTextures.CloneTextures.MP_CloneCommandoB_Shader'
	CloneSkins(7)=Shader'CloneTextures.CloneTextures.MP_CloneCommandoC_Shader'
	CloneSkins(8)=Shader'CloneTextures.CloneTextures.MP_CloneCommandoD_Shader'
	TrandoSkins(0)=Shader'CloneTextures.TrandoshanMercTextures.TrandoshanMerc_Shader'
	TrandoSkins(1)=Shader'CloneTextures.TrandoshanConcTextures.TrandoshanConc_Shader'
	TrandoSkins(2)=Shader'CloneTextures.TrandoshanMercTextures.MP_TrandoshanMercA_Shader'
	TrandoSkins(3)=Shader'CloneTextures.TrandoshanMercTextures.MP_TrandoshanMercB_Shader'
	TrandoSkins(4)=Shader'CloneTextures.TrandoshanMercTextures.MP_TrandoshanMercC_Shader'
	TrandoSkins(5)=Shader'CloneTextures.TrandoshanMercTextures.MP_TrandoshanMercD_Shader'
	NextSkinUpdateTime=10.0
}
