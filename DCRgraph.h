#pragma once
#include<vector>
#include<map>

using namespace std;

class DCRgraph
{
public:
	DCRgraph(int communityId, int threshold, vector<int>* communityNodeIds);
	~DCRgraph();

	void addReachable(int nodeId, vector<int> * reachNodeIds);
	map<int, vector<int> >* getMapReachable();
	int getPopulation();
	vector<int> * getCommunityNodeIds();
	vector<int> * getReachableIds(int nodeId);
	int getThreshold();
	int getCommunityId();
	vector<int> getCurrentLive(vector<int> * sol); // get current unreached node id from current solution
	int getMarginalGain(int nodeId, vector<int> * currentLive);

	void getCurrentLiveAfterAddingNode(int nodeId, vector<int> * currentLive);
	
	// return map <nodeId, reduced in its marginal gain>, used for sandwich solution
	map<int, int> updateGainAndCurrentLiveAfterAddingNode(int nodeId, vector<int> * currentLive);
	// return map <nodeId, reduced in its marginal gain>, used for compare greedy solution
	map<int,bool> updateGainAndCurrentLiveAfterAddingNodeCG(int nodeId, vector<int> * currentLive, vector<int> * canKill);
	
	
	void updateInitalGain(map<int, double> * mapInf, map<int, int> * mapDead = nullptr, map<int, double> * mapInfMaf = nullptr); // used to update intial Gain when new dcrGraph generated
	bool isKill(vector<int> * sol); // return true if sol can reach > threshold number of node in community
	double fractionalInf(vector<int> * sol); // return fractional value of being influenced, using in sandwich solution

	bool isTouchedByNode(int nodeId);
	vector<int> * getListTouchedNode();
	vector<int> * getListTouchedNode(int excludeId);
	void initiateTrackGain();
	map<int, int> * getTrackGain();

	vector<int> * getKeyNodes();
private:
	int communityId;
	int threshold;
	vector<int>* communityNodeIds; // store id of nodes in community
	map<int, vector<int>> mapReachable; // store id (id of community nodes) -> list of node ids that can reach this node
	map<int, vector<int>> mapTouch; // store node id -> list of node ids that in community of dcr graph that this node can touch
	
	map<int, int> trackGain; // store node id -> gain (int type) of that node to this dcr graph , USED FOR SANDWICH SOLUTION
	vector<int> keyNode; // store nodes that can influence this graph at first
};

