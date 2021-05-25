#pragma once
#include<vector>
#include<map>
#include "Common.h"
using namespace std;

class SocialGraph
{
public:
	SocialGraph();
	~SocialGraph();

	void readSocialGraphFromFile(string file); // old format one (communities are generated randomly)
	void readSocialGraph(string file, bool isDirected = false);
	void readSocialGraphFromLargeFile(string inputFile);
	void generateFile(string inputFile); // used only to generate graph file from regular downloaded txt file
	void generateFileIM(string outputFile); // used to generate file in SSA format - used to test IM performance 
	void standardize(string file, bool header = true);
	void formCommunityModularity(string output, bool directed = false);
	void formCommunityClauset(string output);

	void readCommunityFile(string file, bool isMM = true);
	void formCommunitiesFromActualCommunities();

	int randomSelectCommunity();
	vector<int>* getNodesOfCommunity(int commId);	
	vector<std::pair<int, double> > * getIncommingNeighbors(int nodeId);
	map<int, vector<std::pair<int, double> > > * getMapIncommingNeighbors();
	vector<int> * getListNodeIds();
	int getMaxThreshold();
	int getNumberOfNodes();
	int getNumberOfCommunities();
	int getCommunityThreshold(int commId);
	int getCommunityId(int nodeId);
	int getCommunitySize(int commId);
	int getMinBenefit();

private:
	vector<int> listNodeIds;
	bool isDirected;
	map<int, int> mapNodeId2CommId;
	vector<vector<int> > listCommListNodeIds; // comm id -> list of node ids in community, dont need map because comm id in txt file in order from 0
	vector<vector<int>> actualCommNodeIds; // actual communities from community detection alg 
	map<int, vector<std::pair<int, double> > > mapIncommingNeighbors; // id -> list of incomming neighbor id and weight of edge 
	int hMax = 0; // maximum threshold of one community
	int bMin = 1000; // size of the smallest community
	void clear();
	Common * commonInstance;
	long noOfEdges = 0;

	void addCommunity(vector<int> * commNodes);
};

