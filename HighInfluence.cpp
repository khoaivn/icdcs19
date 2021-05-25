#include "HighInfluence.h"
#include "mappedheap.hpp"
#include "HeapData.hpp"

HighInfluence::HighInfluence(SocialGraph * g) : Algorithm(g)
{
}

HighInfluence::~HighInfluence()
{
}

double HighInfluence::getSolution(vector<int>* sol, double * est)
{
	sol->clear();
	vector<int> *listNodes = g->getListNodeIds();
	vector<int> marginalGain(listNodes->size(), 0);
	
	indx.clear();
	mapNodeIdx.clear();

	for (int i = 0; i < listNodes->size(); i++) {
		indx.push_back(i);
		mapNodeIdx[listNodes->at(i)] = i;
	}

	for (int i = 0; i < listNodes->size(); i++) {
		int nodeId = listNodes->at(i);
		vector<pair<int,double>> * neighbors = g->getIncommingNeighbors(nodeId);
		if (neighbors == nullptr || neighbors->size() == 0)
			continue;
		for (int j = 0; j < neighbors->size(); j++) {
			int neiId = neighbors->at(j).first;
			marginalGain[mapNodeIdx[neiId]]++;
		}
	}

	InfCost<int> hd(&marginalGain[0]);
	MappedHeap<InfCost<int>> heap(indx, hd);
	while (sol->size() < Constant::K) {
		unsigned int maxInd = heap.pop();
		sol->push_back(listNodes->at(maxInd));
	}

	*est = estimate(sol, Constant::EPSILON, Constant::DELTA, 10000000);
	clear();
	return 1;
}

double HighInfluence::getDeterministicSolution(vector<int>* sol)
{
	return 0.0;
}
