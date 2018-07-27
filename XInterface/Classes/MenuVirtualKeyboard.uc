class MenuVirtualKeyboard extends MenuTemplateTitledBXA
    native
    /* SBD Removed *** exportstructs*/;

struct KeyLayout
{
    var localized String Label;

    var int Span;

    var int bAppendMargin;
    var int bIsLastInRow;
};

var() KeyLayout KeyboardLayout[39];

var() MenuSprite KeyboardBorder;
var() MenuSprite InputBorder;

var() MenuText InputText;

var() MenuButtonText Buttons[39];

var() float ButtonSizeX, ButtonSizeY;
var() float ButtonGapX, ButtonGapY;

var() float MarginSize;

var() float KeyboardPosX, KeyboardPosY; // DP_MiddleMiddle

var const private transient native int pEditBox; // Natively mapped to MenuEditBox*

var const int MaxLength;
var const int MinLength;
var const int bNoDigits;
var const int bNoSpaces;

var String TextBackup;
var string SpaceChar;

simulated native function UpdateEditBox();

simulated function Init( String Args )
{
    local int i, RowWidth, MarginCount;
    local int ButtonCountX, ButtonCountY, ButtonCountMargin;

    local float KeyboardSizeX, KeyboardSizeY;
    local float PosX, PosY, DX, DY, OrigPosX;
    local int PosI;
    local string opt;

    Super.Init( Args );
    
    opt = ParseOption(Args, "InputText");
    if (opt != "")
        InputText.Text = opt;

    opt = ParseOption(Args, "Title");
    if (opt != "")
        MenuTitle.Text = opt;

    TextBackup = InputText.Text;

    FocusOnNothing();

    // Calculate the width and height:

    RowWidth = 0;
    MarginCount = 0;
    ButtonCountX = 0;
    ButtonCountY = 0;

    for( i = 0; i < ArrayCount (KeyboardLayout); i++ )
    {
        RowWidth += KeyboardLayout[i].Span;

        KeyboardLayout[i].Label = Caps( KeyboardLayout[i].Label );

        if( KeyboardLayout[i].bAppendMargin != 0 )
            MarginCount++;

        if( KeyboardLayout[i].bIsLastInRow == 0 )
            continue;

        ButtonCountY++;

        if( ButtonCountX == 0 )
            ButtonCountX = RowWidth;
        else if( ButtonCountX != RowWidth )
        {
            log( "Keyboard rows are not not all the same width!", 'Error' );
            ButtonCountX = Max (ButtonCountX, RowWidth);
        }

        if( ButtonCountMargin == 0 )
            ButtonCountMargin = MarginCount;
        else if( ButtonCountMargin != MarginCount )
        {
            log( "Keyboard rows don't have the same number of margins!", 'Error' );
            ButtonCountMargin = Max( ButtonCountMargin, MarginCount );
        }

        RowWidth = 0;
        MarginCount = 0;
    }

    if( KeyboardLayout[ArrayCount( KeyboardLayout ) - 1].bIsLastInRow == 0 )
    {
        log ( "Last keyboard row is not terminated!", 'Error' );
        ButtonCountY++;
    }

    KeyboardSizeX = ( ButtonCountX * ButtonSizeX ) + ( ( ButtonCountX - 1 ) * ButtonGapX ) + ( ButtonCountMargin * MarginSize );
    KeyboardSizeY = ( ButtonCountY * ButtonSizeY ) + ( ( ButtonCountY - 1 ) * ButtonGapY );

    PosX = KeyboardPosX - ( KeyboardSizeX * 0.5 );
    PosY = KeyboardPosY - ( KeyboardSizeY * 0.5 );

    KeyboardBorder.PosX = KeyboardPosX;
    KeyboardBorder.PosY = KeyboardPosY;

    OrigPosX = PosX;

    for( PosI = 0; PosI < ArrayCount( KeyboardLayout ); PosI++ )
    {
        DX = (ButtonSizeX * KeyboardLayout[PosI].Span) + ((KeyboardLayout[PosI].Span - 1) * ButtonGapX);
        DY = ButtonSizeY;

        Buttons[PosI].ContextID = PosI;
        Buttons[PosI].bRelativeBackgroundCoords = 0;

        Buttons[PosI].Blurred.PosX = PosX + (DX * 0.5);
        Buttons[PosI].Blurred.PosY = PosY + (DY * 0.5);
        Buttons[PosI].Blurred.Text = KeyboardLayout[PosI].Label;

        Buttons[PosI].BackgroundBlurred.PosX = PosX + (DX * 0.5);
        Buttons[PosI].BackgroundBlurred.PosY = PosY + (DY * 0.5);
        
        if( KeyboardLayout[PosI].Span == 1 )
            Buttons[PosI].BackgroundBlurred.ScaleX *= DX / ButtonSizeX;
        else // These aren't the hacks you're looking for.
            Buttons[PosI].BackgroundBlurred.ScaleX *= (DX * 0.9999) / ButtonSizeX;

        Buttons[PosI].Focused.PosX = Buttons[PosI].Blurred.PosX;
        Buttons[PosI].Focused.PosY = Buttons[PosI].Blurred.PosY;
        Buttons[PosI].Focused.Text = Buttons[PosI].Blurred.Text;

        Buttons[PosI].BackgroundFocused.PosX = Buttons[PosI].BackgroundBlurred.PosX;
        Buttons[PosI].BackgroundFocused.PosY = Buttons[PosI].BackgroundBlurred.PosY;
        Buttons[PosI].BackgroundFocused.ScaleX = Buttons[PosI].BackgroundBlurred.ScaleX;

        if( KeyboardLayout[PosI].bIsLastInRow != 0 )
        {
            PosX = OrigPosX;
            PosY += ButtonSizeY + ButtonGapY;
        }
        else
        {
            PosX += KeyboardLayout[PosI].Span * ( ButtonSizeX + ButtonGapX );

            if( KeyboardLayout[PosI].bAppendMargin != 0 )
                PosX += MarginSize;
        }
    }

    FocusOnWidget( Buttons[37] );


    // Set-up constraints:

    if( bNoSpaces != 0 )
    {
        Buttons[37].bHidden = 1;        
    }
    
    if( bNoDigits != 0 )
    {
        for( i = 0; i < 10; i++ )
            buttons[i].bHidden = 1;
    }
}

simulated function OnKey(int ContextID)
{
    if( (MaxLength > 0) && Len( InputText.Text ) >= MaxLength )
        return;

    OnSelectionChange();
    InputText.Text = InputText.Text $ KeyboardLayout[ContextID].Label;
    UpdateEditBox();
}

simulated function OnBackSpace()
{
    local int Length;

    Length = Len( InputText.Text );

    if( Length == 0 )
        return;

    OnSelectionChange();
    InputText.Text = Left( InputText.Text, Length - 1 );
    UpdateEditBox();
}

simulated function OnSpace()
{
    local int length;

    length = Len(InputText.Text);

    if( MaxLength > 0 && length >= MaxLength )
        return;

    // no leading spaces or consecutive spaces
    if (length == 0 || Right(InputText.Text, 1) == SpaceChar)
        return;

    OnSelectionChange();
    InputText.Text = InputText.Text $ SpaceChar;
    UpdateEditBox();
}

simulated function TrimTrailingSpace()
{
    // trim trailing space if any
    if (Right(InputText.Text, 1) == SpaceChar)
        InputText.Text = Left(InputText.Text, Len(InputText.Text) - 1);
}

simulated function OnDone()
{
    if( Len( InputText.Text ) < MinLength )
    {
        InputText.Text = TextBackup;
        UpdateEditBox();
    }

    TrimTrailingSpace();

    CloseMenu();
}

simulated function HandleInputBack()
{
    InputText.Text = TextBackup;
    UpdateEditBox();
    Super.HandleInputBack();
}

simulated function OnXButton()
{
    local PlayerController PC;

    PC = PlayerController( Owner );
    assert( PC != None );

    PC.PlaySound( SoundOnFocus );

    InputText.Text = "";
    UpdateEditBox();
}



defaultproperties
{
     KeyboardLayout(0)=(Label="1",Span=1)
     KeyboardLayout(1)=(Label="2",Span=1)
     KeyboardLayout(2)=(Label="3",Span=1)
     KeyboardLayout(3)=(Label="4",Span=1)
     KeyboardLayout(4)=(Label="5",Span=1)
     KeyboardLayout(5)=(Label="6",Span=1)
     KeyboardLayout(6)=(Label="7",Span=1)
     KeyboardLayout(7)=(Label="8",Span=1)
     KeyboardLayout(8)=(Label="9",Span=1)
     KeyboardLayout(9)=(Label="0",Span=1,bIsLastInRow=1)
     KeyboardLayout(10)=(Label="a",Span=1)
     KeyboardLayout(11)=(Label="b",Span=1)
     KeyboardLayout(12)=(Label="c",Span=1)
     KeyboardLayout(13)=(Label="d",Span=1)
     KeyboardLayout(14)=(Label="e",Span=1)
     KeyboardLayout(15)=(Label="f",Span=1)
     KeyboardLayout(16)=(Label="g",Span=1)
     KeyboardLayout(17)=(Label="h",Span=1)
     KeyboardLayout(18)=(Label="i",Span=1)
     KeyboardLayout(19)=(Label="j",Span=1,bIsLastInRow=1)
     KeyboardLayout(20)=(Label="k",Span=1)
     KeyboardLayout(21)=(Label="l",Span=1)
     KeyboardLayout(22)=(Label="m",Span=1)
     KeyboardLayout(23)=(Label="n",Span=1)
     KeyboardLayout(24)=(Label="o",Span=1)
     KeyboardLayout(25)=(Label="p",Span=1)
     KeyboardLayout(26)=(Label="q",Span=1)
     KeyboardLayout(27)=(Label="r",Span=1)
     KeyboardLayout(28)=(Label="s",Span=1)
     KeyboardLayout(29)=(Label="t",Span=1,bIsLastInRow=1)
     KeyboardLayout(30)=(Label="u",Span=1)
     KeyboardLayout(31)=(Label="v",Span=1)
     KeyboardLayout(32)=(Label="w",Span=1)
     KeyboardLayout(33)=(Label="x",Span=1)
     KeyboardLayout(34)=(Label="y",Span=1)
     KeyboardLayout(35)=(Label="z",Span=1)
     KeyboardLayout(36)=(Label="<<",Span=2)
     KeyboardLayout(37)=(Label="Ok",Span=2,bIsLastInRow=1)
     KeyboardLayout(38)=(Label="Space",Span=10,bIsLastInRow=1)
     KeyboardBorder=(DrawPivot=DP_MiddleMiddle,ScaleX=0.75,ScaleY=0.42,Style="BorderStyle1")
     InputBorder=(DrawPivot=DP_MiddleMiddle,PosX=0.5,PosY=0.25,ScaleX=0.75,ScaleY=0.1,Style="BorderStyle1")
     InputText=(DrawPivot=DP_MiddleMiddle,PosX=0.5,PosY=0.25,Pass=2,Style="LabelText")
     Buttons(0)=(BackgroundBlurred=(ScaleX=0.05,ScaleY=0.05),OnSelect="OnKey",Pass=2,Style="ButtonTextStyle2")
     Buttons(36)=(OnSelect="OnBackSpace")
     Buttons(37)=(OnSelect="OnDone")
     Buttons(38)=(OnSelect="OnSpace")
     ButtonSizeX=0.05
     ButtonSizeY=0.05
     ButtonGapX=0.02
     ButtonGapY=0.02
     MarginSize=0.001
     KeyboardPosX=0.5
     KeyboardPosY=0.55
     MaxLength=15
     MinLength=1
     SpaceChar="_"
     XLabel=(Text=": CLEAR")
     XButton=(Blurred=(Text="CLEAR"))
     Background=(bHidden=1)
     MenuTitle=(Text="EDIT NAME")
}

