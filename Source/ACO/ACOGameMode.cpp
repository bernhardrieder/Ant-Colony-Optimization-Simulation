// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ACO.h"
#include "ACOGameMode.h"
#include "ACOPlayerController.h"
#include "ACOCharacter.h"

AACOGameMode::AACOGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AACOPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Empty_Character_BP"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}