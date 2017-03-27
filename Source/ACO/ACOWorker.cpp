// Fill out your copyright notice in the Description page of Project Settings.

#include "ACO.h"
#include "ACOWorker.h"
#include "Hexagon.h"

int ACOWorker::s_workerCount = 0;
TArray<FScopedEvent*> ACOWorker::s_waitEvents;
FCriticalSection ACOWorker::s_criticalWaitSection;
float ACOWorker::s_traversePhaseConstantA = 0.2f;
float ACOWorker::s_traversePhaseConstantB = 1.3f;

ACOWorker::ACOWorker(const TArray<AHexagon*>& hexagons, AHexagon* anthillHex, const int& antAmount) : m_hexagons(hexagons)
{
	for (int i = 0; i < antAmount; ++i)
		m_ants.push_back(new Ant(anthillHex));

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

		//do ACO work
		traversePhase();
		markPhase();
		evaporatePhase();
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

void ACOWorker::traversePhase()
{
	GLog->Log("do traverse work " + m_name);
	for (auto ant : m_ants)
	{
		if (!ant->isCarryingFood)
		{
			/* calculate possibility p for a turn from Hex I (current position) to Hex J (neighbor)
			* pij for ant k = (Tij^a * nij^b) / (sum of: Tih^a * nih^b, where h is element of H which are all unvisited neighbours)
			* Tij ... Pheromones from I to J
			* nij = 1 / lij
			* lij ... length from I to J
			*/

			//divisor
			float sumOfUnvisited = 0.f;
			//dividend
			TMap<AHexagon*, float> dividends;

			for (auto neighbour : ant->Position->Neighbours)
			{
				if (ant->visitedPath.Contains(neighbour) || !neighbour->IsWalkable())
					continue;

				float pheromoneLevel = neighbour->GetPheromoneLevel() <= 0.0f ? 1.f : neighbour->GetPheromoneLevel(); //Tih or Tij
				float terrainCost = 1 / neighbour->GetTerrainCost(); //nih or nij

				pheromoneLevel = FMath::Pow(pheromoneLevel, s_traversePhaseConstantA); //Tih^a or Tij^a
				terrainCost = FMath::Pow(terrainCost, s_traversePhaseConstantB); //nih^b or nij^b

				float multiplication = pheromoneLevel * terrainCost;

				sumOfUnvisited += multiplication;
				dividends.Add(neighbour, multiplication);
			}

			if (sumOfUnvisited > 0.0f)
			{
				//calculate move probabilities
				TMap<AHexagon*, float> probabilities;
				for (auto& dividend : dividends)
					probabilities.Add(dividend.Key, dividend.Value / sumOfUnvisited);

				//choose new position randomly
				AHexagon* newPosition = nullptr;
				while (!newPosition)
				{
					float random = FMath::FRandRange(0.0f, 1.0f);
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

				ant->Position->DecrementAntCounter();
				ant->Position = newPosition;
				ant->Position->IncrementAntCounter();
			}
		}
	}

	waitForAllWorkers();
}

void ACOWorker::markPhase()
{
	GLog->Log("do mark work " + m_name);
	waitForAllWorkers();
}

void ACOWorker::evaporatePhase()
{
	GLog->Log("do evaporate work " + m_name);
	waitForAllWorkers();
	//reset maxPheromoneLevel
	//update pheromone Visualization
	//update max Pheromone on Map
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
