#include "Mod.h"
#include "CodeInjection.h"

/*
 * CJK text rendering support (Chinese localization).
 *
 * The retail Engine.dll is an ANSI build (TCHAR == char): UCanvas::WrappedPrint
 * iterates strings byte by byte, so CharRemap keys > 0xFF can never be produced
 * and DBCS text renders as blanks. This file replaces WrappedPrint with a
 * binary-compatible reimplementation (reference: Engine.dll decompilation,
 * UCanvas::WrappedPrint @103E9580, glyph emitter @103E8990) that pairs
 * GBK/Shift-JIS lead+trail bytes into 16-bit CharRemap keys, the same scheme
 * the official Japanese release used.
 *
 * Only installed when the EnableCJKText config property on SWRCFix is set
 * (System.ini [Mod.SWRCFix]); default off, so existing installs are unaffected.
 *
 * NOTE: The in-memory FFontCharacter has a TextureIndex byte (stride 20) that
 * the reconstructed CT headers comment out, and this build's TArray is 8 bytes
 * (Data, Num). Raw layouts below are verified against the decompilation.
 */

struct FGlyphChar{
	INT  StartU;
	INT  StartV;
	INT  USize;
	INT  VSize;
	BYTE TextureIndex;
	BYTE Pad[3];
};

struct FRemapPair{
	INT  Next;
	WORD Key;
	WORD Value;
};

struct FRawFont{ // UFont
	BYTE        UObjectPad[40];
	FGlyphChar* Chars;       INT NumCharsBits;    // +40 +44 (TArray num is a 29-bit field)
	UMaterial** Textures;    INT NumTexturesBits; // +48 +52
	FRemapPair* Pairs;       INT NumPairsBits;    // +56 +60
	INT*        Hash;        INT HashCount;       // +64 +68
	INT         IsRemapped;                       // +72
	INT         Kerning;                          // +76

	INT NumChars() const   { return NumCharsBits & 0x1FFFFFFF; }
	INT NumTextures() const{ return NumTexturesBits & 0x1FFFFFFF; }
};

static inline bool IsDBCSLead(BYTE B) { return B >= 0x81 && B <= 0xFE; }
static inline bool IsDBCSTrail(BYTE B){ return B >= 0x40 && B <= 0xFE && B != 0x7F; }

// GBK punctuation that must not appear at the start of a line.
static bool IsNoBreakBeforePunct(WORD Key){
	switch(Key){
	case 0xA1A2: // ideographic comma
	case 0xA1A3: // ideographic full stop
	case 0xA1A4: // middle dot
	case 0xA1AA: // em dash
	case 0xA1AD: // horizontal ellipsis
	case 0xA1AF: // right single quotation mark
	case 0xA1B1: // right double quotation mark
	case 0xA1B7: // right double angle bracket
	case 0xA1B9: // right corner bracket
	case 0xA1BB: // right white corner bracket
	case 0xA3A1: // fullwidth exclamation mark
	case 0xA3A9: // fullwidth right parenthesis
	case 0xA3AC: // fullwidth comma
	case 0xA3BA: // fullwidth colon
	case 0xA3BB: // fullwidth semicolon
	case 0xA3BF: // fullwidth question mark
		return true;
	default:
		return false;
	}
}

// Reads the next character, pairing DBCS lead+trail bytes into a 16-bit key
static WORD NextCharKey(const BYTE* S, INT& i){
	BYTE B = S[i++];

	if(IsDBCSLead(B) && IsDBCSTrail(S[i]))
		return static_cast<WORD>((B << 8) | S[i++]);

	return B;
}

static INT RemapKey(const FRawFont* F, WORD Key){
	if(!F->IsRemapped)
		return Key;

	for(INT i = F->Hash[Key & (F->HashCount - 1)]; i != -1; i = F->Pairs[i].Next){
		if(F->Pairs[i].Key == Key)
			return F->Pairs[i].Value;
	}

	return 32; // space
}

// Reimplementation of the per-line glyph emitter (@103E8990, ampersand path
// unused by callers omitted). bClip enables clipping against ClipX/ClipY as in
// the ClippedPrint mode of the original. Returns the accumulated advance.
static INT DrawGlyphRun(UCanvas* C, const FRawFont* F, FLOAT ScaleX, FLOAT ScaleY,
                        INT X, INT Y, const BYTE* Text, INT NumBytes, const FPlane& Color,
                        bool bClip = false){
	if(!C->pCanvasUtil)
		return 0;

	FPlane Fog(0.0f, 0.0f, 0.0f, 0.0f);
	INT Width = 0;

	for(INT i = 0; i < NumBytes && Text[i];){
		WORD Key = NextCharKey(Text, i);
		INT Glyph = RemapKey(F, Key);
		INT Advance = 0;

		if(Glyph >= 0 && Glyph < F->NumChars()){
			const FGlyphChar& G = F->Chars[Glyph];

			Advance = G.USize;

			if(G.TextureIndex < F->NumTextures()){
				UMaterial* Material = F->Textures[G.TextureIndex];

				if(Material && G.USize > 0){
					INT GX = Width + X;
					FLOAT U = (FLOAT)G.StartU;
					FLOAT V = (FLOAT)G.StartV;
					FLOAT UL = (FLOAT)G.USize;
					FLOAT VL = (FLOAT)G.VSize;

					if(bClip){ // right/bottom clip as in the original clip mode
						if((FLOAT)(GX + UL * ScaleX) > C->ClipX)
							UL = Max(0.0f, (C->ClipX - GX) / ScaleX);

						if((FLOAT)(Y + VL * ScaleY) > C->ClipY)
							VL = Max(0.0f, (C->ClipY - Y) / ScaleY);
					}

					if(UL > 0.0f && VL > 0.0f){
						C->DrawTile(Material,
						            static_cast<FLOAT>(appFloor((FLOAT)GX + C->OrgX)),
						            static_cast<FLOAT>(appFloor((FLOAT)Y + C->OrgY)),
						            UL * ScaleX, VL * ScaleY,
						            U, V, UL, VL,
						            1.0f, Color, Fog);
					}
				}
			}
		}

		Width += appFloor((F->Kerning + C->SpaceX + Advance) * ScaleX);
	}

	return Width;
}

/*
 * Replacement for UCanvas::WrappedPrint(INT Style, INT& XL, INT& YL, UFont*,
 * FLOAT ScaleX, FLOAT ScaleY, UBOOL Center, const TCHAR* Text).
 * Behavior mirrors the original except characters are fetched DBCS-aware and
 * line breaks are additionally allowed at DBCS character boundaries.
 */
static void __fastcall WrappedPrintOverride(UCanvas* C, DWORD, INT Style, INT* XL, INT* YL,
                                            UFont* InFont, FLOAT ScaleX, FLOAT ScaleY,
                                            INT Center, const TCHAR* Text){
	const FRawFont* F = reinterpret_cast<const FRawFont*>(InFont);
	const BYTE* S = reinterpret_cast<const BYTE*>(Text);

	if(!S || !F || !(C->ClipX >= 0.0f && C->ClipY >= 0.0f))
		return;

	const BYTE* Cb = reinterpret_cast<const BYTE*>(C) + 88; // FColor (B,G,R,A)
	const FLOAT K = 1.0f / 255.0f;
	FPlane Color(Cb[2] * K, Cb[1] * K, Cb[0] * K, Cb[3] * K);
	FLOAT EntryCurX = C->CurX;

	*XL = 0;
	*YL = 0;

	for(;;){
		INT LineH   = 0;             // running max glyph height (incl. overflow char)
		INT X       = appFloor(C->CurX);
		INT FitLen  = 0;             // bytes up to the last usable break point
		INT FitEndX = X;             // absolute end X at that break point
		INT FitH    = 0;
		bool HaveBreak = false;
		INT i = 0;

		for(;;){
			BYTE B = S[i];

			if(!B || B == 10)
				break;

			INT j = i;
			WORD Key = NextCharKey(S, j);
			INT Glyph = RemapKey(F, Key);
			INT W = 0;
			INT H = 0;

			if(Glyph >= 0 && Glyph < F->NumChars()){
				W = F->Chars[Glyph].USize;
				H = F->Chars[Glyph].VSize;
			}

			X += appFloor((F->Kerning + C->SpaceX + W) * ScaleX);

			INT GH = appFloor((H + appFloor(C->SpaceY)) * ScaleY);

			if(GH > LineH)
				LineH = GH;

			if((FLOAT)X > C->ClipX)
				break; // overflow: char not included, wrap at last break point

			i = j;

			BYTE Next = S[i];

			// Keep ASCII periods (including "...") with the preceding CJK text.
			bool NextIsNoBreak = Next == '.';
			if(IsDBCSLead(Next) && IsDBCSTrail(S[i + 1]))
				NextIsNoBreak = IsNoBreakBeforePunct((Next << 8) | S[i + 1]);

			bool Breakable = (Key > 0xFF && !NextIsNoBreak) || Next == 0 || Next == 10 ||
			                 (IsDBCSLead(Next) && !NextIsNoBreak) ||
			                 (Next == 32 && !(S[i + 1] == '!' || S[i + 1] == '?' || S[i + 1] == ':'));

			if(Breakable || !HaveBreak){
				FitLen  = i;
				FitEndX = X;
				FitH    = LineH;
				HaveBreak = HaveBreak || Breakable;
			}
		}

		if(!FitLen)
			break;

		if(Style){
			FLOAT AbsY = C->OrgY + C->CurY;
			INT ViewH = *reinterpret_cast<INT*>(*reinterpret_cast<BYTE**>(reinterpret_cast<BYTE*>(C) + 132) + 132); // Viewport->SizeY

			if((FLOAT)ViewH > AbsY && (FLOAT)FitH + AbsY > 0.0f){
				INT DX;

				if(Center == 1)
					DX = appFloor((C->ClipX - FitEndX) * 0.5f + C->CurX);
				else if(Center == 2)
					DX = appFloor((C->ClipX - FitEndX) - C->CurX);
				else
					DX = appFloor(C->CurX);

				INT Drawn = DrawGlyphRun(C, F, ScaleX, ScaleY, DX, appFloor(C->CurY), S, FitLen, Color);

				C->CurX = static_cast<FLOAT>(Drawn + DX);
			}
		}

		C->CurX = EntryCurX;
		C->CurY += FitH;
		*YL += FitH;

		if(FitEndX > *XL)
			*XL = FitEndX;

		S += FitLen;

		if(*S == 10)
			++S;

		while(*S == 32)
			++S;

		if(!*S)
			break;
	}
}

/*
 * Replacement for FCanvasUtil::DrawString(INT X, INT Y, const TCHAR* Text, UFont*,
 * FColor, FLOAT ScaleX, FLOAT ScaleY, UBOOL Center) - the menu/util text path
 * (reference: @104C9A50 in the decompilation). Glyphs are emitted through the
 * original exported FCanvasUtil::DrawTile. Returns the final X advance.
 *
 * '&' marks a shortcut character: the following character is drawn with an
 * underscore glyph overlaid ("&&" is a literal '&').
 */
typedef void(__fastcall* FCUDrawTileFunc)(void* CanvasUtil, DWORD Edx,
                                          FLOAT X1, FLOAT Y1, FLOAT X2, FLOAT Y2,
                                          FLOAT U1, FLOAT V1, FLOAT U2, FLOAT V2,
                                          FLOAT Z, UMaterial* Material, FColor Color);
static FCUDrawTileFunc OriginalFCUDrawTile = NULL;

static const FGlyphChar* GetGlyph(const FRawFont* F, WORD Key, UMaterial** OutMaterial){
	INT Glyph = RemapKey(F, Key);

	if(Glyph < 0 || Glyph >= F->NumChars())
		return NULL;

	const FGlyphChar& G = F->Chars[Glyph];

	*OutMaterial = G.TextureIndex < F->NumTextures() ? F->Textures[G.TextureIndex] : NULL;

	return &G;
}

static INT __fastcall DrawStringOverride(void* U, DWORD, INT StartX, INT StartY, const TCHAR* Text,
                                         UFont* InFont, FColor Color, FLOAT ScaleX, FLOAT ScaleY,
                                         UBOOL Center){
	const FRawFont* F = reinterpret_cast<const FRawFont*>(InFont);
	const BYTE* S = reinterpret_cast<const BYTE*>(Text);

	if(!S || !F || !OriginalFCUDrawTile)
		return 0;

	INT X = 0;
	INT Y = 0;

	if(Center){ // center on StartX/StartY: offset by half the total extent
		INT TotalW = 0;
		INT MaxH = 0;

		for(INT i = 0; S[i];){
			if(S[i] == '&' && S[i + 1] && S[i + 1] != '&')
				++i; // shortcut marker: the char itself is measured on the next round

			WORD Key = NextCharKey(S, i);
			UMaterial* Material;
			const FGlyphChar* G = GetGlyph(F, Key, &Material);

			if(G && Material){
				TotalW += appFloor(G->USize * ScaleX);

				INT H = appFloor(G->VSize * ScaleY);

				if(H > MaxH)
					MaxH = H;
			}
		}

		X = TotalW / -2;
		Y = MaxH / -2;
	}

	FColor DrawColor = Color;

	if(GIsOpenGL)
		Exchange(DrawColor.R, DrawColor.B);

	WORD UnderscoreFor = 0; // pending shortcut char key; 0 = none

	for(INT i = 0; S[i];){
		WORD Key;

		if(UnderscoreFor){
			Key = '_'; // overlay pass at the previous char's position
		}
		else if(S[i] == '&' && S[i + 1] && S[i + 1] != '&'){
			++i;
			Key = NextCharKey(S, i);
			UnderscoreFor = Key;
		}
		else{
			Key = NextCharKey(S, i);
		}

		UMaterial* Material;
		const FGlyphChar* G = GetGlyph(F, Key, &Material);

		if(G && Material){
			INT W = appFloor(G->USize * ScaleX);
			INT H = appFloor(G->VSize * ScaleY);

			if(UnderscoreFor && Key == '_'){ // underscore no wider than the char it underlines
				UMaterial* CharMaterial;
				const FGlyphChar* CharG = GetGlyph(F, UnderscoreFor, &CharMaterial);
				INT CharW = CharG ? appFloor(CharG->USize * ScaleX) : W;

				if(CharW < W)
					W = CharW;
			}

			OriginalFCUDrawTile(U, 0,
			                    (FLOAT)(X + StartX), (FLOAT)(Y + StartY),
			                    (FLOAT)(X + StartX + W), (FLOAT)(Y + StartY + H),
			                    (FLOAT)G->StartU, (FLOAT)G->StartV,
			                    (FLOAT)W / ScaleX + G->StartU, (FLOAT)(G->StartV + G->VSize),
			                    0.0f, Material, DrawColor);
		}

		if(UnderscoreFor){
			if(Key == '_'){ // overlay drawn: now advance past the shortcut char
				UMaterial* CharMaterial;
				const FGlyphChar* CharG = GetGlyph(F, UnderscoreFor, &CharMaterial);

				if(CharG && CharMaterial)
					X += appFloor(CharG->USize * ScaleX);

				UnderscoreFor = 0;
			}
			// else: shortcut char just drawn, don't advance yet (underscore overlays it)
		}
		else if(G && Material){
			X += appFloor(G->USize * ScaleX);
		}
	}

	return X;
}

/*
 * Replacement for UCanvas::ClippedPrint(UFont*, FLOAT ScaleX, FLOAT ScaleY,
 * UBOOL CheckHotKey, const TCHAR* Text) - single-line print at CurX/CurY with
 * clipping (@103EA170: thin wrapper around the glyph emitter with bClip=1;
 * the CheckHotKey argument is ignored by the original as well).
 */
static void __fastcall ClippedPrintOverride(UCanvas* C, DWORD, UFont* InFont, FLOAT ScaleX,
                                            FLOAT ScaleY, INT /*CheckHotKey*/, const TCHAR* Text){
	const FRawFont* F = reinterpret_cast<const FRawFont*>(InFont);
	const BYTE* S = reinterpret_cast<const BYTE*>(Text);

	if(!S || !F)
		return;

	const BYTE* Cb = reinterpret_cast<const BYTE*>(C) + 88; // FColor (B,G,R,A)
	const FLOAT K = 1.0f / 255.0f;
	FPlane Color(Cb[2] * K, Cb[1] * K, Cb[0] * K, Cb[3] * K);

	DrawGlyphRun(C, F, ScaleX, ScaleY, appFloor(C->CurX), appFloor(C->CurY), S, MAXINT, Color, true);
}

/*
 * Replacement for UCanvas::ClippedStrLen(UFont*, FLOAT ScaleX, FLOAT ScaleY,
 * INT& XL, INT& YL, const TCHAR* Text) - single-line measurement (@103E9EB0).
 * Kerning+SpaceX is added after every character except the last, as in the
 * original.
 */
static void __fastcall ClippedStrLenOverride(UCanvas* C, DWORD, UFont* InFont, FLOAT ScaleX,
                                             FLOAT ScaleY, INT* XL, INT* YL, const TCHAR* Text){
	const FRawFont* F = reinterpret_cast<const FRawFont*>(InFont ? InFont : C->Font);
	const BYTE* S = reinterpret_cast<const BYTE*>(Text);

	*XL = 0;
	*YL = 0;

	if(!S || !F)
		return;

	for(INT i = 0; S[i];){
		WORD Key = NextCharKey(S, i);
		INT Glyph = RemapKey(F, Key);
		INT W = 0;
		INT H = 0;

		if(Glyph >= 0 && Glyph < F->NumChars()){
			W = F->Chars[Glyph].USize;
			H = F->Chars[Glyph].VSize;
		}

		FLOAT CharW = S[i] ? (FLOAT)(F->Kerning + C->SpaceX + W) : (FLOAT)W;

		*XL += appFloor(CharW * ScaleX);

		INT CharH = appFloor(H * ScaleY);

		if(CharH > *YL)
			*YL = CharH;
	}
}

/*
 * DBCS-aware replacement for FString::Caps (Core.dll).
 *
 * The original applies Windows-1252 toupper byte-by-byte, which corrupts GBK
 * characters whose second byte falls in a convertible range (0xE0-0xEF,
 * 0xF1-0xF6, 0xF8-0xFD, 0x61-0x7A).  For example, GBK B3 F6 becomes B3 D6
 * when byte F6 is uppercased.  This override skips the trail byte of every
 * valid DBCS pair while preserving the original single-byte mappings.
 */
static FStringTemp* __fastcall CapsOverride(const FString* Self, DWORD, FStringTemp* Result){
	new(Result) FStringTemp(*Self);

	INT Len = Result->Len();

	if(Len == 0)
		return Result;

	BYTE* Buf = reinterpret_cast<BYTE*>(&(*Result)[0]);

	for(INT i = 0; i < Len; i++){
		BYTE B = Buf[i];

		if(IsDBCSLead(B) && i + 1 < Len && IsDBCSTrail(Buf[i + 1])){
			i++;
			continue;
		}

		if((B >= 'a' && B <= 'z') ||
		   (B >= 0xE0 && B <= 0xEF) ||
		   (B >= 0xF1 && B <= 0xF6) ||
		   (B >= 0xF8 && B <= 0xFD)){
			Buf[i] = (BYTE)(B - 32);
		}
		else if(B == 0x9A || B == 0x9E){
			Buf[i] = (BYTE)(B - 16);
		}
		else if(B == 0xDE){
			Buf[i] = 0xFE;
		}
	}

	return Result;
}

void InitCJKText(void){
	void* Handle = appGetDllHandle("Engine.dll");

	if(!Handle){
		debugf(NAME_Error, "CJKText: failed to get Engine.dll handle");

		return;
	}

	void* WrappedPrint  = appGetDllExport(Handle, "?WrappedPrint@UCanvas@@AAEXHAAH0PAVUFont@@MMHPBD@Z");
	void* DrawString    = appGetDllExport(Handle, "?DrawString@FCanvasUtil@@QAEHHHPBDPAVUFont@@VFColor@@MM_N@Z");
	void* ClippedPrint  = appGetDllExport(Handle, "?ClippedPrint@UCanvas@@UAEXPAVUFont@@MMHPBD@Z");
	void* ClippedStrLen = appGetDllExport(Handle, "?ClippedStrLen@UCanvas@@UAEXPAVUFont@@MMAAH1PBD@Z");

	OriginalFCUDrawTile = reinterpret_cast<FCUDrawTileFunc>(appGetDllExport(Handle, "?DrawTile@FCanvasUtil@@QAEXMMMMMMMMMPAVUMaterial@@VFColor@@@Z"));

	appFreeDllHandle(Handle);

	if(!WrappedPrint || !DrawString || !ClippedPrint || !ClippedStrLen || !OriginalFCUDrawTile){
		debugf(NAME_Error, "CJKText: Engine.dll export not found (WrappedPrint=%p DrawString=%p ClippedPrint=%p ClippedStrLen=%p DrawTile=%p)",
		       WrappedPrint, DrawString, ClippedPrint, ClippedStrLen, OriginalFCUDrawTile);

		return;
	}

	RedirectFunction(WrappedPrint, WrappedPrintOverride);
	RedirectFunction(DrawString, DrawStringOverride);
	RedirectFunction(ClippedPrint, ClippedPrintOverride);
	RedirectFunction(ClippedStrLen, ClippedStrLenOverride);
	debugf(NAME_Init, "CJKText: WrappedPrint/DrawString/ClippedPrint/ClippedStrLen hooked (DBCS pairing enabled)");

	void* CoreHandle = appGetDllHandle("Core.dll");

	if(CoreHandle){
		void* Caps = appGetDllExport(CoreHandle, "?Caps@FString@@QBE?AVFStringTemp@@XZ");

		appFreeDllHandle(CoreHandle);

		if(Caps){
			RedirectFunction(Caps, CapsOverride);
			debugf(NAME_Init, "CJKText: FString::Caps hooked (DBCS-aware toupper)");
		}
		else{
			debugf(NAME_Warning, "CJKText: FString::Caps export not found, Caps() may corrupt DBCS text");
		}
	}
}
