#include "SandwichSolution.h"
#include <omp.h>
#include <iostream>
#include <time.h>
#include "mappedheap.hpp"
#include "HeapData.hpp"


SandwichSolution::SandwichSolution(SocialGraph * g) : Algorithm(g)
{
	
}

SandwichSolution::~SandwichSolution()
{
}

/*Test for speed up greedy*/
double SandwichSolution::getDeterministicSolution(vector<int>* sol)
{
	sol->clear();
	vector<int> * nodeIds = g->getListNodeIds();
	vector<double> marginalGain(nodeIds->size(), 0);

	currentLive.clear();
	for (int i = 0; i < dcrSet.size(); i++) {
		DCRgraph * dcr = dcrSet[i];
		vector<int> * commNodeIds = dcr->getCommunityNodeIds();
		currentLive.push_back(vector<int>(*commNodeIds));
		dcr->initiateTrackGain();
	}

	//#pragma omp parallel for
	for (int i = 0; i < nodeIds->size(); i++) {
		int u = (*nodeIds)[i];
		marginalGain[i] = intialGain[u];
	}

	InfCost<double> hd(&marginalGain[0]);
	MappedHeap<InfCost<double>> heap(indx, hd);

	double gain = 0.0;
	while (sol->size() < Constant::K) {
		unsigned int maxInd = heap.pop();
		double maxGain = marginalGain[maxInd];
		gain += maxGain;
		if (maxGain > 0) {
			sol->push_back((*nodeIds)[maxInd]);
			// update current live
			#pragma omp parallel for
			for (int i = 0; i < dcrSet.size(); i++) {
				map<int, int> reducedGain = dcrSet[i]->updateGainAndCurrentLiveAfterAddingNode((*nodeIds)[maxInd], &(currentLive[i]));

				#pragma omp critical
				{
					for (map<int, int>::iterator it = reducedGain.begin(); it != reducedGain.end(); ++it) {
						marginalGain[mapNodeIdx[it->first]] -= (((double)it->second) / dcrSet[i]->getThreshold());
						heap.heapify(mapNodeIdx[it->first]);
					}
				}
				
			}
		}
		else break;
	}

	return gain * (Constant::IS_WEIGHTED ? g->getNumberOfNodes() : g->getNumberOfCommunities()) / dcrSet.size();
}
/*official running*/
/*
double SandwichSolution::getDeterministicSolution(vector<int>* sol)
{
	sol->clear();
	vector<int> nodeIds(*(g->getListNodeIds()));
	currentLive.clear();
	for (int i = 0; i < dcrSet.size(); i++) {
		DCRgraph * dcr = dcrSet[i];
		vector<int> * commNodeIds = dcr->getCommunityNodeIds();
		currentLive.push_back(vector<int>(*commNodeIds));
	}
		

	double gain = 0;
	while (sol->size() < Constant::K) {
		int maxIndex = 0;
		double maxGain = 0;

		#pragma omp parallel for
		for (int i = 0; i < nodeIds.size(); i++) {
			int u = nodeIds[i];
			double marginalGain = getMarginalGain(u, sol);
			
			#pragma omp critical
			{
				if (marginalGain > maxGain) {
					maxIndex = i;
					maxGain = marginalGain;
				}
			}
		}

		if (maxGain > 0){
			sol->push_back(nodeIds[maxIndex]);
			gain += maxGain;
						
			// update current live
			#pragma omp parallel for
			for (int i = 0; i < dcrSet.size(); i++) {
				dcrSet[i]->getCurrentLiveAfterAddingNode(nodeIds[maxIndex], &(currentLive[i]));
			}
			nodeIds.erase(nodeIds.begin() + maxIndex);
		}
		else break;
	}
	return gain * g->getNumberOfCommunities() / dcrSet.size();
}
*/

/*
double SandwichSolution::estimateInf(vector<int>* sol, int noDcr) {
	double re = 0.0;

	#pragma omp parallel for
	for (int i = 0; i < noDcr; i++) {
		DCRgraph * g = dcrSet[i];
		double fr = g->fractionalInf(sol);
		#pragma omp critical
		{
			re += 1.0;
		}
	}

	return re * g->getNumberOfCommunities() / dcrSet.size();
}
*/

double SandwichSolution::getSolution(vector<int>* sol, double *est)
{
	sol->clear();
	initiate();
	omp_set_num_threads(Constant::NUM_THREAD);
	generateDCRgraphs((int)D);

	while (dcrSet.size() < rMax) {
		double re = getDeterministicSolution(sol);
		int tmp = dcrSet.size();
		*est = estimateInf(sol);
		if (dcrSet.size() * (*est) / (Constant::IS_WEIGHTED ? g->getNumberOfNodes() : g->getNumberOfCommunities()) >= D) {
			double re2 = estimate(sol, e2, Constant::DELTA / 3, dcrSet.size());
			cout << re << " " << re2 << " " << time(NULL) << endl;
			if (re <= (1 + e1)*re2)
				return (*est)/re;
		}

		int tmp2 = dcrSet.size();
		generateDCRgraphs(2 * tmp - tmp2);

	}
	double re = getDeterministicSolution(sol);
	*est = estimateInf(sol);
	clear();
	return (*est)/re;
}

double SandwichSolution::getSolution2Step(vector<int>* sol, double * est)
{
	sol->clear();
	initiate();
	omp_set_num_threads(Constant::NUM_THREAD);
	generateDCRgraphs((int)rMax);
	double re = getDeterministicSolution(sol);
	*est = estimateInf(sol);
	clear();
	return (*est) / re;
}


double SandwichSolution::estimate(vector<int>* sol, double epsilon, double delta, int tMax)
{
	double lamda = 0.72;
	double tmp = 4 * lamda * log(2 / delta) / (epsilon*epsilon);
	double lambda = 1 + (1 + epsilon) * tmp;

	int T = 0;
	double inf = 0.0;

	#pragma omp parallel for
	for (int i = 0; i < tMax; i++) {
		DCRgraph * g = gen.generateDCRgraph();
		double fr = g->fractionalInf(sol);

		#pragma omp critical
		{
			dcrSet.push_back(g);
			g->updateInitalGain(&intialGain, &initialDead);
			if (tMax > 0) {
				T++;
				inf += fr;
				if (inf >= lambda) {
					tMax = -1;
				}
			}
		}
	}

	return (tMax == -1 ? lambda * (Constant::IS_WEIGHTED ? g->getNumberOfNodes() : g->getNumberOfCommunities()) / T : -1);
}



double SandwichSolution::getMarginalGain(int nodeId, vector<int>* sol)
{
	double re = 0.0;
	for (int i = 0; i < dcrSet.size(); i++) {
		DCRgraph * dcr = dcrSet[i];
		int gain = dcr->getMarginalGain(nodeId, &(currentLive[i]));
		re += ((double)gain) / dcr->getThreshold();
	}
	return re;
}

