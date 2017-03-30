// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "ACOWorker.h"
#include "GameFramework/PlayerController.h"
#include "ACOPlayerController.generated.h"

UCLASS()
class AACOPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AACOPlayerController();
	~AACOPlayerController();
	static TArray<class AHexagon*>& GetFoodSources();

protected:

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	virtual void Destroyed() override;
	// End PlayerController interface

	//food source control
	void addOrDeleteFoodSource();
	void addFoodSource(class AHexagon* hex);
	void deleteFoodSource(class AHexagon* hex);
	class AHexagon* getMouseTargetedHexagon() const;

	//pheormone level control
	void findAllGridHex();
	void toggleShowPheromoneLevels();
	void toggleShowAntCounters();
	void startACO();
	void togglePauseACO();
	void toggleShowBestPath();
	void killWorker();
	
	static TArray<class AHexagon*> s_currentFoodSources;
	TArray<class AHexagon*> m_hexGrid;
	TArray<ACOWorker*> m_acoWorkers;
	bool m_isAcoRunning = false;
	bool m_isAcoPaused = false;
};


