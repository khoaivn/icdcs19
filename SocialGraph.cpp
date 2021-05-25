#include "SocialGraph.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "Constant.h"
#include <string>
#include <algorithm>
#include <sstream>
#include <queue>

using namespace std;


SocialGraph::SocialGraph()
{
	commonInstance = Common::getInstance();
}


SocialGraph::~SocialGraph()
{
}

void SocialGraph::readSocialGraphFromFile(string file)
{
	clear();
	srand(time(NULL));
	ifstream inputFile;
	inputFile.open(file);
	if (inputFile) {
		int nodeId = 0, commId = 0;
		inputFile >> nodeId >> commId;
		while (nodeId != -1) {
			if (listCommListNodeIds.size() > commId) {
				listCommListNodeIds[commId].push_back(nodeId);
			}
			else {
				vector<int> tmp;
				tmp.push_back(nodeId);
				listCommListNodeIds.push_back(tmp);
			}
			listNodeIds.push_back(nodeId);
			mapNodeId2CommId[nodeId] = commId;
			inputFile >> nodeId >> commId;
		}

		// setup hMax
		if (Constant::IS_BOUNDED_THRESHOLD)
			hMax = 2;
		else {
			for (int i = 0; i < listCommListNodeIds.size(); i++) {
				if (hMax < listCommListNodeIds[i].size() * Constant::PERCENTAGE_THRESHOLD)
					hMax = (int)listCommListNodeIds[i].size() * Constant::PERCENTAGE_THRESHOLD;
			}
		}

		int startNode, endNode;
		double weight;
		while (inputFile >> startNode >> endNode >> weight) {
			if (mapIncommingNeighbors.find(endNode) != mapIncommingNeighbors.end()) {
				mapIncommingNeighbors[endNode].push_back(pair<int, double>(startNode, weight));
			}
			else {
				vector<pair<int, double>> tmp;
				tmp.push_back(pair<int, double>(startNode, weight));
				mapIncommingNeighbors.insert(pair<int, vector<pair<int, double>>>(endNode, tmp));
			}
			noOfEdges++;
		}
		inputFile.close();
	}
}

void SocialGraph::readSocialGraph(string inputFile, bool isDirected)
{
	this->isDirected = isDirected;
	clear();
	ifstream file;
	file.open(inputFile);
	if (file) {
		int startId, endId;
		int count = 0;
		while (file >> startId >> endId) {
			mapIncommingNeighbors[endId].push_back(pair<int,double>(startId,0));

			if (!isDirected)
				mapIncommingNeighbors[startId].push_back(pair<int, double>(endId,0));

			if (find(listNodeIds.begin(), listNodeIds.end(), startId) == listNodeIds.end())
				listNodeIds.push_back(startId);

			if (find(listNodeIds.begin(), listNodeIds.end(), endId) == listNodeIds.end())
				listNodeIds.push_back(endId);

			noOfEdges++;
		}
	}
}

void extractFromCharArray(char * cArray, int sp, int ep) {
	std::string::size_type sz;
	int startId = stoi(cArray + sp, &sz);
	int endId = stoi(cArray + sz);
}

void SocialGraph::readSocialGraphFromLargeFile(string inputFile) {
	clear();
	ifstream is(inputFile);
	is.seekg(0, is.end);
	long bufSize = is.tellg();
	is.seekg(0, is.beg);
	int item = 0;
	
	char * buffer = new char[bufSize];

	is.read(buffer, bufSize);
	is.close();


	std::string::size_type sz = 0;
	long sp = 0;
	int startId, endId;
	bool isStart = true;

	map<int, vector<int>> mapTmp;
	map<int, bool> mapNode;
	
	while (sp < bufSize) {
		char c = buffer[sp];
		item = item * 10 + c - 48;
		sp++;
		if (sp == bufSize || (sp < bufSize && (buffer[sp] == '\t' || buffer[sp] == '\n'))) {
			sp++;
			
			if (isStart) {
				startId = item;
				isStart = false;
			}
			else {
				endId = item;
				isStart = true;
				mapTmp[endId].push_back(startId);
				noOfEdges++;
			}

			mapNode[item] = true;

			item = 0;
		}
	}

	for (map<int, bool>::iterator it = mapNode.begin(); it != mapNode.end(); ++it) {
		listNodeIds.push_back(it->first);
	}

	for (map<int, vector<int>>::iterator it = mapTmp.begin(); it != mapTmp.end(); ++it) {
		double w = 1.0 / it->second.size();
		vector<pair<int, double>> tmp;
		for (int i = 0; i < it->second.size(); ++i) {
			tmp.push_back(pair<int, double>(it->second[i], w));
		}
		mapIncommingNeighbors[it->first] = tmp;
	}

	// assign community

	// assign community
	int numberOfNodes = listNodeIds.size();
	int numberOfCommunities = ceil(((double)numberOfNodes) / Constant::COMMUNITY_POPULATION);

	vector<int> index;
	for (int i = 0; i < numberOfNodes; i++)
		index.push_back(i);


	std::random_shuffle(index.begin(), index.end());
	listCommListNodeIds = vector<vector<int>>(numberOfCommunities, vector<int>());
	for (int i = 0; i < numberOfNodes; i++) {
		int commId = i / Constant::COMMUNITY_POPULATION;
		listCommListNodeIds[commId].push_back(listNodeIds[index[i]]);
		mapNodeId2CommId[listNodeIds[index[i]]] = commId;
	}

	/*int numberOfNodes = listNodeIds.size();
	int numberOfCommunities = ceil(((double)numberOfNodes) / Constant::COMMUNITY_POPULATION);
	
	srand(time(NULL));
	for (int i = 0; i < numberOfNodes; i++) {
		int nodeId = listNodeIds[i];
		int comm = rand() % numberOfCommunities;
		listCommListNodeIds[comm].push_back(nodeId);
	}*/

	// setup hMax
	hMax =  Constant::IS_BOUNDED_THRESHOLD ? 2 : 0;
	if (!Constant::IS_BOUNDED_THRESHOLD) {
		for (int i = 0; i < listCommListNodeIds.size(); i++) {
			if (hMax < listCommListNodeIds[i].size() * Constant::PERCENTAGE_THRESHOLD)
				hMax = (int)listCommListNodeIds[i].size() * Constant::PERCENTAGE_THRESHOLD;
			if (bMin > listCommListNodeIds[i].size())
				bMin = listCommListNodeIds[i].size();
		}
	}
	
	std::cout << "done reading file";
	delete[] buffer;
}

void SocialGraph::generateFile(string inputFile)
{
	ifstream file;
	file.open(inputFile);
	if (file) {
		map<int, vector<int>> mapIncommingNeighbors;
		vector<int> listNodeIds;
		int startId, endId;
		int count = 0;
		while (file >> startId >> endId) {
			/*cout << count << endl;
			count++;*/
			mapIncommingNeighbors[endId].push_back(startId);
			/*
			if (mapIncommingNeighbors.find(endId) != mapIncommingNeighbors.end()) {
				mapIncommingNeighbors[endId].push_back(startId);
			}
			else {
				vector<int> tmp;
				tmp.push_back(startId);
				mapIncommingNeighbors.insert(std::pair<int, vector<int>>(endId, tmp));
			}
			*/
			if (find(listNodeIds.begin(), listNodeIds.end(), startId) == listNodeIds.end())
				listNodeIds.push_back(startId);

			if (find(listNodeIds.begin(), listNodeIds.end(), endId) == listNodeIds.end())
				listNodeIds.push_back(endId);
		}

		// assign community
		int numberOfNodes = listNodeIds.size();
		int numberOfCommunities = ceil(((double)numberOfNodes)/Constant::COMMUNITY_POPULATION);

		vector<int> index;
		for (int i = 0; i < numberOfNodes; i++)
			index.push_back(i);

		
		std::random_shuffle(index.begin(), index.end());
		map<int, vector<int>> mapCommunities; // map community Id -> list of nodes in community
		for (int i = 0; i < numberOfNodes; i++) {
			int commId = i / Constant::COMMUNITY_POPULATION;
			mapCommunities[commId].push_back(listNodeIds[index[i]]);
		}

		
		//srand(time(NULL));
		//for (int i = 0; i < numberOfNodes; i++) {
		//	int nodeId = listNodeIds[i];
		//	int comm = rand() % numberOfCommunities;
		//	if (mapCommunities.find(comm) != mapCommunities.end()) {
		//		mapCommunities[comm].push_back(nodeId);
		//	}
		//	else {
		//		vector<int> tmp;
		//		tmp.push_back(nodeId);
		//		mapCommunities.insert(std::pair<int, vector<int>>(comm, tmp));
		//	}
		//}

		// write to file
		string outFileName = "data_" + to_string(numberOfNodes) + ".txt";
		ofstream writeFile (outFileName);
		if (writeFile.is_open()) {
			// write list of communities first. Format: node id - comm id 
			for (map<int, vector<int>>::iterator it = mapCommunities.begin(); it != mapCommunities.end(); ++it) {
				for (int i = 0; i < it->second.size(); i++) {
					writeFile << it->second[i] << " " << it->first << "\n";
				}
			}
			// separate list of comm and list of edges by -1
			writeFile << "-1 -1\n";

			// write list of edges. Format: startid endid weight
			for (map<int, vector<int>>::iterator it = mapIncommingNeighbors.begin(); it != mapIncommingNeighbors.end(); ++it) {
				for (int i = 0; i < it->second.size(); i++) {
					writeFile << it->first << " " << it->second[i] << " ";
					writeFile << ((double)1) / it->second.size();
					writeFile << "\n";
				}
			}
			writeFile.close();
		}
		file.close();
	}
}

void SocialGraph::generateFileIM(string outputFile)
{
	map<int, int> mapNodeIdx;
	for (int i = 0; i < listNodeIds.size(); i++) {
		mapNodeIdx[listNodeIds[i]] = i + 1;
	}
	ofstream writeFile(outputFile);
	if (writeFile.is_open()) {
		// first line: no of nodes - no of edges
		writeFile << listNodeIds.size() << " " << noOfEdges << endl;

		// next - list of edge: start node - end node - weight
		for (int i = 0; i < listNodeIds.size(); i++) {
			int nodeId = listNodeIds[i];
			vector<std::pair<int, double>> neighbors = mapIncommingNeighbors[nodeId];
			if (neighbors.size() > 0) {
				double w = Constant::MODEL ? 1.0 / (neighbors.size() + 1) : 1 / neighbors.size(); // weight is different between LT and IC
				for (int j = 0; j < neighbors.size(); j++) {
					int tmp = neighbors[j].first;
					writeFile << mapNodeIdx[tmp] << " " << mapNodeIdx[nodeId] << " " << w << endl;
				}
			}
		}
		writeFile.close();
	}
}

void SocialGraph::standardize(string file, bool header)
{
	map<int, int> mapNodeId2Index;
	for (int i = 0; i < listNodeIds.size(); i++) {
		mapNodeId2Index[listNodeIds[i]] = i;
	}
	ofstream writeFile(file);
	if (writeFile.is_open()) {
		if (header)
			writeFile << listNodeIds.size() << " " << noOfEdges << endl;
		
		for (map<int, vector<std::pair<int, double>>>::iterator it = mapIncommingNeighbors.begin(); it != mapIncommingNeighbors.end(); ++it) {
			int nodeId = it->first;
			vector<std::pair<int, double>> listNei = it->second;
			for (int i = 0; i < listNei.size(); i++) {
				if ((!isDirected && nodeId < listNei[i].first) || isDirected)
					writeFile << mapNodeId2Index[nodeId] << " " << mapNodeId2Index[listNei[i].first] << endl;
			}
		}

		writeFile.close();
	}
}

void SocialGraph::formCommunityModularity(string output, bool directed)
{
	string file = "tmp1.adj";
	standardize(file);
	string tmp2 = "../ldf/ldf -i " + file + " -o " + output;
	if (directed)
		tmp2 = tmp2 + " -d";
	const char * runMMcmd = tmp2.c_str();
	system(runMMcmd);
}

void SocialGraph::formCommunityClauset(string output)
{
	string file = "tmp2.adj";
	standardize(file, false);
	string tmp2 = "../snap-master/examples/community/community -i:" + file + " -o:" + output;
	const char * runCcmd = tmp2.c_str();
	system(runCcmd);
}

void SocialGraph::readCommunityFile(string file, bool isMM)
{
	actualCommNodeIds.clear();
	ifstream inputFile;
	inputFile.open(file);
	if (inputFile) {
		if (isMM) { // community file generated by ldf
			int commId = 0;
			string tmp;
			vector<int> commNodes;
			while (getline(inputFile, tmp)) {
				istringstream iss(tmp);
				while (iss) {
					int nodeIndex;
					iss >> nodeIndex;
					commNodes.push_back(listNodeIds[nodeIndex]);
				}
				actualCommNodeIds.push_back(commNodes);
				commNodes.clear();
			}
		}
		else { // community file generated by Girvan algorithms
			string tmp;

			// first 6 lines are useless
			getline(inputFile, tmp);
			getline(inputFile, tmp);
			getline(inputFile, tmp);
			getline(inputFile, tmp);
			getline(inputFile, tmp);
			getline(inputFile, tmp);

			int nodeIndex, actualCommId, prevCommmId = 0, commId = 0;

			vector<int> commNodes;
			while (getline(inputFile, tmp)) {
				istringstream iss(tmp);
				iss >> nodeIndex >> actualCommId;
				if (actualCommId == prevCommmId) {
					commNodes.push_back(listNodeIds[nodeIndex]);
				} 
				else {
					actualCommNodeIds.push_back(commNodes);
					commNodes.clear();
					commNodes.push_back(listNodeIds[nodeIndex]);
					prevCommmId = actualCommId;
				}

			}
			if (!commNodes.empty()) {
				actualCommNodeIds.push_back(commNodes);
			}
		}
	}
}

void SocialGraph::formCommunitiesFromActualCommunities()
{
	listCommListNodeIds.clear();
	mapNodeId2CommId.clear();
	hMax = 0;
	bMin = 10000;
	vector<int> commNodes;
	int commId = 0;
	for (int i = 0; i < actualCommNodeIds.size(); i++) {
		if (actualCommNodeIds[i].size() <= Constant::COMMUNITY_POPULATION) {
			addCommunity(&actualCommNodeIds[i]);
			for (int j = 0; j < actualCommNodeIds[i].size(); j++)
				mapNodeId2CommId[actualCommNodeIds[i][j]] = commId;
			commId++;
		}
		else {
			commNodes.clear();
			for (int j = 0; j < actualCommNodeIds[i].size(); j++) {
				int nodeId = actualCommNodeIds[i][j];
				commNodes.push_back(nodeId);
				mapNodeId2CommId[nodeId] = commId;
				if (commNodes.size() >= Constant::COMMUNITY_POPULATION) {
					addCommunity(&commNodes);
					commNodes.clear();
					commId++;
				}
			}
			if (!commNodes.empty()) {
				addCommunity(&commNodes);
				commNodes.clear();
				commId++;
			}
		}
	}
}



int SocialGraph::randomSelectCommunity()
{
	if (!Constant::IS_WEIGHTED)
		return commonInstance->randomInThread() % listCommListNodeIds.size();
	else {
		int tmp = commonInstance->randomInThread() % listNodeIds.size();
		return mapNodeId2CommId[listNodeIds[tmp]];
	}
}

vector<int>* SocialGraph::getNodesOfCommunity(int commId)
{
	return &listCommListNodeIds[commId];
}

vector<std::pair<int, double>>* SocialGraph::getIncommingNeighbors(int nodeId)
{
	if (mapIncommingNeighbors.find(nodeId) != mapIncommingNeighbors.end())
		return &mapIncommingNeighbors[nodeId];
	else
		return nullptr;
}

map<int, vector<std::pair<int, double>>>* SocialGraph::getMapIncommingNeighbors()
{
	return &mapIncommingNeighbors;
}

vector<int>* SocialGraph::getListNodeIds()
{
	return &listNodeIds;
}

int SocialGraph::getMaxThreshold()
{
	return Constant::IS_BOUNDED_THRESHOLD ? 2 : hMax;
}

int SocialGraph::getNumberOfNodes()
{
	return listNodeIds.size();
}

int SocialGraph::getNumberOfCommunities()
{
	return listCommListNodeIds.size();
}

int SocialGraph::getCommunityThreshold(int commId)
{
	if (Constant::IS_BOUNDED_THRESHOLD)
		return 2;
	else 
		return listCommListNodeIds[commId].size() * Constant::PERCENTAGE_THRESHOLD;
}

int SocialGraph::getCommunityId(int nodeId)
{
	return mapNodeId2CommId[nodeId];
}

int SocialGraph::getCommunitySize(int commId)
{
	return listCommListNodeIds[commId].size();
}

int SocialGraph::getMinBenefit()
{
	return bMin;
}

void SocialGraph::clear()
{
	listNodeIds.clear();
	listCommListNodeIds.clear();
	mapIncommingNeighbors.clear();
	mapNodeId2CommId.clear();
	noOfEdges = 0;
	hMax = 0;
	bMin = 10000;
}

void SocialGraph::addCommunity(vector<int> * commNodes)
{
	listCommListNodeIds.push_back(vector<int>(*commNodes));
	if (hMax < commNodes->size() * Constant::PERCENTAGE_THRESHOLD)
		hMax = (int)commNodes->size() * Constant::PERCENTAGE_THRESHOLD;
	if (bMin > commNodes->size())
		bMin = commNodes->size();
}
