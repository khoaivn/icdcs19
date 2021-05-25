#include "SSA.h"
#include <iostream>
#include <fstream>
#include <string>

SSA::SSA(SocialGraph * g) : Algorithm(g)
{
	graphSSAformat = "graphSSA.txt";
	g->generateFileIM(graphSSAformat);
	graphBinFile = "graphSSA.bin";

	string tmp = " ../SSA_release_2.0/DSSA/el2bin " + graphSSAformat + " " + graphBinFile;
	formatCmd = tmp.c_str();
	system(formatCmd);

	seedFile = "ssa.seeds";
}

SSA::~SSA()
{
}

double SSA::getDeterministicSolution(vector<int>* sol)
{
	return 0.0;
}

double SSA::getSolution(vector<int>* sol, double * est)
{
	sol->clear();
	initiate();
	
	string tmp2 = "../SSA_release_2.0/DSSA/DSSA -i " + graphBinFile + " -o "
		+ seedFile + " -k " + to_string(Constant::K) + " -epsilon "
		+ to_string(Constant::EPSILON) + " -delta " + to_string(Constant::DELTA);
	const char * runSSAcmd = tmp2.c_str();
	system(runSSAcmd);

	ifstream inputFile;
	inputFile.open(seedFile);
	if (inputFile) {
		vector<int> * listNodes = g->getListNodeIds();
		int nodeIdx;
		while (inputFile >> nodeIdx) {
			int nodeId = listNodes->at(nodeIdx - 1);
			sol->push_back(nodeId);
		}
		inputFile.close();
	}

	*est = estimate(sol, Constant::EPSILON, Constant::DELTA, 100000000);
	clear();
	return 1;
}
