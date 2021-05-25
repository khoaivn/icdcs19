#include "Common.h"
#include <algorithm>

Common * Common::instance = nullptr;

Common::Common()
{
	seed = new int[100];
	for (int i = 0; i < 100; i++) {
		seed[i] = rand();
	}
}


Common::~Common()
{
}

Common * Common::getInstance()
{
	if (instance == nullptr) 
		instance = new Common();
	return instance;
}

unsigned Common::nChoosek(unsigned n, unsigned k)
{
	if (k > n) return 0;
	if (k == 0) return 1;
	unsigned re = n;
	for (int i = 2; i <= k; i++) {
		re *= (n - i + 1);
		re /= i;
	}
	return re;
}

bool Common::isIntersected(vector<int>* set1, vector<int>* set2)
{
	int j = 0;
	for (int i = 0; i < set1->size(); i++) {
		while (j < set2->size() && (*set2)[j] < (*set1)[i]) {
			j++;
		}
		if (j < set2->size() && (*set2)[j] == (*set1)[i])
			return true;
		else if (j == set2->size())
			return false;
	}
	return false;
}

vector<int> Common::setDifference(vector<int>* set1, vector<int>* set2)
{
	vector<int> re;
	for (int i = 0; i < set1->size(); i++) {
		if (find(set2->begin(), set2->end(), set1->at(i)) == set2->end()) {
			re.push_back(set1->at(i));
		}
	}
	return re;
}

unsigned Common::randomInThread()
{
	unsigned tmp = seed[omp_get_thread_num() % 100];
	tmp = tmp * 17931 + 7391;
	seed[omp_get_thread_num() % 100] = tmp;
	return tmp;
}

