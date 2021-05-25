#pragma once
class Constant
{
public:
	Constant();
	~Constant();

	static int K;
	static bool IS_WEIGHTED;
	static int COMMUNITY_POPULATION;
	static const double PERCENTAGE_THRESHOLD;
	static const double EPSILON;
	static const double DELTA;
	static const int NUM_THREAD = 64;
	static bool IS_BOUNDED_THRESHOLD;
	static const bool MODEL = true; // true : LT ; false : IC
};

