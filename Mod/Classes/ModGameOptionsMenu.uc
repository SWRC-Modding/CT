class ModGameOptionsMenu extends MenuTemplateTitled;

var() MenuText			Label;

var() MenuSprite		OptionsBorder;
var() MenuSprite		OptionsDescBorder;

var() MenuText			OptionDesc;

const NUM_OPTIONS = 13;

var() MenuText			OptionLabels[NUM_OPTIONS];
var() MenuButtonEnum	Options[NUM_OPTIONS];
var() MenuButtonSprite  OptionLeftArrows[NUM_OPTIONS];
var() MenuButtonSprite	OptionRightArrows[NUM_OPTIONS];

var() int				OptionDefaults[NUM_OPTIONS];

var() MenuText			GameLabel;
var() MenuSprite		GameLabelBackground;
var() MenuSprite		GameLabelConnector;

var() MenuButtonText	Sound;
var() MenuButtonText	Graphics;
var() MenuButtonText	Controls;

var() MenuButtonText	RestoreToDefault;
var() MenuSprite		DefaultConnector;
var() MenuSprite		DefaultLine;

var() MenuButtonText	Done;

var() bool				bInMultiplayer;

var SWRCFix             SWRCFix;

simulated function Init( string Args )
{
	Super.Init( Args );

	if ( Caps(GetPlayerOwner().GetLanguage()) == "EST" ||
		 Caps(GetPlayerOwner().GetLanguage()) == "DET" )
	{
		Label.ScaleX = 1.5;
	}

	bInMultiplayer = Level.NetMode != NM_StandAlone;

	SWRCFix = SWRCFix(FindObject("Transient.SWRCFixInstance", class'SWRCFix'));

	if(SWRCFix == None)
		Warn("SWRCFix OBJECT NOT FOUND!!!!!");

	Refresh();
}

simulated function DisableOption( int i )
{
	Options[i].bDisabled = 1;
	Options[i].Blurred.DrawColor.R = 128;
	Options[i].Blurred.DrawColor.G = 128;
	Options[i].Blurred.DrawColor.B = 128;
	Options[i].BackgroundBlurred.DrawColor.R = 128;
	Options[i].BackgroundBlurred.DrawColor.G = 128;
	Options[i].BackgroundBlurred.DrawColor.B = 128;

	OptionLabels[i].DrawColor.R = 128;
	OptionLabels[i].DrawColor.G = 128;
	OptionLabels[i].DrawColor.B = 128;

	OptionLeftArrows[i].bDisabled = 1;
	OptionLeftArrows[i].Blurred.DrawColor.R = 128;
	OptionLeftArrows[i].Blurred.DrawColor.G = 128;
	OptionLeftArrows[i].Blurred.DrawColor.B = 128;

	OptionRightArrows[i].bDisabled = 1;
	OptionRightArrows[i].Blurred.DrawColor.R = 128;
	OptionRightArrows[i].Blurred.DrawColor.G = 128;
	OptionRightArrows[i].Blurred.DrawColor.B = 128;
}

simulated function EnableOption( int i )
{
	Options[i].bDisabled = 0;
	Options[i].Blurred.DrawColor.R = ButtonEnumStyle1.Blurred.DrawColor.R;
	Options[i].Blurred.DrawColor.G = ButtonEnumStyle1.Blurred.DrawColor.G;
	Options[i].Blurred.DrawColor.B = ButtonEnumStyle1.Blurred.DrawColor.B;
	Options[i].BackgroundBlurred.DrawColor.R = ButtonEnumStyle1.BackgroundBlurred.DrawColor.R;
	Options[i].BackgroundBlurred.DrawColor.G = ButtonEnumStyle1.BackgroundBlurred.DrawColor.G;
	Options[i].BackgroundBlurred.DrawColor.B = ButtonEnumStyle1.BackgroundBlurred.DrawColor.B;

	OptionLabels[i].DrawColor.R = LabelText.DrawColor.R;
	OptionLabels[i].DrawColor.G = LabelText.DrawColor.G;
	OptionLabels[i].DrawColor.B = LabelText.DrawColor.B;

	OptionLeftArrows[i].bDisabled = 0;
	OptionLeftArrows[i].Blurred.DrawColor.R = 255;
	OptionLeftArrows[i].Blurred.DrawColor.G = 255;
	OptionLeftArrows[i].Blurred.DrawColor.B = 255;

	OptionRightArrows[i].bDisabled = 0;
	OptionRightArrows[i].Blurred.DrawColor.R = 255;
	OptionRightArrows[i].Blurred.DrawColor.G = 255;
	OptionRightArrows[i].Blurred.DrawColor.B = 255;
}

simulated function int SensitivityToScale( float Sensitivity )
{
	local int Scale;

	Scale = int(Sensitivity / 0.5);

	return Scale;
}

simulated function float ScaleToSensitivity( int Scale )
{
	local float Sensitivity;

	Sensitivity = float(Scale) * 0.5;

	return Sensitivity;
}

simulated function Refresh()
{
	local int i;
	local float diff;
	local float prevDiff;
	local float fpsLimit;

	i = SensitivityToScale( GetPlayerOwner().GetMouseSensitivity() );
	Options[0].Current = i - 1;

	if ( GetPlayerOwner().GetInvertLook() )
		Options[1].Current = 0;
	else
		Options[1].Current = 1;

	// Look for current fps limit in list and get the index. If it is a custom one set via ini or console insert it into the options array.
	// There will never be more than one custom value in the list at a time as this is only done to display the correct one even if it is not one of the default options.

	fpsLimit = SWRCFix.FpsLimit;

	for(i = 0; i < Options[9].Items.Length;  ++i){
		diff = float(Options[9].Items[i]) - fpsLimit;

		if(Abs(diff) <= 0.1) // Tolerance to deal with floating point precision
			break;

		if(diff > 0.0 && prevDiff <= 0.0){
			Options[9].Items.Insert(i, 1);
			Options[9].Items[i] = string(fpsLimit);

			break;
		}

		prevDiff = diff;
	}

	if(i == Options[9].Items.Length)
		Options[9].Items[Options[9].Items.Length] = string(fpsLimit);

	Options[9].Current = i;

	if(!SWRCFix.AutoFOV){
		// Same procedure for FOV
		for(i = 0; i < Options[10].Items.Length;  ++i){
			diff = float(Options[10].Items[i]) - class'SWRCFix'.default.FOV;

			if(Abs(diff) <= 0.1) // Tolerance to deal with floating point precision
				break;

			if(diff > 0.0 && prevDiff <= 0.0){
				Options[10].Items.Insert(i, 1);
				Options[10].Items[i] = string(class'SWRCFix'.default.FOV);

				break;
			}

			prevDiff = diff;
		}

		if(i == Options[10].Items.Length){
			SWRCFix.AutoFOV = true;
			Options[10].Current = 0;
		}else{
			Options[10].Current = i;
		}
	}else{
		Options[10].Current = 0;
	}

	Options[11].Current = SWRCFix.HudArmsFOVFactor * 10;
	Options[12].Current = SWRCFix.ViewShake * 10;

	if ( !bInMultiplayer )
	{
		if ( GetPlayerOwner().myHUD.bShowSubtitles )
			Options[2].Current = 0;
		else
			Options[2].Current = 1;

		if ( GetPlayerOwner().bVisor )
			Options[3].Current = 0;
		else
			Options[3].Current = 1;

		if ( GetPlayerOwner().myHUD.bShowPromptText )
			Options[4].Current = 0;
		else
			Options[4].Current = 1;

		if ( GetPlayerOwner().GetDifficultyLevel() >= 0 )
		{
			Options[5].Current = GetPlayerOwner().GetDifficultyLevel();
		}
		else
		{
			log("GetDifficultyLevel returned -1");
			Options[5].Current = 1;
		}

		if ( GetPlayerOwner().bAutoPullManeuvers )
			Options[6].Current = 0;
		else
			Options[6].Current = 1;

		Options[7].Current = GetPlayerOwner().VisorModeDefault;

		if ( GetPlayerOwner().VisorModeDefault == 1 )
			DisableOption( 8 );

		Options[8].Current = int(GetPlayerOwner().TacticalModeIntensity * 10.0);
	}
	else
	{
		DisableOption( 2 );
		DisableOption( 3 );
		DisableOption( 4 );
		DisableOption( 5 );
		DisableOption( 6 );
		DisableOption( 7 );
		DisableOption( 8 );

		// Disable FPS limit option for clients if the engine caps the frame rate
		if(Level.NetMode == NM_Client){
			if(Level.bCapFramerate)
				DisableOption( 9 );
		}
	}
}

simulated function RestoreDefaults()
{
	local int i;

	for ( i = 0; i < NUM_OPTIONS; ++i )
	{
		Options[i].Current = OptionDefaults[i];
		ChangeOption(i, 0);
	}
}

simulated function ChangeOption( int i, int Delta )
{
    local int NewItem;

	NewItem = Options[i].Current + Delta;

	if( NewItem >= Options[i].Items.Length )
	{
		if( Options[i].bNoWrap == 0 )
			NewItem = 0;
		else
			NewItem = Options[i].Items.Length - 1;
	}
	else if( NewItem < 0 )
	{
		if( Options[i].bNoWrap == 0 )
			NewItem = Options[i].Items.Length - 1;
		else
			NewItem = 0;
	}

	Options[i].Current = NewItem;

    switch( i )
    {
		case 0:
			GetPlayerOwner().SetMouseSensitivity( ScaleToSensitivity(Options[i].Current + 1) );
			GetPlayerOwner().SavePlayerInputConfig();
			break;

		case 1:
			if ( Options[i].Current == 0 )
				GetPlayerOwner().SetInvertLook(True);
			else
				GetPlayerOwner().SetInvertLook(False);
			GetPlayerOwner().SavePlayerInputConfig();
			break;

        case 2:
			if ( Options[i].Current == 0 )
				GetPlayerOwner().myHUD.bShowSubtitles = True;
			else
				GetPlayerOwner().myHUD.bShowSubtitles = False;
			GetPlayerOwner().myHUD.SaveConfig();
            break;

		case 3:
			if ( Options[i].Current == 0 )
				GetPlayerOwner().bVisor = True;
			else
				GetPlayerOwner().bVisor = False;
			GetPlayerOwner().SaveConfig();
			break;

        case 4:
			if ( Options[i].Current == 0 )
				GetPlayerOwner().myHUD.bShowPromptText = True;
			else
				GetPlayerOwner().myHUD.bShowPromptText = False;
			GetPlayerOwner().myHUD.SaveConfig();
            break;

        case 5:
			GetPlayerOwner().SetDifficultyLevel( Options[i].Current );
			GetPlayerOwner().SaveConfig();
			break;

		case 6:
			if ( Options[i].Current == 0 )
				GetPlayerOwner().bAutoPullManeuvers = True;
			else
				GetPlayerOwner().bAutoPullManeuvers = False;
			GetPlayerOwner().SaveConfig();
			break;

		case 7:
			GetPlayerOwner().SetVisorModeDefault( Options[i].Current );
			GetPlayerOwner().SaveConfig();

			if ( Options[i].Current == 1 )
				DisableOption( 8 );
			else
				EnableOption( 8 );

			break;

		case 8:
			GetPlayerOwner().TacticalModeIntensity = float(Options[i].Current) / 10.0f;
			GetPlayerOwner().SaveConfig();
			GetPlayerOwner().PassOnTacticalIntensity();
			break;

		case 9:
			SWRCFix.FpsLimit = float(Options[9].Items[Options[9].Current]);
			SWRCFix.SaveConfig();
			break;

		case 10:
			if(Options[10].Current != 0){
				SWRCFix.FOV = float(Options[10].Items[Options[10].Current]); // The FOV is automatically set if it changed
				SWRCFix.AutoFOV = false;
			}else{
				SWRCFix.AutoFOV = true;
			}
			break;

		case 11:
			SWRCFix.HudArmsFOVFactor = float(Options[11].Items[Options[11].Current]);
			break;

		case 12:
			SWRCFix.ViewShake = float(Options[12].Items[Options[12].Current]);
			SWRCFix.SaveConfig();
	}

	GetPlayerOwner().PropagateSettings();
}

simulated function OnLeft()
{
    local int i;

    for( i = 0; i < NUM_OPTIONS; i++)
    {
        if( ( Options[i].bHasFocus != 0 ) ||
			( OptionLeftArrows[i].bHasFocus != 0 ) ||
			( OptionRightArrows[i].bHasFocus != 0 ) )
        {
			ChangeOption( i, -1 );
			return;
        }
    }
    log( "Got spurious OnLeft()", 'Error' );
}

simulated function OnRight()
{
    local int i;

    for( i = 0; i < NUM_OPTIONS; i++)
    {
        if( ( Options[i].bHasFocus != 0 ) ||
			( OptionLeftArrows[i].bHasFocus != 0 ) ||
			( OptionRightArrows[i].bHasFocus != 0 ) )
        {
			ChangeOption( i, 1 );
			return;
        }
    }
    log( "Got spurious OnRight()", 'Error' );
}

simulated function SoundSelected()
{
	GotoMenuClass("XInterfaceCTMenus.CTSoundOptionsPCMenu");
}

simulated function GraphicsSelected()
{
	GotoMenuClass("XInterfaceCTMenus.CTGraphicsOptionsPCMenu");
}

simulated function ControlsSelected()
{
	GotoMenuClass("XInterfaceCTMenus.CTControlsOptionsPCMenu");
}

simulated function RestoreToDefaultSelected()
{
	RestoreDefaults();
}

simulated function DoneSelected()
{
	CloseMenu();
}

defaultproperties
{
     Label=(MenuFont=Font'OrbitFonts.OrbitBold15',Text="OPTIONS",DrawPivot=DP_MiddleLeft,PosX=0.05375,PosY=0.078333,ScaleX=1.75,ScaleY=1.75,Style="LabelText")
     OptionsBorder=(PosX=0.63125,PosY=0.405,ScaleX=0.65,ScaleY=0.68666,Style="BorderStyle1")
     OptionsDescBorder=(PosX=0.63125,PosY=0.858333,ScaleX=0.65,ScaleY=0.1666,Style="BorderStyle1")
     OptionLabels(0)=(MenuFont=Font'OrbitFonts.OrbitBold15',Text="MOUSE SENSITIVITY",DrawPivot=DP_MiddleLeft,PosX=0.34,PosY=0.12,ScaleX=0.6,ScaleY=0.6,Pass=2,Style="LabelText")
     OptionLabels(1)=(Text="INVERT MOUSE",PosY=0.16)
     OptionLabels(2)=(Text="SUBTITLES")
     OptionLabels(3)=(Text="HELMET")
     OptionLabels(4)=(Text="PROMPT TEXT")
     OptionLabels(5)=(Text="DIFFICULTY")
     OptionLabels(6)=(Text="AUTO PULL MANEUVERS")
     OptionLabels(7)=(Text="TACTICAL VISOR MODE")
     OptionLabels(8)=(Text="TACTICAL MODE INTENSITY")
     OptionLabels(9)=(Text="FPS LIMIT")
     OptionLabels(10)=(Text="FIELD OF VIEW")
     OptionLabels(11)=(Text="WEAPON FOV FACTOR")
     OptionLabels(12)=(Text="VIEW SHAKE")
     Options(0)=(Items=("1","2","3","4","5","6","7","8","9","10"),Blurred=(PosX=0.77375,PosY=0.12,ScaleX=0.6,ScaleY=0.6),BackgroundBlurred=(PosX=0.77375,PosY=0.12,ScaleX=0.26,ScaleY=0.02666),OnLeft="OnLeft",OnRight="OnRight",Pass=2,Style="ButtonEnumStyle1")
     Options(1)=(Items=("YES","NO"),Blurred=(PosY=0.16),BackgroundBlurred=(PosY=0.16))
     Options(2)=(Items=("ON","OFF"))
     Options(3)=(Items=("ON","OFF"))
     Options(4)=(Items=("ON","OFF"))
     Options(5)=(Items=("EASY","MEDIUM","HARD"))
     Options(6)=(Items=("YES","NO"))
     Options(7)=(Items=("ON","OFF","CYCLE"))
     Options(8)=(Items=("0","1","2","3","4","5","6","7","8","9","10"))
     Options(9)=(Items=("NONE","30","60","75","100","120","144","240","360"));
     Options(10)=(Items=("AUTO","85","90","95","100","105","110","115","120","125","130"))
     Options(11)=(Items=("0.0","0.1","0.2","0.3","0.4","0.5","0.6","0.7","0.8","0.9","1.0"))
     Options(12)=(Items=("0.0","0.1","0.2","0.3","0.4","0.5","0.6","0.7","0.8","0.9","1.0"))
     OptionLeftArrows(0)=(Blurred=(WidgetTexture=Texture'GUIContent.Menu.CT_ArrowLeft',DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,PosX=0.62625,PosY=0.12,ScaleX=0.5,ScaleY=0.5),Focused=(DrawColor=(B=255,G=255,R=255,A=255),ScaleX=0.65,ScaleY=0.65),bIgnoreController=1,OnSelect="OnLeft",Pass=2)
     OptionLeftArrows(1)=(Blurred=(PosX=0.62625,PosY=0.16))
     OptionLeftArrows(2)=(Blurred=(PosX=0.62625))
     OptionLeftArrows(3)=(Blurred=(PosX=0.62625))
     OptionLeftArrows(4)=(Blurred=(PosX=0.62625))
     OptionLeftArrows(5)=(Blurred=(PosX=0.62625))
     OptionLeftArrows(6)=(Blurred=(PosX=0.62625))
     OptionLeftArrows(7)=(Blurred=(PosX=0.62625))
     OptionLeftArrows(8)=(Blurred=(PosX=0.62625))
     OptionLeftArrows(9)=(Blurred=(PosX=0.62625))
     OptionLeftArrows(10)=(Blurred=(PosX=0.62625))
     OptionLeftArrows(11)=(Blurred=(PosX=0.62625))
     OptionLeftArrows(12)=(Blurred=(PosX=0.62625))
     OptionRightArrows(0)=(Blurred=(WidgetTexture=Texture'GUIContent.Menu.CT_ArrowRight',DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,PosX=0.9225,PosY=0.12,ScaleX=0.5,ScaleY=0.5),Focused=(DrawColor=(B=255,G=255,R=255,A=255),ScaleX=0.65,ScaleY=0.65),bIgnoreController=1,OnSelect="OnRight",Pass=2)
     OptionRightArrows(1)=(Blurred=(PosX=0.9225,PosY=0.16))
     OptionRightArrows(2)=(Blurred=(PosX=0.9225))
     OptionRightArrows(3)=(Blurred=(PosX=0.9225))
     OptionRightArrows(4)=(Blurred=(PosX=0.9225))
     OptionRightArrows(5)=(Blurred=(PosX=0.9225))
     OptionRightArrows(6)=(Blurred=(PosX=0.9225))
     OptionRightArrows(7)=(Blurred=(PosX=0.9225))
     OptionRightArrows(8)=(Blurred=(PosX=0.9225))
     OptionRightArrows(9)=(Blurred=(PosX=0.9225))
     OptionRightArrows(10)=(Blurred=(PosX=0.9225))
     OptionRightArrows(11)=(Blurred=(PosX=0.9225))
     OptionRightArrows(12)=(Blurred=(PosX=0.9225))
     OptionDefaults(0)=5
     OptionDefaults(1)=1
     OptionDefaults(2)=1
     OptionDefaults(5)=1
     OptionDefaults(7)=2
     OptionDefaults(8)=2
     OptionDefaults(11)=10
     OptionDefaults(12)=10
     GameLabel=(MenuFont=Font'OrbitFonts.OrbitBold15',Text="GAME",DrawColor=(A=255),DrawPivot=DP_MiddleLeft,PosX=0.05375,PosY=0.2,ScaleX=1,ScaleY=0.8,Pass=2)
     GameLabelBackground=(WidgetTexture=Texture'GUIContent.Menu.CT_ButtonFocus',DrawColor=(B=255,G=255,R=255,A=192),DrawPivot=DP_MiddleLeft,PosX=0.04375,PosY=0.2,ScaleX=0.245,ScaleY=0.04333,ScaleMode=MSCM_FitStretch,Pass=1)
     GameLabelConnector=(WidgetTexture=Texture'GUIContent.Menu.CT_ButtonFocus',DrawColor=(B=255,G=255,R=255,A=192),DrawPivot=DP_MiddleLeft,PosX=0.295,PosY=0.2,ScaleX=0.005,ScaleY=0.04333,ScaleMode=MSCM_FitStretch)
     Sound=(Blurred=(Text="SOUND",DrawPivot=DP_MiddleLeft,PosX=0.05375,PosY=0.25),BackgroundBlurred=(DrawPivot=DP_MiddleLeft,PosX=0.04375,PosY=0.25,ScaleX=0.245,ScaleY=0.04333),OnSelect="SoundSelected",Style="ButtonTextStyle1")
     Graphics=(Blurred=(Text="GRAPHICS",DrawPivot=DP_MiddleLeft,PosX=0.05375,PosY=0.3),BackgroundBlurred=(DrawPivot=DP_MiddleLeft,PosX=0.04375,PosY=0.3,ScaleX=0.245,ScaleY=0.04333),OnSelect="GraphicsSelected",Style="ButtonTextStyle1")
     Controls=(Blurred=(Text="CONTROLS",DrawPivot=DP_MiddleLeft,PosX=0.05375,PosY=0.35),BackgroundBlurred=(DrawPivot=DP_MiddleLeft,PosX=0.04375,PosY=0.35,ScaleX=0.245,ScaleY=0.04333),OnSelect="ControlsSelected",Style="ButtonTextStyle1")
     RestoreToDefault=(Blurred=(Text="RESTORE DEFAULTS",DrawPivot=DP_MiddleLeft,PosX=0.05375,PosY=0.658333,ScaleX=0.6,ScaleY=0.6),BackgroundBlurred=(DrawPivot=DP_MiddleLeft,PosX=0.04375,PosY=0.658333,ScaleX=0.245,ScaleY=0.04333,ScaleMode=MSCM_FitStretch),OnSelect="RestoreToDefaultSelected",Style="ButtonTextStyle1")
     DefaultConnector=(WidgetTexture=Texture'GUIContent.Menu.CT_ButtonFocus',DrawColor=(B=255,G=255,R=255,A=192),DrawPivot=DP_MiddleLeft,PosX=0.295,PosY=0.688333,ScaleX=0.005,ScaleY=0.02,ScaleMode=MSCM_FitStretch)
     DefaultLine=(WidgetTexture=Texture'GUIContent.Menu.CT_ButtonFocus',DrawColor=(B=255,G=255,R=255,A=192),DrawPivot=DP_MiddleLeft,PosX=0.04375,PosY=0.688333,ScaleX=0.245,ScaleY=0.02,ScaleMode=MSCM_FitStretch)
     Done=(Blurred=(Text="DONE",DrawPivot=DP_MiddleLeft,PosX=0.05375,PosY=0.921666),BackgroundBlurred=(DrawPivot=DP_MiddleLeft,PosX=0.04375,PosY=0.921666,ScaleX=0.245,ScaleY=0.04333),OnSelect="DoneSelected",Style="ButtonTextStyle1")
     Background=(bHidden=1)
}
