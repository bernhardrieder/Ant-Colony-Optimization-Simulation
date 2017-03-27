// Fill out your copyright notice in the Description page of Project Settings.

#include "ACO.h"
#include "ACOWorker.h"
#include "Hexagon.h"

int ACOWorker::s_workerCount = 0;
TArray<FScopedEvent*> ACOWorker::s_waitEvents;
FCriticalSection ACOWorker::s_criticalWaitSection;

ACOWorker::ACOWorker(const TArray<AHexagon*>& hexagons, AHexagon* anthillHex, const int& antAmount)
{
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
	if(Thread)
	{
		Thread->Kill();
		delete Thread;
		Thread = nullptr;
	}

	//decrement overall counter
	--s_workerCount;

	UE_LOG(LogACO, Log, TEXT("%s destroyed!"), *m_name);
}

bool ACOWorker::Init()
{
	//init ants?
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
	waitForAllWorkers();
}

void ACOWorker::markPhase()
{
	GLog->Log("do mark work " + m_name);
	waitForAllWorkers();
}

void ACOWorker::evaporatePhase()
{
	GLog->Log("do evaporate work " + m_name );
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
