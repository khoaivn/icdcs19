#include "GreedySolution.h"
#include <omp.h>


GreedySolution::GreedySolution(SocialGraph * g) : Algorithm(g)
{
	int numberOfCommunities = g->getNumberOfCommunities();
	for (int i = 0; i < numberOfCommunities; i++) {
		countAppearance.push_back(0);
		communityIdx.push_back(i);
	}

	vector<int> * listNodes = g->getListNodeIds();
	countNodeAppearance = vector<int>(listNodes->size(), 0);

	isMaf = true;
}

GreedySolution::~GreedySolution()
{
}

void GreedySolution::initiateMarginalGain(vector<int> * pendingComm, vector<double> * marginalGain) {	

	for (int i = 0; i < dcrSet.size(); i++) {
		if (find(pendingComm->begin(), pendingComm->end(), dcrSet[i]->getCommunityId()) != pendingComm->end()) {
			map<int, int> * mapGain = dcrSet[i]->getTrackGain();
			int thres = dcrSet[i]->getThreshold();
			for (map<int, int>::iterator it = mapGain->begin(); it != mapGain->end(); ++it) {
				(*marginalGain)[it->first] += ((double)it->second)/thres;
			}
		}
	}
}

//double GreedySolution::getDeterministicSolution(vector<int>* sol) {
//	sol->clear();
//	initiateGr();
//
//	vector<int> * listNodeIds = g->getListNodeIds();
//
//	vector<vector<int>> currentLive;
//	currentLive.clear();
//	for (int i = 0; i < dcrSet.size(); i++) {
//		DCRgraph * dcr = dcrSet[i];
//		vector<int> * commNodeIds = dcr->getCommunityNodeIds();
//		currentLive.push_back(vector<int>(*commNodeIds));
//		dcr->initiateTrackGain();
//	}
//
//	InfCost<int> hd(&countAppearance[0]);
//	MappedHeap<InfCost<int>> heap(communityIdx, hd);
//	int k = Constant::K;
//	while (sol->size() < Constant::K) {
//		int maxCost = 0;
//		vector<int> pendingComm;
//		while (maxCost < k) {
//			int commId = heap.pop();
//			int cost = g->getCommunityThreshold(commId);
//			pendingComm.push_back(commId);
//			maxCost += cost;
//		}
//		vector<double> gains(listNodeIds->size(), 0);
//		initiateMarginalGain(&pendingComm, &gains);
//		InfCost<double> hd2(&gains[0]);
//		MappedHeap<InfCost<double>> heap2(indx, hd2);
//		while (sol->size() < Constant::K) {
//			int maxIdx = heap2.pop();
//			int nodeId = listNodeIds->at(maxIdx);
//			if (gains[maxIdx] == 0) break;
//			sol->push_back(nodeId);
//
//			#pragma omp parallel for
//			for (int j = 0; j < dcrSet.size(); j++) {
//				map<int, int> reducedGain = dcrSet[j]->updateGainAndCurrentLiveAfterAddingNode(nodeId, &(currentLive[j]));
//				#pragma omp critical
//				{
//					if (find(pendingComm.begin(), pendingComm.end(), dcrSet[j]->getCommunityId()) != pendingComm.end()) {
//						for (map<int, int>::iterator it = reducedGain.begin(); it != reducedGain.end(); ++it) {
//							gains[mapNodeIdx[it->first]] -= (((double)it->second) / dcrSet[j]->getThreshold());
//							heap2.heapify(mapNodeIdx[it->first]);
//						}
//					}
//				}
//			}
//		}
//
//		k = Constant::K - sol->size();
//	}
//
//	return estimateInf(sol);
//}

// update to improve performance
//double GreedySolution::getDeterministicSolution(vector<int>* sol)
//{
//	sol->clear();
//	initiate();
//	int lastKilled = 0;
//	double re = 0.0;
//
//	InfCost<int> hd(&countAppearance[0]);
//	MappedHeap<InfCost<int>> heap(communityIdx, hd);
//
//	vector<int> dcrIdx;
//	for (int i = 0; i < dcrSet.size(); i++) {
//		dcrIdx.push_back(i);
//	}
//
//	vector<int> * nodeIds = g->getListNodeIds();
//	vector<double> marginalGain(nodeIds->size(), 0);
//	vector<vector<int>> currentLive;
//	map<int, int> mapNodeIdx;
//	currentLive.clear();
//	for (int i = 0; i < dcrSet.size(); i++) {
//		DCRgraph * dcr = dcrSet[i];
//		vector<int> * commNodeIds = dcr->getCommunityNodeIds();
//		currentLive.push_back(vector<int>(*commNodeIds));
//		dcr->initiateTrackGain();
//	}
//	
//	#pragma omp parallel for
//	for (int i = 0; i < nodeIds->size(); i++) {
//		int u = (*nodeIds)[i];
//		marginalGain[i] = intialGain[u];
//		mapNodeIdx[u] = i;
//	}
//
//
//	while (!heap.empty()) {
//		int commId = heap.pop();
//		vector<int> commNodes(*(g->getNodesOfCommunity(commId)));
//		int h = Constant::IS_BOUNDED_THRESHOLD? 2: commNodes.size() * Constant::PERCENTAGE_THRESHOLD;
//		h = h > 0 ? h : 1;
//		if (sol->size() + h <= Constant::K) {
//			vector<double> v(commNodes.size(), 0);
//			vector<int> commNodeIdx;
//			map<int, int> mapCommNodeIdx;
//			for (int i = 0; i < commNodes.size(); i++) {
//				v[i] = marginalGain[mapNodeIdx[commNodes[i]]];
//				commNodeIdx.push_back(i);
//				mapCommNodeIdx[commNodes[i]] = i;
//			}
//			InfCost<double> hd2(&v[0]);
//			MappedHeap<InfCost<double>> heap2(commNodeIdx, hd2);
//			for (int i = 0; i < h; i++) {
//				int tmp = heap2.pop();
//				sol->push_back(commNodes[tmp]);
//				#pragma omp parallel for
//				for (int j = 0; j < dcrSet.size(); j++) {
//					map<int, int> reducedGain = dcrSet[j]->updateGainAndCurrentLiveAfterAddingNode(commNodes[tmp], &(currentLive[j]));
//
//					#pragma omp critical
//					{
//						for (map<int, int>::iterator it = reducedGain.begin(); it != reducedGain.end(); ++it) {
//							marginalGain[mapNodeIdx[it->first]] -= (((double)it->second) / dcrSet[j]->getThreshold());
//							if (find(commNodes.begin(), commNodes.end(), it->first) != commNodes.end()) {
//								v[mapCommNodeIdx[it->first]] -= (((double)it->second) / dcrSet[j]->getThreshold());
//								heap2.heapify(mapCommNodeIdx[it->first]);
//							}
//						}
//					}
//
//				}
//			}
//
//			vector<int> killIdx;
//
//			#pragma omp parallel for
//			for (int i = 0; i < dcrIdx.size(); i--) {
//				bool tmp = dcrSet[dcrIdx[i]]->isKill(sol);
//
//				if (tmp) {
//					#pragma omp critical
//					{
//						int commId = dcrSet[dcrIdx[i]]->getCommunityId();
//						countAppearance[commId]--;
//						heap.heapify(communityIdx[commId]);
//						killIdx.push_back(i);
//					}
//				}
//			}
//
//			sort(killIdx.begin(), killIdx.end());
//			for (int i = killIdx.size() - 1; i >= 0; i--) {
//				dcrIdx.erase(dcrIdx.begin() + killIdx[i]);
//			}
//		}
//	}
//	return  estimateInf(sol);
//}

double GreedySolution::getDeterministicSolution(vector<int>* sol)
{
	sol->clear();
	initiateMaf();

	vector<int> sol1;
	sol1.clear();

	InfCost<int> hd(&countAppearance[0]);
	MappedHeap<InfCost<int>> heap(communityIdx, hd);

	while (!heap.empty()) {
		int commId = heap.pop();
		vector<int> commNodes(*(g->getNodesOfCommunity(commId)));
		int h = commNodes.size() * Constant::PERCENTAGE_THRESHOLD;
		h = h > 0 ? h : 1;
		if (sol1.size() + h <= Constant::K) {
			for (int i = 0; i < h; i++) {
				int r = commonInstance->randomInThread() % commNodes.size();
				sol1.push_back(commNodes[r]);
				commNodes.erase(commNodes.begin() + r);
			}
		}
	}

	vector<int> sol2;
	sol2.clear();
	vector<int> * listNodes = g->getListNodeIds();
	InfCost<int> hd2(&countNodeAppearance[0]);
	MappedHeap<InfCost<int>> heap2(indx, hd2);
	while (sol2.size() < Constant::K) {
		unsigned int maxInd = heap2.pop();
		sol2.push_back(listNodes->at(maxInd));
	}

	double tmp1 = estimateInf(&sol1);
	double tmp2 = estimateInf(&sol2);
	double re;

	if (tmp1 > tmp2) {
		*sol = sol1;
		re = tmp1;
	}
	else {
		*sol = sol2;
		re = tmp2;
	}

	return re;
}

void GreedySolution::initiateMaf()
{
	for (int i = trackCount; i < dcrSet.size(); i++) {
		countAppearance[dcrSet[i]->getCommunityId()]++;

		vector<int> * touchNodes = dcrSet[i]->getListTouchedNode();
		for (int j = 0; j < touchNodes->size(); j++) {
			int nodeId = touchNodes->at(j);
			countNodeAppearance[mapNodeIdx[nodeId]]++;
		}
	}

	trackCount = dcrSet.size();
}


/*
#include "GreedySolution.h"
#include <omp.h>


GreedySolution::GreedySolution(SocialGraph * g) : Algorithm(g)
{
	int numberOfCommunities = g->getNumberOfCommunities();
	for (int i = 0; i < numberOfCommunities; i++) {
		countAppearance.push_back(0);
		sortedListCommunities.push_back(i);
	}
}

GreedySolution::~GreedySolution()
{
}

double GreedySolution::getDeterministicSolution(vector<int>* sol)
{
	sol->clear();
	initiate();
	int lastKilled = 0;
	double re = 0.0;
	for (int i = 0; i < sortedListCommunities.size(); i++) {
		int commId = sortedListCommunities[i];
		vector<int> commNodes(*(g->getNodesOfCommunity(commId)));
		int h = commNodes.size() * Constant::PERCENTAGE_THRESHOLD;
		if (sol->size() + h <= Constant::K) {
			for (int i = 0; i < h; i++) {
				int r = rand() % commNodes.size();
				sol->push_back(commNodes[r]);
				commNodes.erase(commNodes.begin() + r);
			}
			re += countAppearance[commId];
		}
	}
	return g->getNumberOfCommunities() * re/dcrSet.size();
}



void GreedySolution::initiate()
{
	for (int i = 0; i < dcrSet.size(); i++) {
		countAppearance[dcrSet[i]->getCommunityId()]++;
	}

	sortListCommunitiesOnAppearance(&sortedListCommunities, 0, sortedListCommunities.size() - 1);
}

void GreedySolution::sortListCommunitiesOnAppearance(vector<int>* list, int start, int end)
{
	if (start < end) {
		int p = partition(list, start, end);
		sortListCommunitiesOnAppearance(list, start, p - 1);
		sortListCommunitiesOnAppearance(list, p + 1, end);
	}
}

void GreedySolution::swap(int i, int j, vector<int>* list)
{
	int tmp = (*list)[i];
	(*list)[i] = (*list)[j];
	(*list)[j] = tmp;
}

int GreedySolution::partition(vector<int>* list, int start, int end)
{
	int pivot = countAppearance[(*list)[end]];
	int i = start - 1;
	for (int j = start; j < end; j++) {
		if (countAppearance[(*list)[j]] > pivot) {
			i++;
			swap(i, j, list);
		}
	}
	if (countAppearance[(*list)[end]] > countAppearance[(*list)[i + 1]])
		swap(i + 1, end, list);
	return i + 1;
}

#define NUM_THREADS 8

vector<int> GreedySolution::getSolution2KillCommunity(int commId, vector<int>* currentSeed)
{
	vector<DCRgraph*> listCommmDCR;
	//omp_set_num_threads(NUM_THREADS);

	#pragma omp parallel for
	for (int i = 0; i < dcrSet.size(); i++) {
		if (dcrSet[i]->getCommunityId() == commId) {
			#pragma omp critical
			{
				listCommmDCR.push_back(dcrSet[i]);
			}
		}
	}

	// store node ids that is current live (unreached) from current solution in each dcr graph
	vector<vector<int>> currentLive;
	int numberOfDcr = listCommmDCR.size();

	for (int i = 0; i < numberOfDcr; i++) {
		currentLive.push_back(vector<int>());
	}

	#pragma omp parallel for
	for (int i =0; i <numberOfDcr; i++) {
		vector<int> tmp = listCommmDCR[i]->getCurrentLive(currentSeed);
		currentLive[i] = tmp;
	}

	for (int i = numberOfDcr - 1; i >= 0; i--) {
		if (currentLive[i].empty()) {
			listCommmDCR.erase(listCommmDCR.begin() + i);
			currentLive.erase(currentLive.begin() + i);
		}
	}

	vector<int> re;
	vector<int> * listNodeIds = g->getListNodeIds();
	while (!listCommmDCR.empty()) {
		int selectedNodeId = -1;
		int maxMarginalGain = -1;

		#pragma omp parallel for
		for (int i = 0; i < listNodeIds->size(); i++) {
			int nodeId = (*listNodeIds)[i];
			int marginalGain = 0;

			if (find(currentSeed->begin(), currentSeed->end(), nodeId) == currentSeed->end()) {
				for (int j = 0; j < currentLive.size(); j++) {
					marginalGain += listCommmDCR[j]->getMarginalGain(nodeId, &currentLive[j]);
				}
				if (marginalGain > maxMarginalGain) {
					#pragma omp critical
					{
						if (marginalGain > maxMarginalGain) {
							selectedNodeId = nodeId;
							maxMarginalGain = marginalGain;
						}
					}
				}
			}
		}

		if (selectedNodeId != -1) {
			re.push_back(selectedNodeId);

			#pragma omp for
			for (int i = currentLive.size() - 1; i >= 0; i--) {
				listCommmDCR[i]->getCurrentLiveAfterAddingNode(selectedNodeId, &currentLive[i]);
			}

			for (int i = currentLive.size() - 1; i >= 0; i--) {
				if (currentLive[i].empty()) {
					listCommmDCR.erase(listCommmDCR.begin() + i);
					currentLive.erase(currentLive.begin() + i);
				}
			}
		}
	}
	return re;
}
*/