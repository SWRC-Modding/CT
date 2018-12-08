/*=============================================================================
	UnCId.h: Cache Id's for all global Unreal objects.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*----------------------------------------------------------------------------
	Cache ID's.
----------------------------------------------------------------------------*/

//
// 8-bit base Cache ID's.
//
enum ECacheIDBase{
	CID_ShadowMap			= 0x15,
	CID_IlluminationMap		= 0x16,
	CID_LightPalette		= 0x17,
	CID_StaticMap			= 0x18,
	CID_DepthLineTable		= 0x1C,
	CID_TweenAnim			= 0x1D,
	CID_TriPalette			= 0x1E,
	CID_InputMap			= 0x1F,
	CID_VolumetricScaler	= 0x20,
	CID_RenderPalette		= 0x25,
	CID_RenderFogMap		= 0x26,
	CID_CoronaCache			= 0x27,
	CID_PolyPalette         = 0x28,
	CID_PolyMMXPalette      = 0x29,
	CID_SurfPalette         = 0x2A,
	CID_SurfMMXPalette      = 0x2B,
	CID_LitTilePal          = 0x2C,
	CID_LitTileTrans        = 0x2D,
	CID_LitTileMMX          = 0x2E,
	CID_LitTileMod          = 0x2F,
	CID_ActorLightCache     = 0x30,
	CID_DynamicMap          = 0x31,
	CID_GlidePal            = 0x32,
	CID_BumpNormals         = 0x33,
	CID_SkeletalData        = 0x34,
	CID_RenderTexture		= 0xE0,
	CID_RenderVertices		= 0xE1,
	CID_RenderIndices		= 0xE2,
	CID_CollisionData		= 0xE3,
	CID_MAX					= 0xff,
};

/*----------------------------------------------------------------------------
	Functions.
----------------------------------------------------------------------------*/

inline QWORD MakeCacheID(ECacheIDBase Base, UObject* Frame){
	return (Base) + ((Frame ? (QWORD)Frame->GetIndex() : (QWORD)0) << 8);
}

inline QWORD MakeCacheID(ECacheIDBase Base, UObject* Obj, UObject* Frame){
	return (Base) + ((Obj ? Obj->GetIndex() : 0) << 8) + ((Frame ? (QWORD)Frame->GetIndex() : (QWORD)0) << 32);
}

inline QWORD MakeCacheID(ECacheIDBase Base, DWORD Word, DWORD Byte, UObject* Frame){
	return (Base) + (Byte << 8) + (Word << 16) + ((Frame ? (QWORD)Frame->GetIndex() : (QWORD)0) << 32);
}

inline QWORD MakeCacheID(ECacheIDBase Base, DWORD ByteA, DWORD ByteB, DWORD ByteC, UObject* Frame){
	return (Base) + (ByteA << 8) + (ByteB << 16) + (ByteC << 24) + ((Frame ? (QWORD)Frame->GetIndex() : (QWORD)0) << 32);
}

inline QWORD MakeCacheID(ECacheIDBase Base, QWORD Q){
	return Base + (Q & ~(QWORD)CID_MAX);
}

inline QWORD MakeCacheID(ECacheIDBase Base){
	return Base + ((GMakeCacheIDIndex++) << 8);
}

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
