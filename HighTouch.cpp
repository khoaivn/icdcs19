#include "HighTouch.h"
#include "mappedheap.hpp"
#include "HeapData.hpp"


HighTouch::HighTouch(SocialGraph * g) : Algorithm(g)
{
}

HighTouch::~HighTouch()
{
}

double HighTouch::getDeterministicSolution(vector<int>* sol)
{
	sol->clear();
	
	indx.clear();
	mapNodeIdx.clear();

	vector<int> * listNodes = g->getListNodeIds();

	for (int i = 0; i < listNodes->size(); i++) {
		indx.push_back(i);
		mapNodeIdx[listNodes->at(i)] = i;
	}

	vector<int> marginalGain(listNodes->size(), 0);

	for (int i = 0; i < dcrSet.size(); i++) {
		vector<int> * touchNodes = dcrSet[i]->getListTouchedNode();
		for (int j = 0; j < touchNodes->size(); j++) {
			int nodeId = touchNodes->at(j);
			marginalGain[mapNodeIdx[nodeId]]++;
		}
	}

	InfCost<int> hd(&marginalGain[0]);
	MappedHeap<InfCost<int>> heap(indx, hd);
	while (sol->size() < Constant::K) {
		unsigned int maxInd = heap.pop();
		sol->push_back(listNodes->at(maxInd));
	}

	return estimateInf(sol);
}
