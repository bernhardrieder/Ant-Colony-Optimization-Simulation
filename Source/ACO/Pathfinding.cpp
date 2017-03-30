// Fill out your copyright notice in the Description page of Project Settings.

#include "ACO.h"
#include "Pathfinding.h"
#include <queue>
#include <functional>
#include <string>

void Pathfinding::AStarSearch(AHexagon* start, AHexagon* goal, std::unordered_map<AHexagon*, AHexagon*>& came_from)
{
	std::unordered_map<AHexagon*, float> cost_so_far;
	std::priority_queue<std::pair<float, AHexagon*>, std::vector<std::pair<float, AHexagon*>>, std::greater<std::pair<float, AHexagon*>>> open;
	open.emplace(0, start);

	came_from[start] = start; // = closed list!
	cost_so_far[start] = 0;

	while (!open.empty())
	{
		AHexagon* currentItemWithBestCost = open.top().second;
		open.pop();

		if (currentItemWithBestCost == goal)
			break;

		for (auto next : currentItemWithBestCost->GetNeighbourHexagons())
		{
			float new_cost = cost_so_far[currentItemWithBestCost] + next->GetPheromoneAStarCost();
			if (!cost_so_far.count(next) || new_cost < cost_so_far[next])
			{
				cost_so_far[next] = new_cost;
				double overallCost = new_cost + AStarSearchHeuristic(start, goal);
				open.emplace(overallCost, next);
				came_from[next] = currentItemWithBestCost;
			}
		}
	}
}

std::vector<AHexagon*> Pathfinding::ReconstructPath(AHexagon* start, AHexagon* goal, std::unordered_map<AHexagon*, AHexagon*> came_from, bool shouldBeSortedStartToEnd)
{
	std::vector<AHexagon*> path;
	AHexagon* current = goal;
	path.push_back(current);
	while (current != start)
	{
		current = came_from[current];
		path.push_back(current);
	}
	if (shouldBeSortedStartToEnd)
		std::reverse(path.begin(), path.end());
	return path;
}

float Pathfinding::AStarSearchHeuristic(AHexagon* start, AHexagon* goal)
{
	float manhattan = FVector::Dist(start->GetActorLocation(), goal->GetActorLocation()) / 10;
	return manhattan;
}
