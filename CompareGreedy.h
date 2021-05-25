#pragma once
#include "Algorithm.h"

class CompareGreedy : public Algorithm
{
public:
	CompareGreedy(SocialGraph * g);
	~CompareGreedy();

	double getDeterministicSolution(vector<int> * sol);

private:
	vector<vector<int>> currentLive; // store current live node in each dcr graph after each iteration in greedy 
	vector<vector<int>> canKill; // store nodes that can kill dcr graph
};

