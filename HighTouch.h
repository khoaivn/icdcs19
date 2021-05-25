#pragma once
#include "Algorithm.h"
class HighTouch : public Algorithm 
{
public:
	HighTouch(SocialGraph *g);
	~HighTouch();
	double getDeterministicSolution(vector<int> * sol);
};

