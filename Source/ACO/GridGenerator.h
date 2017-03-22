// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "GridGenerator.generated.h"

UCLASS()
class ACO_API AGridGenerator : public AActor
{
	GENERATED_BODY()

#if WITH_EDITOR
		virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(EditAnywhere)
		FVector2D Elements;
	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> HexagonBP;
	UPROPERTY(EditAnywhere)
		bool CreateGrid;
public:
	// Sets default values for this actor's properties
	AGridGenerator();

private:
	void alingHexagons(TArray<AActor*> hexagonActors) const;
};
