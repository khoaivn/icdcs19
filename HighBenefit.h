#pragma once
#include "Algorithm.h"
class HighBenefit : public Algorithm
{
public:
	HighBenefit(SocialGraph * g);
	~HighBenefit();
	double getSolution(vector<int> * sol, double *est);
	double getDeterministicSolution(vector<int> * sol);
private:
	void initiateHBC();
	vector<double> benefits;
};

