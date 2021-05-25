#include "Algorithm.h"
#include <math.h>
#include "Common.h"
#include "Constant.h"
#include <omp.h>
#include <iostream>
#include <time.h>

Algorithm::Algorithm(SocialGraph * g)
{
	this->g = g;
	gen.setSocialGraph(g);
	commonInstance = Common::getInstance();
}

Algorithm::Algorithm()
{
}

Algorithm::~Algorithm()
{
	for (int i = 0; i < dcrSet.size(); i++) {
		delete dcrSet[i];
	}
}

double Algorithm::estimate(vector<int> * sol, double epsilon, double delta, int tMax)
{
	double lamda = 0.72;
	double tmp = 4 * lamda * log(2 / delta) / (epsilon*epsilon);
	double lambda = 1 + (1 + epsilon) * tmp;
	
	int T = 0, inf = 0;

	#pragma omp parallel for
	for (int i = 0; i < tMax; i++) {
		DCRgraph * g = gen.generateDCRgraph();
		bool kill = g->isKill(sol);
		
		#pragma omp critical
		{
			dcrSet.push_back(g);
			g->updateInitalGain(&intialGain, &initialDead);
			if (tMax > 0) {
				T++;
				if (kill) inf++;
				if (inf >= lambda) {
					tMax = -1;
				}
			}
		}
	}

	/*
	while (inf < lambda && T < tMax) {
		T++;
		DCRgraph * g = gen.generateDCRgraph();
		if (g->isKill(sol)) {
			inf++;
		}
	}*/
	
	return (tMax == -1? (Constant::IS_WEIGHTED ? g->getNumberOfNodes() : g->getNumberOfCommunities()) * lambda / T : -1);
}

double Algorithm::getSolution(vector<int>* sol, double *est)
{
	sol->clear();
	initiate();
	omp_set_num_threads(Constant::NUM_THREAD);

	generateDCRgraphs((int)D);

	while (dcrSet.size() < rMax) {
		
		*est = getDeterministicSolution(sol);
		int tmp = dcrSet.size();
		if (dcrSet.size() * (*est) / (Constant::IS_WEIGHTED ? g->getNumberOfNodes() : g->getNumberOfCommunities()) >= D) {
			double re2 = estimate(sol, e2, Constant::DELTA / 3, dcrSet.size());
			cout << *est << " " << re2 << " " << time(NULL) << endl;
			if (*est <= (1 + e1)*re2)
				return 1;
		}
		
		int tmp2 = dcrSet.size();
		generateDCRgraphs(2 * tmp - tmp2);
		
	}
	*est = getDeterministicSolution(sol);
	clear();
	return 1;
}

double Algorithm::getSolution2Step(vector<int>* sol, double * est)
{
	sol->clear();
	initiate();
	omp_set_num_threads(Constant::NUM_THREAD);
	generateDCRgraphs((int)rMax);
	*est = getDeterministicSolution(sol);
	clear();
	return 1;
}

double Algorithm::estimateInf(vector<int>* sol)
{
	double re = 0.0;

	#pragma omp parallel for
	for (int i = 0; i < dcrSet.size(); i++) {
		bool kill = dcrSet[i]->isKill(sol);

		if (kill) {
			#pragma omp critical
			{
				re += 1.0;
			}
		}
	}

	return re * (Constant::IS_WEIGHTED?g->getNumberOfNodes() : g->getNumberOfCommunities()) / dcrSet.size();
}

void Algorithm::initiate()
{
	double ep1 = Constant::EPSILON - 0.081;
	double ep2 = Constant::EPSILON - ep1;
	int nChoosek = Common::getInstance()->nChoosek(g->getNumberOfNodes(), Constant::K);
	/*double d1 = (double)nChoosek / (nChoosek + 1) * Constant::DELTA;
	double d2 = Constant::DELTA - d1;*/
	double d1 = Constant::DELTA - 0.01;
	double d2 = Constant::DELTA - d1;
	int hMax = g->getMaxThreshold();
	
	double tmp1 = log((double)nChoosek / d1) / (ep1*ep1);
	double tmp2 = 2 * log(1.0 / d2) / (ep2*ep2);
	
	if (!Constant::IS_WEIGHTED) {
		int t = g->getNumberOfCommunities();
		rMax = ceil(hMax*t / Constant::K * max(3 * log((double)nChoosek / d1) / (ep1*ep1), 2 * log(1.0 / d2) / (ep2*ep2))); // maximum number of DCR graph
	}
	else {
		int t = g->getNumberOfNodes();
		int bMin = g->getMinBenefit();
		rMax = ceil(hMax*t / (Constant::K * bMin) * max(3 * log((double)nChoosek / d1) / (ep1*ep1), 2 * log(1.0 / d2) / (ep2*ep2))); // maximum number of DCR graph
	}
	
	
	e1 = Constant::EPSILON / 4;
	e2 = Constant::EPSILON / 4;
	e3 = Constant::EPSILON / 4;
	D = (1 + e1) * (1 + e2) * 2 / (e3*e3) * log(3 / Constant::DELTA);
	
	vector<int> * nodeIds = g->getListNodeIds();
	indx = vector<int>(nodeIds->size(), 0);
	for (int i = 0; i < nodeIds->size(); i++) {
		int u = (*nodeIds)[i];
		indx[i] = i;
		mapNodeIdx.insert(pair<int, int>(u, i));
	}
}

void Algorithm::generateDCRgraphs(int number)
{
	#pragma omp parallel for
	for (int i = 0; i < number; i++) {
		DCRgraph * dcr = gen.generateDCRgraph();
		#pragma omp critical
		{
			dcrSet.push_back(dcr);
			if (!isMaf)
				dcr->updateInitalGain(&intialGain, &initialDead);
			else 
				dcr->updateInitalGain(&intialGain, &initialDead, &initialOtherCommunityGain);
		}
		//cout << i << endl;
	}
	//cout << "done generating samples" << endl;
}

void Algorithm::clear()
{
	for (int i = 0; i < dcrSet.size(); i++)
		delete dcrSet[i];
	dcrSet.clear();
	intialGain.clear();
}



