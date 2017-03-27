// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * 
 */
struct Ant
{
	class AHexagon* Position;
	TArray<class AHexagon*> visitedPath;
	bool isCarryingFood;
};

class ACO_API ACOWorker : public FRunnable
{
public:
	ACOWorker(const TArray<class AHexagon*>& hexagons, class AHexagon* anthillHex, const int& antAmount);
	~ACOWorker();

	//Begin FRunnable Methods
	bool Init() override;
	uint32 Run() override;
	void Stop() override;
	//End

	void Pause();
	void Unpause();
protected:
	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;
	/** Stop this thread? Uses Thread Safe Counter */
	FThreadSafeCounter StopTaskCounter;


	//Begin ACO
	void traversePhase();
	void markPhase();
	void evaporatePhase();

	static int s_workerCount;
	FString m_name;
};
