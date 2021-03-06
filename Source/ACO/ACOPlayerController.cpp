// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ACO.h"
#include "ACOPlayerController.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "ACOCharacter.h"
#include "Hexagon.h"
#include "EngineUtils.h"
#include "ACOWorker.h"

TArray<class AHexagon*> AACOPlayerController::s_currentFoodSources;

AACOPlayerController::AACOPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

AACOPlayerController::~AACOPlayerController()
{
	for (auto a : m_acoWorkers)
		delete a;
}

TArray<AHexagon*>& AACOPlayerController::GetFoodSources()
{
	return s_currentFoodSources;
}

void AACOPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
}

void AACOPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	if (m_worldHex.Num() == 0)
		findAllHexagonsInWorld();

	InputComponent->BindAction("AddOrDeleteFoodSource", IE_Pressed, this, &AACOPlayerController::addOrDeleteFoodSource);
	InputComponent->BindAction("ToggleShowPheromoneLevels", IE_Pressed, this, &AACOPlayerController::toggleShowPheromoneLevels);
	InputComponent->BindAction("StartACO", IE_Pressed, this, &AACOPlayerController::startACO);
	InputComponent->BindAction("TogglePauseACO", IE_Pressed, this, &AACOPlayerController::togglePauseACO);
	InputComponent->BindAction("ToggleShowBestPath", IE_Pressed, this, &AACOPlayerController::toggleShowBestPath);
}

void AACOPlayerController::Destroyed()
{
	APlayerController::Destroyed();
	if (m_isAcoPaused)
		togglePauseACO();
	killACOWorker();
}

void AACOPlayerController::addOrDeleteFoodSource()
{
	auto hex = getMouseTargetedHexagon();
	if (!hex || !hex->IsWalkable() || hex->GetTerrainType() == ETerrainType::TT_Anthill) return;

	bool containsFoodSource = s_currentFoodSources.Contains(hex);
	if (containsFoodSource)
		deleteFoodSource(hex);
	else
		addFoodSource(hex);

	hex->SetFoodSource(!containsFoodSource);
}

void AACOPlayerController::addFoodSource(AHexagon* hex)
{
	GLog->Log("added food source!");
	s_currentFoodSources.Add(hex);
	hex->ActivateBlinking(true);
}

void AACOPlayerController::deleteFoodSource(AHexagon* hex)
{
	GLog->Log("deleted food source!");
	hex->ActivateBlinking(false);
	s_currentFoodSources.Remove(hex);
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

void AACOPlayerController::findAllHexagonsInWorld()
{
	for (TActorIterator<AHexagon> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		m_worldHex.Add(*ActorItr);
}

void AACOPlayerController::toggleShowPheromoneLevels()
{
	for (auto a : m_worldHex)
		a->ToggleShowPheromonoLevel();
}

void AACOPlayerController::startACO()
{
	if (m_isAcoRunning)
	{
		UE_LOG(LogACO, Error, TEXT("ACO is already running!"));
		return;
	}

	int antAmount = 5000;
	//how much threads?
	int acoThreads = 10;
	AHexagon* antHill = nullptr;
	TArray<AHexagon*> usableHex;

	for (auto a : m_worldHex)
	{
		if (a->IsWalkable())
		{
			//locate anthill hexagon
			if (!antHill && a->GetTerrainType() == ETerrainType::TT_Anthill)
				antHill = a;

			//locate usable hexs
			if(antHill != a)
				usableHex.Push(a);
		}
	}
	if (!antHill || usableHex.Num() == 0)
	{
		UE_LOG(LogACO, Error, TEXT("Couldn't find Anthill OR there is no usable/ walkable Hexagon!!!"));
		return;
	}

	/** split the resoures and create the thread worker */
	int hexFractionPerThread = usableHex.Num() / acoThreads;
	int antAmountPerThread = antAmount / acoThreads;

	
	for (int i = 1; i <= acoThreads; ++i)
	{
		TArray<AHexagon*> threadHex;
		int threadAntAmount;
		if (i == acoThreads)
		{
			//remaining hex/ants
			threadAntAmount = antAmount;
			threadHex = usableHex;
		}
		else
		{
			antAmount -= antAmountPerThread;
			threadAntAmount = antAmountPerThread;
			for (int x = 0; x < hexFractionPerThread; ++x)
				threadHex.Push(usableHex.Pop());
		}
		m_acoWorkers.Push(new ACOWorker(threadHex, antHill, threadAntAmount));
	}

	m_isAcoRunning = true;
	m_isAcoPaused = false;
}

void AACOPlayerController::togglePauseACO()
{
	if (m_acoWorkers.Num() == 0) return;
	for(auto a : m_acoWorkers)
	{
		if (m_isAcoPaused)
			a->Unpause();
		else
			a->Pause();
	}
	m_isAcoPaused = !m_isAcoPaused;
}

void AACOPlayerController::toggleShowBestPath()
{
	ACOWorker::ToggleShowBestPath();
}

void AACOPlayerController::killACOWorker()
{
	for (auto a : m_acoWorkers)
		a->Stop();

	FPlatformProcess::Sleep(0.2);
}
