class MenuTemplate extends MenuBase
    native
    abstract;

const LabelTextSpacing = 0.04f;
const ListButtonSpacing = 0.05f;
const EnumButtonSpacing = 0.06f;

// NOTE: Bad Things might happen if you don't use the var(MenuDefault) /* SBD Removed *** nonlocalized*/ group!

var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuSprite         FullScreen;     // For background pics fit to the whole screen
var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuSprite         Darken;         // For a black tint fit the whole screen
var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuSprite         Border;         // A stretched background border
var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuSprite         BlackBorder;    // A stretched background border for the Scroll bar area
var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuSprite         Image;          // Scalable image

var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuText           SmallLabelText;
var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuText           LabelText;      // Default text for labels etc
var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuText           BigText;
var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuText           StatsText;
var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuText           HugeText;

var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuText           FakeDecoText;

var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuButtonText     ListButton;     // For most of the buttons that are in the menus.. Singleplayer , multiplayer , settings, etc
var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuButtonText     PushButton;     // For "OKAY" / "BACK" buttons, etc
var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuButtonText     PushButtonWide; // For "OKAY" / "BACK" buttons also just a bit fatter for the really bigText. etc

var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuStringList     ButtonList;     // A StringList of ListButtons
var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuStringList     SmallButtonList;

var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuButtonEnum     EnumButton;     // For config options
var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuButtonEnum     HugeEnumButton; // For config options

var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuButtonSprite   UpArrow;
var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuButtonSprite   DownArrow;
var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuButtonSprite   LeftArrow;
var(MenuDefault) /*SBD Removed *** nonlocalized*/ MenuButtonSprite   RightArrow;

var(MenuDefault) /* SBD Removed *** nonlocalized*/ MenuSprite         XboxButtonA;
var(MenuDefault) /* SBD Removed *** nonlocalized*/ MenuSprite         XboxButtonB;
var(MenuDefault) /* SBD Removed *** nonlocalized*/ MenuSprite         XboxButtonX;
var(MenuDefault) /* SBD Removed *** nonlocalized*/ MenuSprite         XboxButtonY;

var(MenuDefault) /* SBD Removed *** nonlocalized*/ MenuButtonSprite   VerticalScrollBarArrowUp;
var(MenuDefault) /* SBD Removed *** nonlocalized*/ MenuButtonSprite   VerticalScrollBarArrowDown;
var(MenuDefault) /* SBD Removed *** nonlocalized*/ MenuScrollBar      VerticalScrollBar;

var(MenuDefault) /* SBD Removed *** nonlocalized*/ MenuButtonSprite   SpriteButtonBordered;       // Default bordered sprite button
var(MenuDefault) /* SBD Removed *** nonlocalized*/ MenuButtonSprite   SpriteButtonBorderedFit;   // Default bordered sprite button

var(MenuDefault) /* SBD Removed *** nonlocalized*/ Color              RedColor;
var(MenuDefault) /* SBD Removed *** nonlocalized*/ Color              BlueColor;
var(MenuDefault) /* SBD Removed *** nonlocalized*/ Color              WhiteColor;

var(MenuDefault) MenuButtonText		ButtonTextStyle1;
var(MenuDefault) MenuSprite			PlanetBorderStyle;
var(MenuDefault) MenuSprite			BorderStyle1;
var(MenuDefault) MenuSprite			BorderStyle1Clear;
var(MenuDefault) MenuSprite			BorderStyle1Opaque;
var(MenuDefault) MenuButtonEnum		ButtonEnumStyle1;
var(MenuDefault) MenuStringList		StringListStyle1;
var(MenuDefault) MenuButtonText		ButtonTextStyle2;


defaultproperties
{
     FullScreen=(DrawPivot=DP_MiddleMiddle,PosX=0.5,PosY=0.5,ScaleX=1,ScaleY=1,ScaleMode=MSCM_Fit)
     Darken=(WidgetTexture=Texture'Engine.MenuBlack',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=180),DrawPivot=DP_MiddleMiddle,PosX=0.5,PosY=0.5,ScaleX=1,ScaleY=1,ScaleMode=MSCM_Fit)
     Border=(WidgetTexture=Texture'GUIContent.Menu.BorderBoxD',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=255),ScaleMode=MSCM_FitStretch,Pass=1)
     BlackBorder=(WidgetTexture=Texture'GUIContent.Menu.BorderBoxD',RenderStyle=STY_Alpha,DrawColor=(A=175),ScaleMode=MSCM_FitStretch,Pass=1)
     Image=(WidgetTexture=Texture'Engine.TerrainBad',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,ScaleX=1,ScaleY=1,Pass=1)
     SmallLabelText=(MenuFont=Font'OrbitFonts.OrbitBold8',DrawColor=(B=160,G=160,R=160,A=255),DrawPivot=DP_MiddleMiddle,PosX=0.5,ScaleX=0.9,ScaleY=0.8)
     LabelText=(MenuFont=Font'OrbitFonts.OrbitBold15',DrawColor=(B=180,G=180,R=180,A=255),DrawPivot=DP_MiddleRight,ScaleX=1,ScaleY=0.8,Pass=1)
     BigText=(MenuFont=Font'OrbitFonts.OrbitBold15',DrawColor=(B=180,G=180,R=180,A=255),PosX=0.5,ScaleX=1,ScaleY=0.8,Pass=1)
     StatsText=(MenuFont=Font'OrbitFonts.OrbitBold8',DrawColor=(B=180,G=180,R=180,A=255),ScaleX=0.9,ScaleY=0.8,Pass=1)
     HugeText=(MenuFont=Font'OrbitFonts.OrbitBold15',DrawColor=(B=180,G=180,R=180,A=255),PosX=0.5,ScaleX=1.3,ScaleY=1.2,Pass=1)
     FakeDecoText=(MenuFont=Font'OrbitFonts.OrbitBold15',DrawColor=(B=200,G=200,R=200,A=222))
     ListButton=(Blurred=(MenuFont=Font'OrbitFonts.OrbitBold15',DrawColor=(B=255,G=200,R=120,A=255),PosX=0.5,ScaleX=1,ScaleY=0.8),Focused=(DrawColor=(A=255)),BackgroundFocused=(WidgetTexture=Texture'GUIContent.Menu.CT_ButtonFocus',RenderStyle=STY_Alpha,DrawPivot=DP_MiddleMiddle,PosY=-0.004,ScaleX=0.4,ScaleY=0.045,ScaleMode=MSCM_FitStretch),bRelativeBackgroundCoords=1)
     PushButton=(Blurred=(MenuFont=Font'OrbitFonts.OrbitBold15',ScaleX=0.8,ScaleY=0.6),Focused=(DrawColor=(B=61,G=169,R=222,A=255)),BackgroundBlurred=(WidgetTexture=Texture'GUIContent.Menu.BorderBoxD',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,ScaleX=0.23,ScaleY=0.046666,ScaleMode=MSCM_FitStretch),BackgroundFocused=(WidgetTexture=FinalBlend'GUIContent.Menu.ButtonBigPulse'),bRelativeBackgroundCoords=1)
     PushButtonWide=(Blurred=(MenuFont=Font'OrbitFonts.OrbitBold15',ScaleX=0.8,ScaleY=0.6),Focused=(DrawColor=(B=61,G=169,R=222,A=255)),BackgroundBlurred=(WidgetTexture=Texture'GUIContent.Menu.BorderBoxD',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,ScaleX=0.25,ScaleY=0.046666,ScaleMode=MSCM_FitStretch),BackgroundFocused=(WidgetTexture=FinalBlend'GUIContent.Menu.ButtonBigPulse'),bRelativeBackgroundCoords=1)
     ButtonList=(Template=(Blurred=(MenuFont=Font'OrbitFonts.OrbitBold15',DrawColor=(B=255,G=200,R=120,A=255),DrawPivot=DP_MiddleMiddle,PosX=0.5,ScaleX=1,ScaleY=0.8),Focused=(DrawColor=(A=255)),BackgroundBlurred=(WidgetTexture=Texture'GUIContent.Menu.CT_ButtonDefault',RenderStyle=STY_Alpha,DrawPivot=DP_MiddleMiddle,PosY=-0.004,ScaleX=0.615,ScaleY=0.04333,ScaleMode=MSCM_FitStretch),BackgroundFocused=(WidgetTexture=Texture'GUIContent.Menu.CT_ButtonFocus',RenderStyle=STY_Alpha,DrawPivot=DP_MiddleMiddle,PosY=-0.004,ScaleX=0.615,ScaleY=0.04333,ScaleMode=MSCM_FitStretch),bRelativeBackgroundCoords=1))
     SmallButtonList=(Template=(Blurred=(MenuFont=Font'OrbitFonts.OrbitBold8',DrawColor=(B=160,G=160,R=160,A=255),DrawPivot=DP_MiddleMiddle,PosX=0.5,ScaleX=0.9,ScaleY=0.8),Focused=(DrawColor=(B=255,G=255,R=255,A=255)),BackgroundFocused=(WidgetTexture=Texture'GUIContent.Menu.ButtonFocus',RenderStyle=STY_Alpha,DrawPivot=DP_MiddleMiddle,PosY=-0.004,ScaleX=0.4,ScaleY=0.04,ScaleMode=MSCM_FitStretch),bRelativeBackgroundCoords=1))
     EnumButton=(Blurred=(MenuFont=Font'OrbitFonts.OrbitBold15',ScaleX=0.8,ScaleY=0.6),Focused=(DrawColor=(B=61,G=169,R=222,A=255)),BackgroundBlurred=(WidgetTexture=Texture'GUIContent.Menu.BorderBoxD',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,ScaleX=0.2,ScaleY=0.046666,ScaleMode=MSCM_FitStretch),BackgroundFocused=(WidgetTexture=FinalBlend'GUIContent.Menu.ButtonBigPulse'),bRelativeBackgroundCoords=1)
     HugeEnumButton=(Blurred=(MenuFont=Font'OrbitFonts.OrbitBold15',ScaleX=1.1,ScaleY=1.1),Focused=(DrawColor=(B=61,G=169,R=222,A=255)),BackgroundBlurred=(WidgetTexture=Texture'GUIContent.Menu.BorderBoxD',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,ScaleX=0.6,ScaleY=0.06,ScaleMode=MSCM_FitStretch),BackgroundFocused=(WidgetTexture=FinalBlend'GUIContent.Menu.ButtonBigPulse'),bRelativeBackgroundCoords=1)
     UpArrow=(Blurred=(WidgetTexture=Texture'GUIContent.Menu.CT_ArrowUp',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,ScaleX=0.75,ScaleY=0.75),Focused=(DrawColor=(B=255,G=255,R=255,A=255),ScaleX=0.9,ScaleY=0.9),BackgroundBlurred=(WidgetTexture=Texture'GUIContent.Menu.CT_ArrowUp',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=192),DrawPivot=DP_MiddleMiddle,ScaleX=1,ScaleY=0.75),bRelativeBackgroundCoords=1,bIgnoreController=1)
     LeftArrow=(Blurred=(WidgetTexture=Texture'GUIContent.Menu.CT_ArrowLeft',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,ScaleX=0.75,ScaleY=0.75),Focused=(DrawColor=(B=255,G=255,R=255,A=255),ScaleX=0.9,ScaleY=0.9),BackgroundBlurred=(WidgetTexture=Texture'GUIContent.Menu.CT_ArrowLeft',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=192),DrawPivot=DP_MiddleMiddle,ScaleX=0.75,ScaleY=0.75),bRelativeBackgroundCoords=1,bIgnoreController=1)
     RightArrow=(Blurred=(WidgetTexture=Texture'GUIContent.Menu.CT_ArrowRight',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,ScaleX=0.75,ScaleY=0.75),Focused=(DrawColor=(B=255,G=255,R=255,A=255),ScaleX=0.9,ScaleY=0.9),BackgroundBlurred=(WidgetTexture=Texture'GUIContent.Menu.CT_ArrowRight',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=192),DrawPivot=DP_MiddleMiddle,ScaleX=0.75,ScaleY=0.75),bRelativeBackgroundCoords=1,bIgnoreController=1)
     XboxButtonA=(WidgetTexture=Texture'HUDTextures.Icons.AButton',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=255),ScaleX=0.85,ScaleY=0.85,Pass=2)
     XboxButtonB=(WidgetTexture=Texture'HUDTextures.Icons.BButton',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=255),ScaleX=0.85,ScaleY=0.85,Pass=2)
     XboxButtonX=(WidgetTexture=Texture'HUDTextures.Icons.XButton',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=255),ScaleX=0.85,ScaleY=0.85,Pass=2)
     XboxButtonY=(WidgetTexture=Texture'HUDTextures.Icons.Ybutton',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=255),ScaleX=0.85,ScaleY=0.85,Pass=2)
     VerticalScrollBarArrowUp=(Blurred=(WidgetTexture=Texture'GUIContent.Menu.CT_ArrowUp',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,ScaleX=0.75,ScaleY=0.75),Focused=(DrawColor=(B=255,G=255,R=255,A=255),ScaleX=0.9,ScaleY=0.9),bRelativeBackgroundCoords=1,bIgnoreController=1)
     VerticalScrollBarArrowDown=(Blurred=(WidgetTexture=Texture'GUIContent.Menu.CT_ArrowDown',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,ScaleX=0.75,ScaleY=0.75),Focused=(DrawColor=(B=255,G=255,R=255,A=255),ScaleX=0.9,ScaleY=0.9),bRelativeBackgroundCoords=1,bIgnoreController=1)
     VerticalScrollBar=(MinScaleX=0.02,MinScaleY=0.075,Blurred=(WidgetTexture=Texture'GUIContent.Menu.CT_ScrollDefault',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=180),DrawPivot=DP_MiddleMiddle,ScaleMode=MSCM_FitStretch),Focused=(WidgetTexture=Texture'GUIContent.Menu.CT_ScrollDefault'),SelectedBlurred=(WidgetTexture=Texture'GUIContent.Menu.CT_ScrollClick',DrawColor=(B=255,G=255,R=255,A=180)),bIgnoreController=1)
     SpriteButtonBordered=(Blurred=(WidgetTexture=Texture'Engine.TerrainBad',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=200),DrawPivot=DP_MiddleMiddle,ScaleX=0.1,ScaleY=0.1),BackgroundBlurred=(WidgetTexture=Texture'GUIContent.Menu.BorderBoxD',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=200),DrawPivot=DP_MiddleMiddle,ScaleX=0.1,ScaleY=0.1,ScaleMode=MSCM_FitStretch),BackgroundFocused=(WidgetTexture=FinalBlend'GUIContent.Menu.ButtonBigPulse'))
     SpriteButtonBorderedFit=(Blurred=(WidgetTexture=Texture'Engine.TerrainBad',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=200),DrawPivot=DP_MiddleMiddle,ScaleX=0.1,ScaleY=0.1,ScaleMode=MSCM_Fit),BackgroundBlurred=(WidgetTexture=Texture'GUIContent.Menu.BorderBoxD',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=200),DrawPivot=DP_MiddleMiddle,ScaleX=0.1,ScaleY=0.1,ScaleMode=MSCM_FitStretch),BackgroundFocused=(WidgetTexture=FinalBlend'GUIContent.Menu.ButtonBigPulse'))
     RedColor=(B=23,G=23,R=166,A=255)
     BlueColor=(B=186,G=29,R=25,A=255)
     WhiteColor=(B=200,G=200,R=200,A=255)
     ButtonTextStyle1=(Blurred=(MenuFont=Font'OrbitFonts.OrbitBold15',DrawColor=(B=255,G=200,R=120,A=255),DrawPivot=DP_MiddleMiddle,ScaleX=1,ScaleY=0.8),Focused=(MenuFont=Font'OrbitFonts.OrbitBold15',DrawColor=(A=255),DrawPivot=DP_MiddleMiddle),BackgroundBlurred=(WidgetTexture=Texture'GUIContent.Menu.CT_ButtonDefault',DrawColor=(B=255,G=255,R=255,A=192),DrawPivot=DP_MiddleMiddle,ScaleMode=MSCM_FitStretch),BackgroundFocused=(WidgetTexture=Texture'GUIContent.Menu.CT_ButtonFocus',DrawColor=(B=255,G=255,R=255,A=192),DrawPivot=DP_MiddleMiddle,ScaleMode=MSCM_FitStretch))
     PlanetBorderStyle=(WidgetTexture=Texture'GUIContent.Menu.CT_SelectionRing',DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,ScaleMode=MSCM_Fit)
     BorderStyle1=(WidgetTexture=Texture'GUIContent.Menu.CT_DisplayBox',DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,ScaleMode=MSCM_FitStretch)
     BorderStyle1Clear=(WidgetTexture=Texture'GUIContent.Menu.CT_DisplayBoxClear',DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,ScaleMode=MSCM_FitStretch)
     BorderStyle1Opaque=(WidgetTexture=Texture'GUIContent.Menu.CT_DisplayBoxOpaque',DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,ScaleMode=MSCM_FitStretch)
     ButtonEnumStyle1=(Blurred=(MenuFont=Font'OrbitFonts.OrbitBold15',DrawColor=(B=255,G=200,R=120,A=255),ScaleX=1,ScaleY=0.8),Focused=(DrawColor=(A=255)),BackgroundBlurred=(WidgetTexture=Texture'GUIContent.Menu.CT_ButtonDefault',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,ScaleX=0.2,ScaleY=0.046666,ScaleMode=MSCM_FitStretch),BackgroundFocused=(WidgetTexture=Texture'GUIContent.Menu.CT_ButtonFocus',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,ScaleX=0.2,ScaleY=0.046666,ScaleMode=MSCM_FitStretch))
     StringListStyle1=(Template=(Style="ButtonTextStyle1"))
     ButtonTextStyle2=(Blurred=(MenuFont=Font'OrbitFonts.OrbitBold15',DrawColor=(B=204,G=204,R=150,A=255),ScaleX=0.8,ScaleY=0.6),Focused=(DrawColor=(A=255)),BackgroundBlurred=(WidgetTexture=Texture'GUIContent.Menu.CT_ButtonDefault',RenderStyle=STY_Alpha,DrawColor=(B=255,G=255,R=255,A=255),DrawPivot=DP_MiddleMiddle,ScaleX=0.23,ScaleY=0.046666,ScaleMode=MSCM_FitStretch),BackgroundFocused=(WidgetTexture=Texture'GUIContent.Menu.CT_ButtonFocus'),bRelativeBackgroundCoords=1)
     MouseCursorTexture=Texture'UWindow.Icons.MouseCursor'
     MouseCursorScale=1
     VirtualKeyboardClass=Class'XInterface.MenuVirtualKeyboard'
     SoundTweenIn=Sound'UI_Sound.UI.int_GEN_selEMPgrn_01'
     SoundTweenOut=Sound'UI_Sound.UI.int_GEN_selSonicGrn_01'
     SoundOnFocus=Sound'UI_Sound.UI.int_GEN_textScroll_lp_01'
     SoundOnSelect=Sound'UI_Sound.UI.int_GEN_button_01'
     SoundOnError=Sound'UI_Sound.UI.int_GEN_button_06'
     ForceFeedbackOnFocus="GUIFocus"
     DefaultMenuDecoText=(MenuFont=Font'OrbitFonts.OrbitBold15',DrawColor=(B=200,G=200,R=200,A=222),TimePerCharacter=0.02,TimePerLineFeed=0.75,TimePerLoopEnd=2,TimePerCursorBlink=0.08,CursorScale=0.75,CursorOffset=0.25,bCapitalizeText=1)
     DefaultMenuButtonText=(Blurred=(MenuFont=Font'OrbitFonts.OrbitBold15',DrawColor=(B=180,G=180,R=180,A=255),DrawPivot=DP_MiddleMiddle,ScaleX=1,ScaleY=0.8))
     DefaultMenuCheckBoxText=(Blurred=(MenuFont=Font'OrbitFonts.OrbitBold15',DrawColor=(B=180,G=180,R=180,A=255),DrawPivot=DP_MiddleMiddle,ScaleX=1,ScaleY=0.8))
     DefaultMenuEditBox=(TimePerCursorBlink=0.2,CursorScale=0.6,Blurred=(MenuFont=Font'OrbitFonts.OrbitBold15',DrawColor=(B=180,G=180,R=180,A=255),DrawPivot=DP_MiddleMiddle,ScaleX=1,ScaleY=0.8))
     DefaultMenuBindingBox=(BackgroundSelected=(WidgetTexture=FinalBlend'GUIContent.Menu.ButtonBigPulse2',DrawColor=(G=255,A=255)),Blurred=(MenuFont=Font'OrbitFonts.OrbitBold15',DrawColor=(B=180,G=180,R=180,A=255),DrawPivot=DP_MiddleMiddle,ScaleX=0.8,ScaleY=0.6),BackgroundBlurred=(WidgetTexture=Texture'GUIContent.Menu.BorderBoxD',DrawColor=(B=175,G=175,R=175,A=255),DrawPivot=DP_MiddleMiddle,ScaleX=2,ScaleY=0.35,ScaleMode=MSCM_Stretch),BackgroundFocused=(WidgetTexture=FinalBlend'GUIContent.Menu.ButtonBigPulse',DrawColor=(B=255,G=255,R=255,A=125)),Pass=1)
     HelpText=(DrawColor=(B=61,G=169,R=222,A=255),DrawPivot=DP_MiddleMiddle,PosX=0.5,PosY=0.896,Pass=4,Style="LabelText")
     BackgroundMovieName="Menu_BG.xmv"
     BackgroundMovie=(DrawColor=(B=192,G=192,R=192))
     BackgroundMusic=SoundStreamed'UI_Music.Menu_Music.musUI_mainmenuVOX_lp'
}

