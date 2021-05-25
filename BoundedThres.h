#pragma once
#include "Algorithm.h"

class BoundedThres : public Algorithm
{
public:
	BoundedThres(SocialGraph * g);
	~BoundedThres();

	double getDeterministicSolution(vector<int> * sol);

private:

	void initiateMapTouch();

	int greedy(vector<DCRgraph*>* set, int exclude, vector<int>* sol);

	map<int, vector<DCRgraph *>> mapTouchGraphs;
	int oldTmp = 0; // old size of dcrSet, used to icrementally update mapTouchGraphs
};

