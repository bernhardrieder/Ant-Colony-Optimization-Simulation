// Fill out your copyright notice in the Description page of Project Settings.

#include "ACO.h"
#include "ACOWorker.h"
#include "Hexagon.h"
#include "Pathfinding.h"
#include "ACOPlayerController.h"

int ACOWorker::s_workerCount = 0;
TArray<FScopedEvent*> ACOWorker::s_waitEvents;
FCriticalSection ACOWorker::s_criticalWaitSection;
float ACOWorker::s_traversePhaseConstantA = 5.f;
float ACOWorker::s_traversePhaseConstantB = 9.f;
float ACOWorker::s_evaporationCoefficentP = 0.05f;
int ACOWorker::s_iterationCounter = 0;
bool ACOWorker::s_updateByOneWorker = true;
FCriticalSection ACOWorker::criticalStatic;
AHexagon* ACOWorker::s_anthill = nullptr;
std::vector<class AHexagon*> ACOWorker::s_pathHexagons;
bool ACOWorker::s_renderBestPath = false;

ACOWorker::ACOWorker(const TArray<AHexagon*>& hexagons, AHexagon* anthillHex, const int& antAmount) : m_hexagons(hexagons)
{
	for (int i = 0; i < antAmount; ++i)
		m_ants.push_back(new Ant(anthillHex));

	s_anthill = anthillHex;
	m_randomStream.Initialize(1610585006 * FDateTime::Now().GetMillisecond());

	m_name = "ACO_Thread_";
	m_name.AppendInt(++s_workerCount);
	Thread = FRunnableThread::Create(this, *m_name, 0, TPri_Normal); //windows default = 8mb for thread, could specify more

	if (!Thread)
	{
		UE_LOG(LogACO, Error, TEXT("Failed to create %s!"), *m_name);
	}
	else
	{
		UE_LOG(LogACO, Log, TEXT("%s created with %d different Hexagons and %d Ants!"), *m_name, hexagons.Num(), antAmount);
	}
}

ACOWorker::~ACOWorker()
{
	//free waiting waitEvents
	for (auto a : s_waitEvents)
	{
		if (a)
			a->Trigger();
	}
	s_waitEvents.Empty();

	//kill thread
	if (Thread)
	{
		Thread->Kill();
		delete Thread;
		Thread = nullptr;
	}

	//decrement overall counter
	--s_workerCount;

	for (auto a : m_ants)
		delete a;

	UE_LOG(LogACO, Log, TEXT("%s destroyed!"), *m_name);
}

bool ACOWorker::Init()
{
	return true;
}

uint32 ACOWorker::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03f);

	while (StopTaskCounter.GetValue() == 0)
	{
		//prevent thread from using too many resources
		FPlatformProcess::Sleep(0.01);
		s_updateByOneWorker = true;

		//do ACO work
		traversePhase();
		markPhase();
		evaporatePhase();

		//other
		updateThingsByOneWorker();
	}

	return 0;
}

void ACOWorker::Stop()
{
	StopTaskCounter.Increment();
}

void ACOWorker::Pause() const
{
	Thread->Suspend(true);
}

void ACOWorker::Unpause() const
{
	Thread->Suspend(false);
}

void ACOWorker::ToggleShowBestPath()
{
	s_renderBestPath = !s_renderBestPath;
}

void ACOWorker::traversePhase()
{
	for (auto ant : m_ants)
	{
		AHexagon* newPosition = nullptr;
		if (!ant->isCarryingFood && ant->isSearchingFood)
		{
			//add current position for finding the path back to anthill
			ant->visitedPath.Push(ant->Position);

			/* calculate probability p for a turn from Hex I (current position) to Hex J (neighbor)
			* pij for ant k = (Tij^a * nij^b) / (sum of: Tih^a * nih^b, where h is element of H which are all unvisited neighbours)
			* Tij ... Pheromones from I to J
			* nij = 1 / lij
			* lij ... length from I to J
			*/

			//probability divisor
			float sumOfUnvisitedNodes = 0.f;
			//probability dividend
			TMap<AHexagon*, float> dividends;
			int visitableNeighbours = ant->Position->Neighbours.Num();

			//iterate through every neighbour
			for (auto neighbour : ant->Position->Neighbours)
			{
				//neighbour visitable?
				if (ant->visitedPath.Contains(neighbour) || !neighbour->IsWalkable())
				{
					--visitableNeighbours;
					continue;
				}

				//Tih or Tij
				float pheromoneLevel = neighbour->GetPheromoneLevel() <= 0.0f ? 1.f : neighbour->GetPheromoneLevel();
				//nih or nij
				float terrainCost = 1 / neighbour->GetTerrainCost();

				//Tih^a or Tij^a
				pheromoneLevel = FMath::Pow(pheromoneLevel, s_traversePhaseConstantA);
				//nih^b or nij^b
				terrainCost = FMath::Pow(terrainCost, s_traversePhaseConstantB);

				float multiplication = pheromoneLevel * terrainCost;

				sumOfUnvisitedNodes += multiplication;
				//add multiplication with costs from I to J
				dividends.Add(neighbour, multiplication);
			}

			//if there are neighbours to visit
			if (visitableNeighbours > 0)
			{
				//calculate move probabilities
				TMap<AHexagon*, float> probabilities;
				for (auto& dividend : dividends)
					probabilities.Add(dividend.Key, dividend.Value / sumOfUnvisitedNodes);

				//choose new position randomly
				while (!newPosition)
				{
					float random = m_randomStream.FRandRange(0.0f, 1.0f);
					for (auto& prob : probabilities)
					{
						if (random < prob.Value)
						{
							newPosition = prob.Key;
							break;
						}
						random -= prob.Value;

					}
				}
			}
			else
			{
				//no visitable node
				//return to anthill
				ant->isSearchingFood = false;
			}
		}
		if (!ant->isSearchingFood || ant->isCarryingFood)
		{
			//go back to anthill
			newPosition = ant->visitedPath.Pop();
			if (newPosition == ant->Position)
				newPosition = ant->visitedPath.Pop();
		}
		ant->Position->DecrementAntCounter();
		ant->Position = newPosition;
		ant->Position->IncrementAntCounter();

		//is new pos foodsource?
		if (newPosition->IsFoodSource() && ant->isSearchingFood)
		{
			ant->isCarryingFood = true;
			ant->isSearchingFood = false;
			ant->pheromonesPerNode = Pathfinding::AStarSearchHeuristic(ant->visitedPath[0], newPosition) / ant->visitedPath.Num() + 1;
		}
		else if (!ant->isSearchingFood && newPosition->GetTerrainCost() == static_cast<int>(ETerrainType::TT_Anthill))
		{
			//is back in anthill
			ant->isCarryingFood = false;
			ant->isSearchingFood = true;
		}
	}

	waitForAllWorkers();
}

void ACOWorker::markPhase()
{
	for (auto ant : m_ants)
	{
		if (ant->isCarryingFood)
			ant->Position->AddPheromones(ant->pheromonesPerNode);
	}
	waitForAllWorkers();
}

void ACOWorker::evaporatePhase()
{
	for (auto a : m_hexagons)
	{
		a->SetPheromoneLevel((1.0f - s_evaporationCoefficentP) * a->GetCapturedPheromoneLevel() + a->GetPreviouslyAddedPheromonesAndResetVar());
		a->CapturePheromoneLevel();
		a->UpdateMaxPheromonesOnTheMap();
		a->UpdatePheromoneVisualization();
	}
	waitForAllWorkers();
}

void ACOWorker::waitForAllWorkers()
{
	FScopedEvent myEvent;
	FScopeLock lock(&s_criticalWaitSection);
	s_waitEvents.Push(&myEvent);
	if (s_waitEvents.Num() == s_workerCount)
	{
		for (auto a : s_waitEvents)
			a->Trigger();
		s_waitEvents.Empty();
	}
}

void ACOWorker::updateThingsByOneWorker()
{
	{
		FScopeLock lock(&criticalStatic);
		if (s_updateByOneWorker)
		{
			//reset current best path hexs
			for (auto hex : s_pathHexagons)
				hex->SetIsAPath(false);
			s_pathHexagons.clear();

			if (s_renderBestPath)
			{
				// do pathfinding for each foodsource
				auto foodSources = AACOPlayerController::GetFoodSources();
				for (const auto& foodSource : foodSources)
				{
					std::unordered_map<AHexagon*, AHexagon*> came_from;
					Pathfinding::AStarSearch(s_anthill, foodSource, came_from);
					for (auto pathHex : Pathfinding::ReconstructPath(s_anthill, foodSource, came_from))
					{
						if (pathHex != s_anthill && pathHex != foodSource)
						{
							s_pathHexagons.push_back(pathHex);
							pathHex->SetIsAPath(true);
						}
					}
				}
			}

			GLog->Log("Iteration: " + FString::FromInt(++s_iterationCounter));
			AHexagon::ResetMaxPheromonesOnTheMap();
			s_updateByOneWorker = false;
		}
	}
	waitForAllWorkers();
}
