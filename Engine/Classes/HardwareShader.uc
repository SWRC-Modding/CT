class HardwareShader extends RenderedMaterial
	native
	noexport;

// DS_SHADER

enum SConstant
{
	EVC_Unused, // This constant isn't used by the shader and does not need to be updated
	EVC_MaterialDefined, // The constant is defined by by the Value member of FVSConstantsInfo
	EVC_WorldToScreenMatrix, // The constant, and the three constants after it will contain the view projection matrix
	EVC_ObjectToScreenMatrix,
	EVC_ObjectToWorldMatrix,
	EVC_CameraToWorldMatrix, // The constant, and the three constants after it will contain the inverse view matrix
	EVC_WorldToCameraMatrix, // The constant, and the three constants after it will contain the view matrix
	EVC_WorldToObjectMatrix,
	EVC_Time, // Time, cycled over 120 seconds
	EVC_CosTime,
	EVC_SinTime,
	EVC_TanTime,
	EVC_EyePosition,
	EVC_XYCircle,
	EVC_LightPos1,
	EVC_LightColor1,
	EVC_LightInvRadius1,
	EVC_LightPos2,
	EVC_LightColor2,
	EVC_LightInvRadius2,
	EVC_LightPos3,
	EVC_LightColor3,
	EVC_LightInvRadius3,
	EVC_LightPos4,
	EVC_LightColor4,
	EVC_LightInvRadius4,
	EVC_AmbientLightColor,
	EVC_Flicker,
	EVC_SpotlightDirection,
	EVC_SpotlightCosCone,
	EVC_DrawScale3D,
	EVC_Fog,
	EVC_ObjectToCameraMatrix,
	EVC_EyePositionObjectSpace,
	EVC_2DRotator,
	EVC_Max
};

enum EFixedVertexFunction
{
	FVF_Position,
	FVF_Normal,
	FVF_Diffuse,
	FVF_Specular,
	FVF_TexCoord0,
	FVF_TexCoord1,
	FVF_TexCoord2,
	FVF_TexCoord3,
	FVF_TexCoord4,
	FVF_TexCoord5,
	FVF_TexCoord6,
	FVF_TexCoord7,
	FVF_Tangent,
	FVF_Binormal,
	FVF_MAX
};

enum ED3DBLEND
{
	NOBLEND,
    ZERO,
    ONE,
    SRCCOLOR,
    INVSRCCOLOR,
    SRCALPHA,
    INVSRCALPHA,
    DESTALPHA,
    INVDESTALPHA,
    DESTCOLOR,
    INVDESTCOLOR,
    SRCALPHASAT,
    BOTHSRCALPHA,
    BOTHINVSRCALPHA
};

/*
 * Used to specify constants for a vertex shader.
 */
struct SConstantsInfo
{
	var() SConstant Type;
	var() Plane Value;
};

struct BumpSettingInfo
{
	var() float BumpSize;
	var() float BumpLumaOffset;
	var() float BumpLumaScale;
};

/*
 * Indexed array of where the streams will show up
 * if StreamMapping[0] == NORMAL then v0 will contain the vertex normal
 */
var() array<EFixedVertexFunction> StreamMapping;

// Shader Code
var() string VertexShaderText;
var() string PixelShaderText;
var() string XVertexShaderText;
var() string XPixelShaderText;

// Shader Constants
var() SConstantsInfo VSConstants[96];
var() SConstantsInfo PSConstants[8];

// Textures to be used by each stage
var() BitmapMaterial Textures[8];
var() BumpSettingInfo BumpSettings[8];

// Blend states
var() bool ZTest;
var() bool ZWrite;
var() bool AlphaBlending;
var() bool AlphaTest;
var() bool SpecularEnable;
var() byte AlphaRef;
var() ED3DBLEND SrcBlend;
var() ED3DBLEND DestBlend;

// Native pointers
var transient const int VertexShader;
var transient const int PixelShader;

// Cache the max number of constants
var int NumVSConstants;
var int NumPSConstants;


defaultproperties
{
}
