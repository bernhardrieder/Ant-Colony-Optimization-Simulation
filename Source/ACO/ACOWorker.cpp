// Fill out your copyright notice in the Description page of Project Settings.

#include "ACO.h"
#include "ACOWorker.h"
#include "Hexagon.h"

int ACOWorker::s_workerCount = 0;

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
	UE_LOG(LogACO, Log, TEXT("%s destroyed!"), *m_name);
	if(Thread)
	{
		Thread->Kill();
		delete Thread;
		Thread = nullptr;
	}
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

void ACOWorker::Pause()
{
	Thread->Suspend(true);
}

void ACOWorker::Unpause()
{
	Thread->Suspend(false);
}

void ACOWorker::traversePhase()
{
}

void ACOWorker::markPhase()
{
}

void ACOWorker::evaporatePhase()
{
	//reset maxPheromoneLevel
	//update pheromone Visualization
	//update max Pheromone on Map
}
