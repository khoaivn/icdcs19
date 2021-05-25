#pragma once
#include "Algorithm.h"
class SSA : public Algorithm
{
public:
	SSA(SocialGraph *g);
	~SSA();
	double getDeterministicSolution(vector<int> * sol);
	double getSolution(vector<int> * sol, double *est);

private:
	string graphSSAformat;
	string graphBinFile;
	const char * formatCmd;
	string seedFile;
};

