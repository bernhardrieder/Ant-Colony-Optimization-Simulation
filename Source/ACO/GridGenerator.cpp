// Fill out your copyright notice in the Description page of Project Settings.

#include "ACO.h"
#include "GridGenerator.h"
#include "Hexagon.h"

#if WITH_EDITOR
void AGridGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (HexagonBP && PropertyName == GET_MEMBER_NAME_CHECKED(AGridGenerator, CreateGrid) && CreateGrid)
	{
		TArray<AActor*> hexagonActors;
		for (int i = 0; i < Elements.X*Elements.Y; ++i)
		{
			auto hexActor = GetWorld()->SpawnActor(HexagonBP);
			hexActor->SetFolderPath("/HexGrid");
			hexagonActors.Add(hexActor);
		}
		alingHexagons(hexagonActors);
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

// Sets default values
AGridGenerator::AGridGenerator()
{
}

void AGridGenerator::alingHexagons(TArray<AActor*> hexagonActors) const
{
	auto hex = Cast<AHexagon>(hexagonActors[0]);
	FBoxSphereBounds hexBounds;
	if (hex)
		hexBounds = hex->GetMeshComponent()->GetStaticMesh()->GetBounds();

	for (int y = 0; y < Elements.Y; ++y)
	{
		for (int x = 0; x < Elements.X; ++x)
		{
			auto actor = hexagonActors[y*Elements.X + x];
			float yCoord = y * (2 * hexBounds.BoxExtent.Y);
			float xCoord = x * (1.5*hexBounds.BoxExtent.X);
			if (x % 2 == 1)
				yCoord -= hexBounds.BoxExtent.Y;
			actor->SetActorLocation(FVector(xCoord, yCoord, 0.f));
		}
	}
}
