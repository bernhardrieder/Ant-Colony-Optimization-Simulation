// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Hexagon.generated.h"

UENUM()
enum class ETerrainType : uint8
{
	TT_Mountain = 0		UMETA(DisplayName = "Mountain"),
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
		UStaticMeshComponent* ThreadMeshComponent;

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

	float GetCost(bool withThreat = true) const;
	float GetTerrainCost() const;
	float GetThreatCost() const;
	void SetPathColor(bool val, FColor color = FColor::Orange);
	void SetColor(FColor color, float emission = 0);
	void SetTerrainColor();
	void SetDestinationColor(FColor color = FColor::Red);
	bool IsWalkable() const;
	void SetThreat(float cost, FLinearColor color);
	void AddThreatCost(float cost);
	void SetThreatColor(FLinearColor color);
	void ActivateBlinking(bool val, bool resetEmission = true);
	void SetEmission(float emission);
	bool IsThreat() const;

	TArray<AHexagon*> Neighbours;

private:
	void setTerrainSpecifics(ETerrainType type);
	void blink(float deltaTime);

	UMaterialInstanceDynamic* m_dynamicMaterial;
	UMaterialInstanceDynamic* m_threatDynamicMaterial;
	float m_threatCost = 0;
	bool m_isBlinkingActivated;
	float m_currentDestinationEmission = 0;
	float m_emissionDelta = 0.3f;
	bool m_isThreat = false;

};
