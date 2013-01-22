#ifndef OSMHYPOTHESIS
#define OSMHYPOTHESIS
# include "SRILM-API.h"

using namespace std;

class osmHypothesis
{

	private:
	
	vector <string> operations;	// List of operations required to generated this hyp ...
	map <int,string> gap;	// Maintains gap history ...
	int j;	// Position after the last source word generated ...
	int E; // Position after the right most source word so far generated ...
	int gapCount; // Number of gaps inserted ...
	int deletionCount;
	int openGapCount;
	int gapWidth;	
	double opProb;

	int closestGap(map <int,string> gap,int j1, int & gp);
	int firstOpenGap(vector <int> & coverageVector);
	string intToString(int);
	int  getOpenGaps();
 
	public:

	osmHypothesis();
	~osmHypothesis(){};
	void generateOperations(int j1 , int contFlag , vector <int> & coverageVector , string english , string german , set <int> & targetNullWords , vector <string> & currF);
	void calculateOSMProb(Api & opPtr , vector <string> & history , int order);
	void print();
};

#endif

