#pragma once

# include "SRILM-API.h"
# include "moses/FFState.h"
# include "moses/Manager.h"
# include <set>
# include <map>
# include <string>
# include <vector>

using namespace std;

namespace Moses
{

class osmState : public FFState
{
public:
  osmState();
  int Compare(const FFState& other) const;
  void saveState(int jVal, int eVal, vector <string> & history);
  std::string getName() const {cerr<<"This is inside osm state"; int xx; cin>>xx; return "inside osm";}

protected:
  int j, E;
  std::map <int,std::string> gap;
  std::vector <std::string> history;
};

class osmHypothesis
{

	private:
	
  std::vector <std::string> history;
  std::vector <std::string> operations;	// List of operations required to generated this hyp ...
	std::map <int,std::string> gap;	// Maintains gap history ...
	int j;	// Position after the last source word generated ...
	int E; // Position after the right most source word so far generated ...
	int gapCount; // Number of gaps inserted ...
	int deletionCount;
	int openGapCount;
	int gapWidth;	
	double opProb;

	int closestGap(std::map <int,std::string> gap,int j1, int & gp);
	int firstOpenGap(std::vector <int> & coverageVector);
	std::string intToString(int);
	int  getOpenGaps();
 
	public:

	osmHypothesis();
	~osmHypothesis(){};
	void generateOperations(int & startIndex, int j1 , int contFlag , WordsBitmap & coverageVector , std::string english , std::string german , std::set <int> & targetNullWords , std::vector <std::string> & currF);
	void calculateOSMProb(Api & opPtr , std::vector <std::string> & hist , int order);
	void print();

	osmState * saveState();
};

} // namespace



