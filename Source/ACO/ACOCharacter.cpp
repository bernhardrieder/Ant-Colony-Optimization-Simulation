// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ACO.h"
#include "ACOCharacter.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"

AACOCharacter::AACOCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	AActor::SetActorHiddenInGame(true);
}