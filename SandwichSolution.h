#pragma once
#include "Algorithm.h"

class SandwichSolution : public Algorithm
{
public:
	SandwichSolution(SocialGraph * g);
	~SandwichSolution();

	double getDeterministicSolution(vector<int> * sol);
	//double estimate(vector<int> * sol, double epsilon2, double delta, int tMax);
	double getSolution(vector<int> * sol, double * est);
	double getSolution2Step(vector<int> * sol, double * est);
	double estimate(vector<int>* sol, double epsilon, double delta, int tMax);
private:
	double getMarginalGain(int nodeId, vector<int> * sol);
	vector<vector<int>> currentLive; // store current live node in each dcr graph after each iteration in greedy 
};

