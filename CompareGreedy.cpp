#include "CompareGreedy.h"
#include "HeapData.hpp"
#include "mappedheap.hpp"


CompareGreedy::CompareGreedy(SocialGraph * g) : Algorithm(g)
{
}

CompareGreedy::~CompareGreedy()
{
}

double CompareGreedy::getDeterministicSolution(vector<int>* sol)
{
	sol->clear();
	vector<int> * nodeIds = g->getListNodeIds();
	vector<int> marginalGain(nodeIds->size(), 0);

	currentLive.clear();
	canKill.clear();
	for (int i = 0; i < dcrSet.size(); i++) {
		DCRgraph * dcr = dcrSet[i];
		vector<int> * commNodeIds = dcr->getCommunityNodeIds();
		currentLive.push_back(vector<int>(*commNodeIds));
		vector<int> keys(*(dcr->getKeyNodes()));
		canKill.push_back(keys);
	}

	//#pragma omp parallel for
	for (int i = 0; i < nodeIds->size(); i++) {
		int u = (*nodeIds)[i];
		marginalGain[i] = initialDead[u];
	}

	InfCost<int> hd(&marginalGain[0]);
	MappedHeap<InfCost<int>> heap(indx, hd);

	double gain = 0.0;
	while (sol->size() < Constant::K && !heap.empty()) {
		unsigned int maxInd = heap.pop();
		double maxGain = marginalGain[maxInd];
		gain += maxGain;
		sol->push_back((*nodeIds)[maxInd]);
		// update current live
		#pragma omp parallel for
		for (int i = 0; i < dcrSet.size(); i++) {
			map<int, bool> getGain = dcrSet[i]->updateGainAndCurrentLiveAfterAddingNodeCG((*nodeIds)[maxInd], &(currentLive[i]), &(canKill[i]));

			if (!getGain.empty()) {
				#pragma omp critical
				{
					for (map<int, bool>::iterator it = getGain.begin(); it != getGain.end(); ++it) {
						if (it->second)
							++marginalGain[mapNodeIdx[it->first]];
						else --marginalGain[mapNodeIdx[it->first]];
						heap.heapify(mapNodeIdx[it->first]);
					}
				}
			}
		}
		
	}

	return gain * (Constant::IS_WEIGHTED ? g->getNumberOfNodes() : g->getNumberOfCommunities()) / dcrSet.size();
}

