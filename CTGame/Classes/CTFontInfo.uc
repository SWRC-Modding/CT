class CTFontInfo extends Info;

var font HugeFont, BigFont, MediumFont, SmallFont;
var float SavedWidth[7];
var font SavedFont[7];

function font GetHugeFont(float Width)
{
	if ( (SavedFont[6] != None) && (Width == SavedWidth[6]) )
		return SavedFont[6];

	SavedWidth[6] = Width;
	SavedFont[6] = GetStaticHugeFont(Width);
	return SavedFont[6];
}

static function font GetStaticHugeFont(float Width)
{
	if (Width < 512)
		return Default.MediumFont;
	else if (Width < 640)
		return Default.BigFont;
	else
		return Default.HugeFont;
}

function font GetBigFont(float Width)
{
	if ( (SavedFont[5] != None) && (Width == SavedWidth[5]) )
		return SavedFont[5];

	SavedWidth[5] = Width;
	SavedFont[5] = GetStaticBigFont(Width);
	return SavedFont[5];
}

static function font GetStaticBigFont(float Width)
{
	if (Width < 512)
		return Default.MediumFont;
	else if (Width < 1024)
		return Default.BigFont;
	else
		return Default.HugeFont;
}

function font GetMediumFont(float Width)
{
	if ( (SavedFont[4] != None) && (Width == SavedWidth[4]) )
		return SavedFont[4];

	SavedWidth[4] = Width;
	SavedFont[4] = GetStaticMediumFont(Width);
	return SavedFont[4];
}

static function font GetStaticMediumFont(float Width)
{
	if (Width < 512)
		return Default.SmallFont;
	else if (Width < 1024)
		return Default.MediumFont;
	else
		return Default.BigFont;
}

function font GetSmallFont(float Width)
{
	if ( (SavedFont[3] != None) && (Width == SavedWidth[3]) )
		return SavedFont[3];

	SavedWidth[3] = Width;
	SavedFont[3] = GetStaticSmallFont(Width);
	return SavedFont[3];
}

static function font GetStaticSmallFont(float Width)
{
	if (Width < 512)
		return Default.SmallFont;
	else
		return Default.MediumFont;
}

function font GetSmallestFont(float Width)
{
	return Default.SmallFont;
}

static function font GetStaticSmallestFont(float Width)
{
	return Default.SmallFont;
}


defaultproperties
{
     HugeFont=Font'OrbitFonts.OrbitBold15'
     BigFont=Font'OrbitFonts.OrbitBold15'
     MediumFont=Font'OrbitFonts.OrbitBold8'
     SmallFont=Font'OrbitFonts.OrbitBold8'
}

