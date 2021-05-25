#pragma once
#include "Algorithm.h"
#include "HeapData.hpp"
#include "mappedheap.hpp"

class GreedySolution : public Algorithm 
{
public:
	GreedySolution(SocialGraph * g);
	~GreedySolution();

	double getDeterministicSolution(vector<int> * sol);

private:
	void initiateMaf();
	//vector<int> sortedListCommunities;
	vector<int> communityIdx;
	vector<int> countAppearance;
	vector<int> countNodeAppearance;
	int trackCount = 0;
	void initiateMarginalGain(vector<int>* pendingComm, vector<double>* marginalGain);
	/*
	void sortListCommunitiesOnAppearance(vector<int> * list, int start, int end);
	void swap(int i, int j, vector<int> * list);
	int partition(vector<int> * list, int start, int end);
	vector<int> getSolution2KillCommunity(int commId, vector<int> * currentSeed);
	*/
};

