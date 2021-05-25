#include "DCRgraph.h"
#include "Common.h"
#include <algorithm>

DCRgraph::DCRgraph(int communityId, int threshold, vector<int>* communityNodeIds)
{
	this->communityId = communityId;
	this->threshold = threshold;
	this->communityNodeIds = communityNodeIds;
}

DCRgraph::~DCRgraph()
{

}

void DCRgraph::addReachable(int nodeIds, vector<int> * reachNodeIds)
{
	vector<int> reachable(*reachNodeIds);
	this->mapReachable.insert(std::pair<int, vector<int>>(nodeIds, reachable));
	for (int i = 0; i < reachable.size(); i++) {
		int nodeId = reachable[i];
		if (mapTouch.find(nodeId) != mapTouch.end()) {
			mapTouch[nodeId].push_back(nodeIds);
		}
		else {
			vector<int> tmp;
			tmp.push_back(nodeIds);
			mapTouch.insert(pair<int, vector<int>>(nodeId, tmp));
		}
	}
}

map<int, vector<int>>* DCRgraph::getMapReachable()
{
	return &mapReachable;
}

int DCRgraph::getPopulation()
{
	return communityNodeIds->size();
}

vector<int>* DCRgraph::getCommunityNodeIds()
{
	return communityNodeIds;
}

vector<int>* DCRgraph::getReachableIds(int nodeId)
{
	return &(mapReachable[nodeId]);
}

int DCRgraph::getThreshold()
{
	return threshold;
}

int DCRgraph::getCommunityId()
{
	return communityId;
}

vector<int> DCRgraph::getCurrentLive(vector<int>* sol)
{
	/*
	vector<int> re;
	for (int i = 0; i < communityNodeIds->size(); i++) {
		int nodeId = (*communityNodeIds)[i];
		vector<int> reachable = mapReachable[nodeId];
		if (!Common::getInstance()->isIntersected(sol, &reachable))
			re.push_back(nodeId);
	}
	if (re.size() > communityNodeIds->size() - threshold)
		return re;
	else return vector<int>();
	*/

	vector<int> re;
	vector<int> currentKill;
	for (int i = 0; i < sol->size(); i++) {
		int nodeId = (*sol)[i];
		if (mapTouch.find(nodeId) != mapTouch.end()) {
			vector<int> tmp = mapTouch[nodeId];
			for (int j = 0; j < tmp.size(); j++) {
				currentKill.push_back(tmp[j]);
			}
		}
	}
	for (int i = 0; i < communityNodeIds->size(); i++) {
		int nodeId = (*communityNodeIds)[i];
		if (find(currentKill.begin(), currentKill.end(), nodeId) == currentKill.end())
			re.push_back(nodeId);
	}
	if (re.size() > communityNodeIds->size() - threshold)
		return re;
	else return vector<int>();
}

/*
int DCRgraph::getMarginalGain(int nodeId, vector<int>* infNodes)
{
	int gain = 0;

	if (infNodes->size() >= threshold) // if number of influenced nodes exceed threshold, then no more gain
		return 0;

	if (mapTouch.find(nodeId) != mapTouch.end()) {
		vector<int> touch = mapTouch[nodeId];
		for (int i = currentLive->size() - 1; i >= 0; i--) {
			if (find(touch.begin(), touch.end(), (*currentLive)[i]) != touch.end()) {
				gain++;
				if (gain + communityNodeIds->size() - currentLive->size() >= threshold)
					break;
			}
		}
	}
	return gain;
}
*/


int DCRgraph::getMarginalGain(int nodeId, vector<int>* currentLive)
{
	int gain = 0;

	if (currentLive->size() < communityNodeIds->size() - threshold) // if number of influenced nodes exceed threshold, then no more gain
		return 0;

	if (mapTouch.find(nodeId) != mapTouch.end()) {
		vector<int> touch = mapTouch[nodeId];
		for (int i = currentLive->size() - 1; i >= 0; i--) {
			if (find(touch.begin(), touch.end(), (*currentLive)[i]) != touch.end()) {
				gain++;
				if (gain + communityNodeIds->size() - currentLive->size() >= threshold)
					break;
			}
		}
	}
	return gain;
}


void DCRgraph::getCurrentLiveAfterAddingNode(int nodeId, vector<int>* currentLive)
{
	if (mapTouch.find(nodeId) != mapTouch.end()) {
		vector<int> touch = mapTouch[nodeId];
		for (int i = currentLive->size() -1; i >= 0; i--) {
			if (find(touch.begin(), touch.end(), (*currentLive)[i]) != touch.end()) {
				currentLive->erase(currentLive->begin() + i);
				if (currentLive->size() <= communityNodeIds->size() - threshold) {
					currentLive->clear();
					return;
				}
			}
		}
	}
}

map<int,int> DCRgraph::updateGainAndCurrentLiveAfterAddingNode(int nodeId, vector<int>* currentLive)
{
	map<int, int> re;
	if (currentLive->empty()) return re;

	if (mapTouch.find(nodeId) != mapTouch.end()) {
		vector<int> touch = mapTouch[nodeId];
		for (int i = currentLive->size() - 1; i >= 0; i--) {
			int nodeId = currentLive->at(i);
			if (find(touch.begin(), touch.end(), nodeId) != touch.end()) {
				currentLive->erase(currentLive->begin() + i);
				vector<int> reachable = mapReachable[nodeId];

				for (int j = 0; j < reachable.size(); j++) {
					int tmp = reachable[j];
					re[tmp]++;
					trackGain[tmp]--;
				}
				
				if (currentLive->size() <= communityNodeIds->size() - threshold) { // if this graph is influenced
					// reduce gain of all other node that touch remaining node of community in this dcr graph
					for (map<int, int>::iterator it = trackGain.begin(); it != trackGain.end(); ++it) {
						re[it->first] += it->second;
					}
					currentLive->clear();
					return re;
				}
			}
		}

		int toInf = currentLive->size() - communityNodeIds->size() + threshold;
		for (map<int, int>::iterator it = trackGain.begin(); it != trackGain.end(); ++it) {
			if (it->second > toInf) {
				re[it->first] += (it->second - toInf);
				trackGain[it->first] = toInf;
			}
		}
	}

	return re;
}

map<int, bool> DCRgraph::updateGainAndCurrentLiveAfterAddingNodeCG(int nodeId, vector<int>* currentLive, vector<int>* canKill)
{
	map<int, bool> re;
	if (currentLive->empty()) return re;

	if (mapTouch.find(nodeId) != mapTouch.end()) {
		vector<int> touch = mapTouch[nodeId];
		for (int i = currentLive->size() - 1; i >= 0; i--) {
			int nodeId = currentLive->at(i);
			if (find(touch.begin(), touch.end(), nodeId) != touch.end()) {
				currentLive->erase(currentLive->begin() + i);

				if (currentLive->size() <= communityNodeIds->size() - threshold) {
					currentLive->clear();
					for (int j = 0; j < canKill->size(); j++)
						re[canKill->at(j)] = false; // false if reduced gain
					canKill->clear();
					return re;
				}
			}
		}

		for (map<int, vector<int>>::iterator it = mapTouch.begin(); it != mapTouch.end(); it++) {
			int nodeId = it->first;
			if (find(canKill->begin(), canKill->end(), nodeId) == canKill->end()) {
				int count = 0;
				vector<int> kill = it->second;
				for (int i = 0; i < kill.size(); i++) {
					if (find(currentLive->begin(), currentLive->end(), kill[i]) != currentLive->end()) {
						count++;
					}

					if (communityNodeIds->size() - currentLive->size() + count >= threshold) {
						canKill->push_back(nodeId);
						re[nodeId] = true; // true if increasing gain
						break;
					}
				}
			}
		}
	}

	return re;
}

void DCRgraph::updateInitalGain(map<int, double>* gain, map<int, int> * mapDead, map<int, double> * mapInfMaf)
{
	keyNode.clear();
	for (map<int, vector<int>>::iterator it = mapTouch.begin(); it != mapTouch.end(); ++it) {
		int nodeId = it->first;
		int touch = it->second.size();
		double tmp = touch > threshold ? threshold : touch;
		tmp = tmp / threshold;
		(*gain)[nodeId] += tmp;

		if (touch >= threshold && mapDead != nullptr) {
			(*mapDead)[nodeId]++;
		}

		if (mapInfMaf != nullptr && mapReachable.find(nodeId) == mapReachable.end()) {
			(*mapInfMaf)[nodeId] += tmp;
		}

		if (touch >= threshold)
			keyNode.push_back(nodeId);
	}
}

bool DCRgraph::isKill(vector<int>* sol)
{
	vector<int> kill;
	for (int i = 0; i < sol->size(); i++) {
		int nodeId = (*sol)[i];
		if (mapTouch.find(nodeId) != mapTouch.end()) {
			vector<int> tmp = mapTouch[nodeId];
			for (int j = 0; j < tmp.size(); j++) {
				if (find(kill.begin(), kill.end(), tmp[j]) == kill.end()) {
					kill.push_back(tmp[j]);
					if (kill.size() >= threshold)
						return true;
				}
			}
		}
	}
	return false;
}

double DCRgraph::fractionalInf(vector<int>* sol)
{
	vector<int> kill;
	for (int i = 0; i < sol->size(); i++) {
		int nodeId = (*sol)[i];
		if (mapTouch.find(nodeId) != mapTouch.end()) {
			vector<int> tmp = mapTouch[nodeId];
			for (int j = 0; j < tmp.size(); j++) {
				if (find(kill.begin(), kill.end(), tmp[j]) == kill.end()) {
					kill.push_back(tmp[j]);
					if (kill.size() >= threshold)
						return 1;
				}
			}
		}
	}
	return ((double)kill.size()) / threshold;
}

bool DCRgraph::isTouchedByNode(int nodeId)
{
	return (mapTouch.find(nodeId) != mapTouch.end());
}

vector<int>* DCRgraph::getListTouchedNode()
{
	vector<int> * list = new vector<int>();
	for (map<int, vector<int>>::iterator it = mapTouch.begin(); it != mapTouch.end(); ++it) {
		list->push_back(it->first);
	}
	return list;
}

vector<int>* DCRgraph::getListTouchedNode(int excludeId)
{
	vector<int> * list = new vector<int>();

	if (mapTouch.find(excludeId) == mapTouch.end() ||  mapTouch[excludeId].size() < threshold)
	{
		for (map<int, vector<int>>::iterator it = mapTouch.begin(); it != mapTouch.end(); ++it) {
			if (it->first != excludeId)
				list->push_back(it->first);
		}
	}

	return list;
}

void DCRgraph::initiateTrackGain()
{
	for (map<int, vector<int>>::iterator it = mapTouch.begin(); it != mapTouch.end(); ++it) {
		int tmp = it->second.size();
		tmp = tmp < threshold ? tmp : threshold;
		trackGain[it->first] = tmp;
	}
}

map<int, int>* DCRgraph::getTrackGain()
{
	return &trackGain;
}

vector<int>* DCRgraph::getKeyNodes()
{
	return &keyNode;
}


