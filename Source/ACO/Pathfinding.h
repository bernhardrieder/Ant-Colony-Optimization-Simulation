// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <unordered_map>
#include "Hexagon.h"

/**
 * 
 */
static class ACO_API Pathfinding
{
public:
	static void AStarSearch(AHexagon* start, AHexagon* goal, std::unordered_map<AHexagon*, AHexagon*>& came_from);
	static std::vector<AHexagon*> ReconstructPath(AHexagon* start, AHexagon* goal, std::unordered_map<AHexagon*, AHexagon*> came_from, bool shouldBeSortedStartToEnd = false);
	static float AStarSearchHeuristic(AHexagon* start, AHexagon* goal);
};