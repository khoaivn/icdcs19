#pragma once
#include <vector>
#include <omp.h>
using namespace std;

class Common
{
public:
	Common();
	~Common();

	static Common * getInstance();

	unsigned nChoosek(unsigned n, unsigned k);
	bool isIntersected(vector<int> * set1, vector<int> * set2); // both 2 set is sorted
	vector<int> setDifference(vector<int> * set1, vector<int> * set2);
	unsigned randomInThread();
private:
	static Common * instance;
	int * seed;
};

