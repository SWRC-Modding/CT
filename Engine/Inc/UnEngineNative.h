/*=============================================================================
	UnEngineNative.h: Native function lookup table for static libraries.
	Copyright 2000 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Brandon Reinhart
=============================================================================*/

#ifndef UNENGINENATIVE_H
#define UNENGINENATIVE_H

DECLARE_NATIVE_TYPE(Engine,AActor);
DECLARE_NATIVE_TYPE(Engine,APawn);
DECLARE_NATIVE_TYPE(Engine,APlayerPawn);
DECLARE_NATIVE_TYPE(Engine,ADecal);
DECLARE_NATIVE_TYPE(Engine,AStatLog);
DECLARE_NATIVE_TYPE(Engine,AStatLogFile);
DECLARE_NATIVE_TYPE(Engine,AZoneInfo);
DECLARE_NATIVE_TYPE(Engine,AWarpZoneInfo);
DECLARE_NATIVE_TYPE(Engine,ALevelInfo);
DECLARE_NATIVE_TYPE(Engine,AGameInfo);
DECLARE_NATIVE_TYPE(Engine,ANavigationPoint);

DECLARE_NATIVE_TYPE(Engine,UCanvas);
DECLARE_NATIVE_TYPE(Engine,UConsole);
DECLARE_NATIVE_TYPE(Engine,UScriptedTexture);

#define AUTO_INITIALIZE_REGISTRANTS_ENGINE \
	AMutator::StaticClass(); \
	ULodMesh::StaticClass(); \
	AActor::StaticClass(); \
	ALight::StaticClass();\
	AWeapon::StaticClass();\
	ALevelInfo::StaticClass();\
	AGameInfo::StaticClass();\
	ACamera::StaticClass();\
	AZoneInfo::StaticClass();\
	ASkyZoneInfo::StaticClass();\
	APathNode::StaticClass();\
	ANavigationPoint::StaticClass();\
	AScout::StaticClass();\
	AInterpolationPoint::StaticClass();\
	ADecoration::StaticClass();\
	AProjectile::StaticClass();\
	AWarpZoneInfo::StaticClass();\
	ATeleporter::StaticClass();\
	APlayerStart::StaticClass();\
	AKeypoint::StaticClass();\
	AInventory::StaticClass();\
	AInventorySpot::StaticClass();\
	ATriggers::StaticClass();\
	ATrigger::StaticClass();\
	ATriggerMarker::StaticClass();\
	AButtonMarker::StaticClass();\
	AWarpZoneMarker::StaticClass();\
	AHUD::StaticClass();\
	AMenu::StaticClass();\
	ASavedMove::StaticClass();\
	ACarcass::StaticClass();\
	ALiftCenter::StaticClass();\
	ALiftExit::StaticClass();\
	AInfo::StaticClass();\
	AReplicationInfo::StaticClass();\
	APlayerReplicationInfo::StaticClass();\
	AInternetInfo::StaticClass();\
	AStatLog::StaticClass();\
	AStatLogFile::StaticClass();\
	AGameReplicationInfo::StaticClass();\
	ULevelSummary::StaticClass();\
	Alocationid::StaticClass();\
	ADecal::StaticClass();\
	ASpawnNotify::StaticClass();\
	AAmmo::StaticClass();\
	APickup::StaticClass();\
	USound::StaticClass();\
	UMusic::StaticClass();\
	UAudioSubsystem::StaticClass();\
	UClient::StaticClass();\
	UViewport::StaticClass();\
	UCanvas::StaticClass();\
	UChannel::StaticClass();\
	UControlChannel::StaticClass();\
	UActorChannel::StaticClass();\
	UFileChannel::StaticClass();\
	UConsole::StaticClass();\
	UNetConnection::StaticClass();\
	UDemoPlayPendingLevel::StaticClass();\
	UDemoRecConnection::StaticClass();\
	UDemoRecDriver::StaticClass();\
	ABrush::StaticClass();\
	UEngine::StaticClass();\
	URenderBase::StaticClass();\
	URenderDevice::StaticClass();\
	URenderIterator::StaticClass();\
	UServerCommandlet::StaticClass();\
	UPolys::StaticClass();\
	UFont::StaticClass();\
	UGameEngine::StaticClass();\
	UInput::StaticClass();\
	ULevelBase::StaticClass();\
	ULevel::StaticClass();\
	UMesh::StaticClass();\
	UBspNodes::StaticClass();\
	UBspSurfs::StaticClass();\
	UVectors::StaticClass();\
	UVerts::StaticClass();\
	UModel::StaticClass();\
	AMover::StaticClass();\
	UPackageMapLevel::StaticClass();\
	UNetDriver::StaticClass();\
	APawn::StaticClass();\
	APlayerPawn::StaticClass();\
	UPendingLevel::StaticClass();\
	UNetPendingLevel::StaticClass();\
	UPlayer::StaticClass();\
	UPrimitive::StaticClass();\
	UScriptedTexture::StaticClass();\
	UBitmap::StaticClass();\
	UTexture::StaticClass(); \
	UPalette::StaticClass(); \
	UAnimation::StaticClass(); 
#endif
