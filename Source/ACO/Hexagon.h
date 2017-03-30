// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Hexagon.generated.h"

UENUM()
enum class ETerrainType : uint8
{
	TT_Mountain = 0		UMETA(DisplayName = "Mountain"),
	TT_Anthill = 1		UMETA(DisplayName = "Anthill"),
	TT_Street = 10		UMETA(DisplayName = "Street"),
	TT_Grass = 20		UMETA(DisplayName = "Grass"),
	TT_Sand = 30		UMETA(DisplayName = "Sand"),
	TT_Mud = 40			UMETA(DisplayName = "Mud"),
	TT_Water = 50		UMETA(DisplayName = "Water")
};

static FColor to_color(ETerrainType type)
{
	switch (type)
	{
	case ETerrainType::TT_Mountain: return FColor(159, 182, 205); //slategray
	case ETerrainType::TT_Anthill: return FColor(43, 29, 14); //darkbworn
	case ETerrainType::TT_Grass: return FColor(34, 139, 34); //forrestgreen
	case ETerrainType::TT_Sand: return FColor(255, 215, 0); //gold
	case ETerrainType::TT_Water: return FColor(28, 134, 238); //dodgerblue
	case ETerrainType::TT_Street: return FColor(105, 105, 105); //grey
	case ETerrainType::TT_Mud: return FColor(139, 69, 19); //brown
	default:
		return FColor::Red;
	}
}

UCLASS()
class ACO_API AHexagon : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = Hexagon)
		UStaticMeshComponent* HexagonMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = Hexagon)
		UStaticMeshComponent* PheromoneMeshComponent;

	UPROPERTY(EditDefaultsOnly, Category = Hexagon)
		UStaticMeshComponent* NeighbourColliderComponent;

	UPROPERTY(EditAnywhere, Category = Hexagon)
		ETerrainType TerrainType;

	UPROPERTY(EditAnywhere, Category = Hexagon)
		UMaterial* BaseMaterial;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	AHexagon();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	UStaticMeshComponent* GetMeshComponent() const { return HexagonMeshComponent; };

	float GetPheromoneAStarCost() const;
	float GetTerrainCost() const;
	float GetPheromoneLevel() const;
	void SetPheromoneLevel(float pheromones);
	float GetCapturedPheromoneLevel() const;
	void CapturePheromoneLevel();
	void SetIsAPath(bool val);
	void SetColor(FColor color, float emission = 0);
	void SetTerrainColor();
	void SetFoodSource(bool yesOrNo);
	bool IsWalkable() const;
	void AddPheromones(float cost);
	void UpdatePheromoneVisualization();
	void UpdateMaxPheromonesOnTheMap();
	static void ResetMaxPheromonesOnTheMap();
	void SetPheromoneColor(FLinearColor color);
	void ActivateBlinking(bool val, bool resetEmission = true);
	void SetEmission(float emission);
	bool HasPheromones() const;
	void ShowPheromoneLevel(bool val);
	void ToggleShowPheromonoLevel();
	bool IsFoodSource() const;
	float GetPreviouslyAddedPheromonesAndResetVar();
	ETerrainType GetTerrainType() const;
	TArray<AHexagon*>& GetNeighbourHexagons();

private:
	void findNeighbourHexagons();
	void setTerrainSpecifics(ETerrainType type);
	void materialBlinkUpdate(float deltaTime);

	//other
	TArray<AHexagon*> m_neighbourHexagons;
	bool m_isFoodSource = false;

	//materials
	UMaterialInstanceDynamic* m_dynamicMaterial;
	bool m_isMaterialBlinkingActivated;
	float m_currentDestinationMaterialEmission = 0;
	float m_materialEmissionDelta = 0.3f;

	//pheromones
	UMaterialInstanceDynamic* m_pheromoneDynamicMaterial;
	float m_pheromoneLevel = 0;
	bool m_hasPheromones = true;
	bool m_showPheromoneLevel = false;
	float m_previouslyAddedPheromones = 0.f;
	float m_capturedPheromoneLevel = 0.f;
	float m_elapsedWaitTimeForPheromoneVisualization = 0;
	static float s_maxGlobalPheromoneLevel;

	//Multithreading
	FCriticalSection m_criticalPheromoneSection;

};
