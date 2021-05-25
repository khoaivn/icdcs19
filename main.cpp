#include<iostream>
#include "SocialGraph.h"
#include "DCRgenerator.h"
//#include "ExactSolution.h"
#include "GreedySolution.h"
#include "SandwichSolution.h"
#include "BoundedThres.h"
#include "CompareGreedy.h"
#include "HighInfluence.h"
#include "HighTouch.h"
#include "HighBenefit.h"
#include "SSA.h"
#include <omp.h>
#include <time.h>
#include <fstream>
#include "Constant.h"
#include <stdlib.h>
#include <string>

using namespace std;

#pragma warning(disable : 4996)

using namespace std;

SocialGraph *g;
ofstream writefile;

void printResult(bool isScalable, bool isLargeFile) {
	vector<int> sol;
	sol.clear();
	GreedySolution maf(g);
	long startMaf = time(NULL);
	double remaf = 0;
	if (isScalable)
		maf.getSolution(&sol, &remaf);
	else
		maf.getSolution2Step(&sol, &remaf);
	long timeMaf = time(NULL) - startMaf;

	cout << "MAF: " << remaf << endl;

	SandwichSolution ubg(g);
	long startUbg = time(NULL);
	double reubg = 0;
	double ratio = 0;
	if (isScalable)
		ratio = ubg.getSolution(&sol, &reubg);
	else
		ratio = ubg.getSolution2Step(&sol, &reubg);
	long timeUbg = time(NULL) - startUbg;

	cout << "UBG: " << reubg << endl;

	/*CompareGreedy grd(g);
	long startGrd = time(NULL);
	double reGrd = 0;
	if (isScalable)
	grd.getSolution(&sol, &reGrd);
	else
	grd.getSolution2Step(&sol, &reGrd);
	long timeGrd = time(NULL) - startGrd;*/

	long timeBt = 0;
	double reBt = 0;
	if (Constant::IS_BOUNDED_THRESHOLD && !isLargeFile) {
		BoundedThres bt(g);
		long startBt = time(NULL);
		reBt = 0;
		if (isScalable)
			bt.getSolution(&sol, &reBt);
		else
			bt.getSolution2Step(&sol, &reBt);
		timeBt = time(NULL) - startBt;
	}

	SSA ssa(g);
	double reSSA = 0;
	long startSSA = time(NULL);
	ssa.getSolution(&sol, &reSSA);
	long timeSSA = time(NULL) - startSSA;
	cout << "SSA: " << reSSA << endl;

	HighBenefit hb(g);
	long startHB = time(NULL);
	double reHb = 0;
	hb.getSolution(&sol, &reHb);
	long timeHB = time(NULL) - startHB;

	cout << "HB: " << reHb << endl;

	writefile << Constant::K << " \t " << Constant::COMMUNITY_POPULATION << "\t" << remaf << " " << timeMaf
		<< "\t" << reubg << " " << ratio << " " << timeUbg
		//<< "\t" << reGrd << " " << timeGrd
		<< (Constant::IS_BOUNDED_THRESHOLD && !isLargeFile ? "\t" + to_string(reBt) + " " + to_string(timeBt) : "")
		//<< "\t" << reHi << " " << timeHi
		<< "\t" << reHb << " " << timeHB
		<< "\t" << reSSA << " " << timeSSA << endl;
}

void runExperiment(string input, string inputCommunity, int min, int max, int step,
	bool isScalable, bool isBoundedThres, bool isLargeFile = false, 
	bool changeK = true, bool isDirected = true, bool isWeighted = false, bool isCommMM = true) {
	Constant::IS_BOUNDED_THRESHOLD = isBoundedThres;
	Constant::IS_WEIGHTED = isWeighted;
	Constant::COMMUNITY_POPULATION = 8;
	if (isDirected) {
		if (!isLargeFile)
			g->readSocialGraphFromFile("data/" + input);
		else
			g->readSocialGraphFromLargeFile("data/" + input);
	}
	else {
		g->readSocialGraph("data/" + input);
		g->readCommunityFile("data/" + inputCommunity, isCommMM);
	}
		
	
	string outfilename = "result/" + input + "_result_" 
		+ (isScalable? "scalable_" : "2step_") 
		+ (Constant::IS_BOUNDED_THRESHOLD?"boundedThres" : "freeThres") 
		+ (changeK? "_changeK" : "_changePop") 
		+ (isWeighted? "_weighted" : "_unweighted" ) 
		+ (isCommMM? "_isMM" : "_isClauset") + ".txt";
	writefile.open(outfilename);
	if (writefile.is_open()) {
		if (Constant::IS_BOUNDED_THRESHOLD && !isLargeFile)
			writefile << "k \t Pop \t maf \t ubg-ratio \t grd \t bt \t hb \t ssa" << endl;
		else
			writefile << "k \t Pop \t maf \t ubg-ratio \t grd \t hb \t ssa" << endl;
		
		if (changeK) {	
			if (!isDirected)
				g->formCommunitiesFromActualCommunities();
			for (Constant::K = min; Constant::K <= max; Constant::K += step) {
				printResult(isScalable, isLargeFile);
			}
		}
		else {
			for (Constant::COMMUNITY_POPULATION = min; Constant::COMMUNITY_POPULATION <= max; Constant::COMMUNITY_POPULATION += step) {
				g->formCommunitiesFromActualCommunities();
				printResult(isScalable, isLargeFile);
			}
		}
		
		writefile.close();
	}
}

int main() {
	cout << "Hello I'm running!" << endl;

	g = new SocialGraph();
	omp_set_num_threads(Constant::NUM_THREAD);
	/*runExperiment("data_7115.txt", 5, 50, 5, false, false);
	runExperiment("data_7115.txt", 5, 50, 5, false, true);*/

	/*runExperiment("data_75879.txt", 10, 100, 10, true, false);
	runExperiment("data_75879.txt", 10, 100, 10, true, true);*/

	//runExperiment("soc-pokec-relationships.txt", 1000, 10000, 1000, true, false, true);
	//runExperiment("soc-pokec-relationships.txt", 1000, 10000, 1000, true, true, true);

	/*runExperiment("soc-LiveJournal1.txt", 2000, 10000, 2000, true, false, true);
	runExperiment("soc-LiveJournal1.txt", 2000, 10000, 2000, true, true, true);*/

	/*g->readSocialGraph("data/facebook.txt");
	g->formCommunityClauset("data/facebookCommClauset.txt");
	g->formCommunityModularity("data/facebookCommMM.txt");

	g->readSocialGraph("data/com-dblp.ungraph.txt");
	g->formCommunityClauset("data/dblpCommClauset.txt");
	g->formCommunityModularity("data/dblpCommMM.txt");*/

	/*runExperiment("facebook.txt", "facebookCommMM.txt", 4, 10, 2, true, false, false, false, false, true);
	runExperiment("facebook.txt", "facebookCommGirvan.txt", 4, 10, 2, true, false, false, false, false, true, false);
	runExperiment("facebook.txt", "facebookCommMM.txt", 4, 10, 2, true, true, false, false, false, true, true);
	runExperiment("facebook.txt", "facebookCommGirvan.txt", 4, 10, 2, true, true, false, false, false, true, false);
	runExperiment("com-dblp.ungraph.txt", "dblpCommMM.txt", 4, 10, 2, true, false, false, false, false, true);*/

	g->readSocialGraph("data/Wiki-Vote.txt", true);
	g->formCommunityModularity("data/WikieCommMM.txt", true);

	g->readSocialGraph("data/soc-Epinions1.txt", true);
	g->formCommunityModularity("data/EspinionCommMM.txt", true);

	g->readSocialGraph("data/soc-pokec-relationships.txt", true);
	g->formCommunityModularity("data/PokecCommMM.txt", true);

	//runExperiment("soc-LiveJournal1.txt", "", 2000,10000,2000, true, false, true, true, true, false, false);

	delete g;
	return 0;
}