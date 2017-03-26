// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/PlayerController.h"
#include "ACOPlayerController.generated.h"

UCLASS()
class AACOPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AACOPlayerController();

protected:

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	// End PlayerController interface

	void addOrDeleteFoodSource();
	void addFoodSource(class AHexagon* hex);
	void deleteFoodSource(class AHexagon* hex);
	class AHexagon* getMouseTargetedHexagon() const;
	
	TArray<class AHexagon*> m_currentFoodSources;
};


