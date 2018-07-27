/*=============================================================================
	UnDDraw.h: Unreal DirectDraw definitions
	Required for S3TC compression/decompression

	Created by Seth Sowerby
=============================================================================*/

#ifndef __DDRAW_INCLUDED__

// DirectDraw Surface definitions
// Needed for S3TC compression/decompression code
#ifdef __cplusplus
extern "C" {
#endif

	//
// For compilers that don't support nameless unions, do a
//
// #define NONAMELESSUNION
//
// before #include <ddraw.h>
//
#ifndef DUMMYUNIONNAMEN
#if defined(__cplusplus) || !defined(NONAMELESSUNION)
#define DUMMYUNIONNAMEN(n)
#else
#define DUMMYUNIONNAMEN(n)      u##n
#endif
#endif

typedef long LONG;
typedef void *LPVOID;

//
// DDCOLORKEY
//
typedef struct _DDCOLORKEY
{
    DWORD	dwColorSpaceLowValue;	// low boundary of color space that is to
					// be treated as Color Key, inclusive
    DWORD	dwColorSpaceHighValue;	// high boundary of color space that is
					// to be treated as Color Key, inclusive
} DDCOLORKEY;

//
// DDSCAPS
//
typedef struct _DDSCAPS
{
    DWORD	dwCaps;		// capabilities of surface wanted
} DDSCAPS;

//
// DDPIXELFORMAT
//
typedef struct _DDPIXELFORMAT
{
    DWORD	dwSize;			// size of structure
    DWORD	dwFlags;		// pixel format flags
    DWORD	dwFourCC;		// (FOURCC code)
    union
    {
	DWORD	dwRGBBitCount;		// how many bits per pixel
	DWORD	dwYUVBitCount;		// how many bits per pixel
	DWORD	dwZBufferBitDepth;	// how many total bits/pixel in z buffer (including any stencil bits)
	DWORD	dwAlphaBitDepth;	// how many bits for alpha channels
	DWORD   dwLuminanceBitCount;	// how many bits per pixel
        DWORD   dwBumpBitCount;         // how many bits per "buxel", total
    } DUMMYUNIONNAMEN(1);
    union
    {
	DWORD	dwRBitMask;		// mask for red bit
	DWORD	dwYBitMask;		// mask for Y bits
	DWORD   dwStencilBitDepth;	// how many stencil bits (note: dwZBufferBitDepth-dwStencilBitDepth is total Z-only bits)
	DWORD   dwLuminanceBitMask;	// mask for luminance bits
        DWORD   dwBumpDuBitMask;        // mask for bump map U delta bits
    } DUMMYUNIONNAMEN(2);
    union
    {
	DWORD	dwGBitMask;		// mask for green bits
	DWORD	dwUBitMask;		// mask for U bits
	DWORD   dwZBitMask;		// mask for Z bits
        DWORD   dwBumpDvBitMask;        // mask for bump map V delta bits
    } DUMMYUNIONNAMEN(3);
    union
    {
	DWORD	dwBBitMask;		// mask for blue bits
	DWORD	dwVBitMask;		// mask for V bits
	DWORD   dwStencilBitMask;	// mask for stencil bits
        DWORD   dwBumpLuminanceBitMask; // mask for luminance in bump map
    } DUMMYUNIONNAMEN(4);
    union
    {
	DWORD	dwRGBAlphaBitMask;	// mask for alpha channel
	DWORD	dwYUVAlphaBitMask;	// mask for alpha channel
        DWORD   dwLuminanceAlphaBitMask;// mask for alpha channel
	DWORD	dwRGBZBitMask;		// mask for Z channel
	DWORD	dwYUVZBitMask;		// mask for Z channel
    } DUMMYUNIONNAMEN(5);
} DDPIXELFORMAT;

//
// DDSURFACEDESC
//
typedef struct _DDSURFACEDESC
{
    DWORD		dwSize;			// size of the DDSURFACEDESC structure
    DWORD		dwFlags;		// determines what fields are valid
    DWORD		dwHeight;		// height of surface to be created
    DWORD		dwWidth;		// width of input surface
    union
    {
        long		lPitch;			// distance to start of next line (return value only)
        DWORD           dwLinearSize;           // Formless late-allocated optimized surface size
    } DUMMYUNIONNAMEN(1);
    DWORD		dwBackBufferCount;	// number of back buffers requested
    union
    {
        DWORD           dwMipMapCount;          // number of mip-map levels requested
	DWORD		dwZBufferBitDepth;	// depth of Z buffer requested
	DWORD		dwRefreshRate;		// refresh rate (used when display mode is described)
    } DUMMYUNIONNAMEN(2);
    DWORD		dwAlphaBitDepth;	// depth of alpha buffer requested
    DWORD		dwReserved;		// reserved
    LPVOID		lpSurface;		// pointer to the associated surface memory
    DDCOLORKEY		ddckCKDestOverlay;	// color key for destination overlay use
    DDCOLORKEY		ddckCKDestBlt;		// color key for destination blt use
    DDCOLORKEY		ddckCKSrcOverlay;	// color key for source overlay use
    DDCOLORKEY		ddckCKSrcBlt;		// color key for source blt use
    DDPIXELFORMAT	ddpfPixelFormat;	// pixel format description of the surface
    DDSCAPS		ddsCaps;		// direct draw surface capabilities
} DDSURFACEDESC;

/*
 * ddsCaps field is valid.
 */
#define DDSD_CAPS		0x00000001l	// default

/*
 * dwHeight field is valid.
 */
#define DDSD_HEIGHT		0x00000002l

/*
 * dwWidth field is valid.
 */
#define DDSD_WIDTH		0x00000004l

/*
 * lPitch is valid.
 */
#define DDSD_PITCH		0x00000008l

/*
 * lpSurface is valid.
 */
#define DDSD_LPSURFACE		0x00000800l

/*
 * ddpfPixelFormat is valid.
 */
#define DDSD_PIXELFORMAT	0x00001000l

/*
 * The FourCC code is valid.
 */
#define DDPF_FOURCC				0x00000004l

/*
 * The RGB data in the pixel format structure is valid.
 */
#define DDPF_RGB				0x00000040l


#ifdef __cplusplus
};
#endif*/

#endif
