#include "HighBenefit.h"
#include "mappedheap.hpp"
#include "HeapData.hpp"


HighBenefit::HighBenefit(SocialGraph * g) : Algorithm(g)
{
	vector<int> * listNodes = g->getListNodeIds();
	benefits = vector<double>(listNodes->size(), 0);
}

HighBenefit::~HighBenefit()
{
}

double HighBenefit::getSolution(vector<int>* sol, double * est)
{
	initiate();
	sol->clear();
	initiateHBC();
	InfCost<double> hd(&benefits[0]);
	MappedHeap<InfCost<double>> heap(indx, hd);
	vector<int> * listNodes = g->getListNodeIds();
	for (int i = 0; i < Constant::K; i++) {
		int idx = heap.pop();
		sol->push_back(listNodes->at(idx));
	}
	*est = estimate(sol, Constant::EPSILON, Constant::DELTA, 100000000);
	clear();
	return 1;
}

double HighBenefit::getDeterministicSolution(vector<int>* sol)
{
	return 0.0;
}

void HighBenefit::initiateHBC()
{
	vector<int> * listNodes = g->getListNodeIds();
	for (int i = 0; i < listNodes->size(); i++) {
		int nodeId = listNodes->at(i);
		double benefit = Constant::IS_WEIGHTED ? g->getCommunitySize(g->getCommunityId(nodeId)) : 1;
		vector<std::pair<int, double>> * neighbors = g->getIncommingNeighbors(nodeId);
		if (neighbors != nullptr && neighbors->size() > 0) {
			double w = 1.0 / neighbors->size();
			int commId = g->getCommunityId(nodeId);
			int thres = g->getCommunityThreshold(commId);
			for (int j = 0; j < neighbors->size(); j++) {
				int nId = neighbors->at(j).first;
				int nIndex = mapNodeIdx[nId];
				benefits[nIndex] += (w * benefit / thres);
			}
		}
	}
}
