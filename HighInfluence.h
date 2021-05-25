#pragma once
#include "Algorithm.h"
class HighInfluence : public Algorithm
{
public:
	HighInfluence(SocialGraph *g);
	~HighInfluence();
	double getSolution(vector<int> * sol, double *est);
	double getDeterministicSolution(vector<int> * sol);
};

