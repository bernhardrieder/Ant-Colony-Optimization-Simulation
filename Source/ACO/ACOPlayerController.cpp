// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ACO.h"
#include "ACOPlayerController.h"
#include "AI/Navigation/NavigationSystem.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "ACOCharacter.h"
#include "Hexagon.h"

AACOPlayerController::AACOPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}


void AACOPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
}

void AACOPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("AddOrDeleteFoodSource", IE_Pressed, this, &AACOPlayerController::addOrDeleteFoodSource);
}

void AACOPlayerController::addOrDeleteFoodSource()
{
	auto hex = getMouseTargetedHexagon();
	if (!hex) return;

	if (m_currentFoodSources.Contains(hex))
		deleteFoodSource(hex);
	else
		addFoodSource(hex);
}

void AACOPlayerController::addFoodSource(AHexagon* hex)
{
	if (hex->IsWalkable())
	{
		GLog->Log("added food source!");
		m_currentFoodSources.Add(hex);
		hex->ActivateBlinking(true);
	}
}

void AACOPlayerController::deleteFoodSource(AHexagon* hex)
{
	GLog->Log("deleted food source!");
	hex->ActivateBlinking(false);
	m_currentFoodSources.Remove(hex);

}

AHexagon* AACOPlayerController::getMouseTargetedHexagon() const
{
	AHexagon* resultHex = nullptr;
	FHitResult hitResult;
	GetHitResultUnderCursor(ECC_Visibility, true, hitResult);
	if (hitResult.bBlockingHit)
		resultHex = Cast<AHexagon>(hitResult.Actor.Get());

	return resultHex;
}
