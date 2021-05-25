#include "BoundedThres.h"
#include "mappedheap.hpp"
#include "HeapData.hpp"


BoundedThres::BoundedThres(SocialGraph * g) : Algorithm(g)
{
}

BoundedThres::~BoundedThres()
{
}

double BoundedThres::getDeterministicSolution(vector<int>* sol)
{
	initiateMapTouch();

	vector<int> * listNodeIds = g->getListNodeIds();
	
	sol->clear();
	vector<int> idx(listNodeIds->size(), 0);
	vector<int> gain(listNodeIds->size(), 0);
	vector<vector<int>> solNode(listNodeIds->size(), vector<int>());
	int maxGain = 0;

	#pragma omp parallel for
	for (int i = 0; i < listNodeIds->size(); i++) {
		vector<int> solTmp;
		int nodeId = listNodeIds->at(i);
		if (mapTouchGraphs.find(nodeId) != mapTouchGraphs.end()) {
			idx[i] = i;
			gain[i] = greedy(&(mapTouchGraphs[nodeId]), nodeId, &solTmp);
			solNode[i] = solTmp;
		}
		
		/*if (gain[i] > maxGain) {
			#pragma omp critical
			{
				if (gain[i] > maxGain) {
					maxGain = gain[i];
					*sol = solTmp;
				}
			}
		}*/
	}

	InfCost<int> hd(&gain[0]);
	MappedHeap<InfCost<int>> heap(idx, hd);

	map<int, bool> isInSol;

	while (sol->size() < Constant::K && !heap.empty()) {
		int maxIdx = heap.pop();
		vector<int> addTmp = solNode[maxIdx];
		for (int i = 0; i < addTmp.size(); i++) {
			if (isInSol.find(addTmp[i]) == isInSol.end()) {
				sol->push_back(addTmp[i]);
				isInSol[addTmp[i]] = true;
			}
		}
	}

	return estimateInf(sol);
}

void BoundedThres::initiateMapTouch()
{
	for (int i = oldTmp; i < dcrSet.size(); ++i) {
		vector<int> * list = dcrSet[i]->getListTouchedNode();
		for (int j = 0; j < list->size(); ++j) {
			mapTouchGraphs[list->at(j)].push_back(dcrSet[i]);
		}
		delete list;
	}
	oldTmp = dcrSet.size();
}

int BoundedThres::greedy(vector<DCRgraph *> * set, int exclude, vector<int> * sol) {
	if (set->size() == 0) return 0;

	sol->push_back(exclude);
	
	vector<vector<int>> listBeingTouch(set->size(), vector<int>()); // index of dcr graph -> list of node that touches it 

	vector<int> * listNodeIds = g->getListNodeIds();
	vector<int> indx(listNodeIds->size(), 0);
	vector<int> degree(listNodeIds->size(), 0); // node id -> number of dcr graph that it touches
	map<int, int> mapNodeIds;

	vector<vector<int>> listTouch(listNodeIds->size(), vector<int>()); // index of node -> list of dcr graph index that it touch
	
	for (int i = 0; i < listNodeIds->size(); i++) {
		indx[i] = i;
		mapNodeIds[listNodeIds->at(i)] = i;
	}

	// set up degree for each node (how many dcr graphs each node touch) except the exclude one
	for (int i = 0; i < set->size(); i++) {
		vector<int> * list = set->at(i)->getListTouchedNode(exclude);
		for (int j = 0; j < list->size(); ++j) {
			int indexTmp = mapNodeIds[(*list)[j]];
			degree[indexTmp]++;
			listTouch[indexTmp].push_back(i);
		}
		listBeingTouch[i] = *list;
		delete list;
	}

	InfCost<int> hd(&degree[0]);
	MappedHeap<InfCost<int>> heap(indx, hd);

	int select = 0;
	int gain = 0;
	while (select < Constant::K && !heap.empty()) {
		//int maxIdx = select == 0 ? mapNodeIds[exclude] : heap.pop();
		int maxIdx = heap.pop();
		int nodeId = (*listNodeIds)[maxIdx];
		select++;
		sol->push_back(nodeId);
		vector<int> touch = listTouch[maxIdx]; // list of dcr index that this node touch
		gain += touch.size();
		if (touch.empty()) break;
		for (int i = 0; i < touch.size(); i++) {
			vector<int> beingTouched = listBeingTouch[touch[i]]; // list of node that touch this dcr graph
			for (int j = 0; j < beingTouched.size(); j++) {
				int indexTmp = mapNodeIds[beingTouched[j]];
				degree[indexTmp]--;
				auto it = find(listTouch[indexTmp].begin(), listTouch[indexTmp].end(), touch[i]);
				if (it != listTouch[indexTmp].end())
					listTouch[indexTmp].erase(it);
				heap.heapify(indexTmp);
			}
		}
	}

	return gain;
}