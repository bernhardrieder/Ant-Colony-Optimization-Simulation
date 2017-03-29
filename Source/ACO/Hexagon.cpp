// Fill out your copyright notice in the Description page of Project Settings.

#include "ACO.h"
#include "Hexagon.h"
#include <limits>
#include <string>
#include "Components/TextRenderComponent.h"

float AHexagon::s_maxGlobalPheromoneLevel = 0.0f;

#if WITH_EDITOR
void AHexagon::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHexagon, TerrainType))
	{
		setTerrainSpecifics(TerrainType);
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

AHexagon::AHexagon() : m_isBlinkingActivated(false), m_hasPheromones(false)
{
	PrimaryActorTick.bCanEverTick = true;

	HexagonMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("HexagonMesh");
	HexagonMeshComponent->bGenerateOverlapEvents = true;
	HexagonMeshComponent->SetSimulatePhysics(false);

	RootComponent = HexagonMeshComponent;
	NeighbourColliderComponent = CreateDefaultSubobject<UStaticMeshComponent>("NeighbourCollider");
	NeighbourColliderComponent->bGenerateOverlapEvents = true;
	NeighbourColliderComponent->SetSimulatePhysics(false);
	NeighbourColliderComponent->SetWorldScale3D(FVector(1.2f, 1.2f, 1.f));
	NeighbourColliderComponent->SetStaticMesh(HexagonMeshComponent->GetStaticMesh());
	NeighbourColliderComponent->SetCollisionProfileName("OverlapAll");
	NeighbourColliderComponent->bVisible = false;
	NeighbourColliderComponent->bHiddenInGame = false;
	NeighbourColliderComponent->SetupAttachment(RootComponent);

	PheromoneMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("ThreadMesh");
	PheromoneMeshComponent->bGenerateOverlapEvents = false;
	PheromoneMeshComponent->SetSimulatePhysics(false);
	PheromoneMeshComponent->SetupAttachment(RootComponent);
	PheromoneMeshComponent->SetWorldLocation(FVector(0, 0, 100));
	PheromoneMeshComponent->SetWorldScale3D(FVector(0.5f, 0.5f, 0.01f));
	PheromoneMeshComponent->SetCollisionProfileName("NoCollision");
	PheromoneMeshComponent->SetHiddenInGame(true);
	PheromoneMeshComponent->SetVisibility(true);

	Text = CreateDefaultSubobject<UTextRenderComponent>("TextRender");
	Text->SetWorldRotation(FRotator(90, 0, 180));
	Text->SetupAttachment(RootComponent);
	Text->SetVerticalAlignment(EVRTA_TextCenter);
	Text->SetHorizontalAlignment(EHTA_Center);
	Text->SetTextRenderColor(FColor::Black);
	Text->Text = FText::FromString("");
	Text->WorldSize = 50.0f;
	Text->bVisible = false;

	TerrainType = ETerrainType::TT_Street;
	setTerrainSpecifics(TerrainType);
	m_pheromoneLevel = 0;
}

void AHexagon::BeginPlay()
{
	Super::BeginPlay();

	findNeighbours();

	//create own materialinstance for hexagons
	m_dynamicMaterial = HexagonMeshComponent->CreateDynamicMaterialInstance(0, BaseMaterial);
	m_pheromoneDynamicMaterial = PheromoneMeshComponent->CreateDynamicMaterialInstance(0, BaseMaterial);

	setTerrainSpecifics(TerrainType);
	PheromoneMeshComponent->SetStaticMesh(HexagonMeshComponent->GetStaticMesh());
}

void AHexagon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (m_isBlinkingActivated)
		blink(DeltaTime);

	m_elapsedWaitForTextUpdate += DeltaTime;
	if (m_elapsedWaitForTextUpdate >= 0.1f)
	{
		m_elapsedWaitForTextUpdate = 0.f;
		PheromoneMeshComponent->SetHiddenInGame(!m_hasPheromones || !m_showPheromoneLevel);
		
		if (Text->IsVisible())
			Text->SetText(FText::FromString(FString::FromInt(m_antCounter)));
	}
}

float AHexagon::GetAStarCost() const
{
	return s_maxGlobalPheromoneLevel - m_pheromoneLevel;
}

float AHexagon::GetTerrainCost() const
{
	return static_cast<float>(TerrainType);
}

float AHexagon::GetPheromoneLevel() const
{
	return m_pheromoneLevel;
}

void AHexagon::SetPheromoneLevel(float pheromones)
{
	FScopeLock lock(&criticalPheromoneSection);
	m_pheromoneLevel = pheromones;
	m_hasPheromones = m_pheromoneLevel > std::numeric_limits<float>::epsilon();
	if (m_pheromoneLevel < std::numeric_limits<float>::epsilon())
	{
		m_pheromoneLevel = 0.0f;
	}
}

float AHexagon::GetCapturedPheromoneLevel() const
{
	return m_capturedPheromoneLevel;
}

void AHexagon::CapturePheromoneLevel()
{
	FScopeLock lock(&criticalPheromoneSection);
	m_capturedPheromoneLevel = m_pheromoneLevel;
}

void AHexagon::SetIsAPath(bool val)
{
	if (val)
		SetColor(to_color(TerrainType), 1.f);
	else
		SetTerrainColor();
}

void AHexagon::SetColor(FColor color, float emission)
{
	m_dynamicMaterial->SetVectorParameterValue("BaseColor", FLinearColor(color));
	SetEmission(emission);
}

void AHexagon::SetTerrainColor()
{
	SetColor(to_color(TerrainType));
}

void AHexagon::SetFoodSource(bool yesOrNo)
{
	if (yesOrNo)
		SetColor(FColor::Red);
	else
		SetTerrainColor();

	m_isFoodSource = yesOrNo;
}

bool AHexagon::IsWalkable() const
{
	return static_cast<int>(TerrainType) != 0;
}

//per ant in worker
void AHexagon::AddPheromones(float cost)
{
	{
		FScopeLock lock(&criticalPheromoneSection);
		m_previouslyAddedPheromones += cost;
	}
	SetPheromoneLevel(m_pheromoneLevel + cost);
}

//per hexagon in worker
void AHexagon::UpdatePheromoneVisualization()
{
	static float maxPhero = 0;
	FScopeLock lock(&criticalPheromoneSection);
	if (m_hasPheromones)
	{
		maxPhero = maxPhero < s_maxGlobalPheromoneLevel ? s_maxGlobalPheromoneLevel : maxPhero;
		float pheromonesPercent =  ((m_pheromoneLevel*100) / (maxPhero*0.5f));
		SetPheromoneColor(FMath::Lerp(FLinearColor(1, 1, 0), FLinearColor(1, 0, 0), pheromonesPercent));
		m_pheromoneDynamicMaterial->SetScalarParameterValue("Emission", FMath::Clamp(pheromonesPercent, 0.f, 50.f));
	}
}

//per hexagon in worker
void AHexagon::UpdateMaxPheromonesOnTheMap()
{
	//track max pheromone level
	if (m_pheromoneLevel > s_maxGlobalPheromoneLevel)
	{
		FScopeLock lock(&criticalPheromoneSection);
		s_maxGlobalPheromoneLevel = m_pheromoneLevel;
	}
}

void AHexagon::ResetMaxPheromonesOnTheMap()
{
	s_maxGlobalPheromoneLevel = 0;
}

void AHexagon::SetPheromoneColor(FLinearColor color)
{
	m_pheromoneDynamicMaterial->SetVectorParameterValue("BaseColor", FLinearColor(color));
}

void AHexagon::ActivateBlinking(bool val, bool resetEmission)
{
	m_isBlinkingActivated = val;
	if (resetEmission)
		SetEmission(0);
}

void AHexagon::SetEmission(float emission)
{
	m_dynamicMaterial->SetScalarParameterValue("Emission", emission);
	m_currentDestinationEmission = emission;
}

bool AHexagon::hasPheromones() const
{
	return m_hasPheromones;
}

void AHexagon::ShowPheromoneLevel(bool val)
{
	m_showPheromoneLevel = val;
}

void AHexagon::ToggleShowPheromonoLevel()
{
	ShowPheromoneLevel(!m_showPheromoneLevel);
}

void AHexagon::IncrementAntCounter()
{
	inOrDecrementAntCounter(true);
}

void AHexagon::DecrementAntCounter()
{
	inOrDecrementAntCounter(false);
}

void AHexagon::ToggleShowAntCounter()
{
	Text->SetVisibility(!Text->IsVisible());
}

bool AHexagon::IsFoodSource() const
{
	return m_isFoodSource;
}

float AHexagon::GetPreviouslyAddedPheromonesAndResetVar()
{
	float val = m_previouslyAddedPheromones;
	m_previouslyAddedPheromones = 0.0f;
	return val;
}

void AHexagon::findNeighbours()
{
	TArray<AActor*> overlapping;
	NeighbourColliderComponent->GetOverlappingActors(overlapping, TSubclassOf<AHexagon>());
	for (auto actor : overlapping)
	{
		auto hex = Cast<AHexagon>(actor);
		if (hex && actor->GetName() != this->GetName() && hex->IsWalkable())
			Neighbours.Add(Cast<AHexagon>(actor));
	}
}

void AHexagon::setTerrainSpecifics(ETerrainType type)
{
	if (!BaseMaterial) return;
	m_dynamicMaterial = HexagonMeshComponent->CreateDynamicMaterialInstance(0, BaseMaterial);
	SetTerrainColor();
	if (TerrainType == ETerrainType::TT_Mountain || TerrainType == ETerrainType::TT_Anthill)
	{
		HexagonMeshComponent->SetWorldScale3D(FVector(1.f, 1.f, 2.f));
		this->SetActorLocation(FVector(this->GetActorLocation().X, this->GetActorLocation().Y, HexagonMeshComponent->GetStaticMesh()->GetBounds().BoxExtent.Z));
	}
	else
	{
		HexagonMeshComponent->SetWorldScale3D(FVector(1.f, 1.f, 1.f));
		this->SetActorLocation(FVector(this->GetActorLocation().X, this->GetActorLocation().Y, 0.f));
		Text->SetWorldLocation(FVector(this->GetActorLocation().X, this->GetActorLocation().Y, 10.f + HexagonMeshComponent->GetStaticMesh()->GetBounds().BoxExtent.Z));
	}
}

void AHexagon::blink(float deltaTime)
{
	SetEmission(m_currentDestinationEmission + m_emissionDelta);
	if (m_currentDestinationEmission < 0 || m_currentDestinationEmission > 10)
		m_emissionDelta *= -1;
}

void AHexagon::inOrDecrementAntCounter(bool increment)
{
	FScopeLock lock(&criticalAntCounterSection);
	if (increment)
		++m_antCounter;
	else
		--m_antCounter;
}
