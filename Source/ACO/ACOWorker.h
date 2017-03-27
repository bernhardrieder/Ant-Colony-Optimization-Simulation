// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
/**
 * 
 */
struct Ant
{
	explicit Ant(class AHexagon* pos): Position(pos), isCarryingFood(false)	{}

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

	void Pause() const;
	void Unpause() const;

protected:
	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;
	/** Stop this thread? Uses Thread Safe Counter */
	FThreadSafeCounter StopTaskCounter;	
	
	// thread safe variables
	FString m_name;
	static int s_workerCount;
	/** Events for thread synchronization - wait & notfiy */
	static TArray<FScopedEvent*> s_waitEvents;
	static FCriticalSection s_criticalWaitSection;

	//ACO variables
	std::vector<Ant*> m_ants;
	TArray<AHexagon*> m_hexagons;
	//ACO constants
	static float s_traversePhaseConstantA;
	static float s_traversePhaseConstantB;
	
	//Begin ACO
	void traversePhase();
	void markPhase();
	void evaporatePhase();

	/** wait for completion of other threads*/
	static void waitForAllWorkers();




};