#ifndef OSMHYPOTHESIS
#define OSMHYPOTHESIS
# include "SRILM-API.h"
# include "moses/FFState.h"
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
  int Compare(const osmState& other) const;
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
	void generateOperations(int j1 , int contFlag , std::vector <int> & coverageVector , std::string english , std::string german , std::set <int> & targetNullWords , std::vector <std::string> & currF);
	void calculateOSMProb(Api & opPtr , std::vector <std::string> & hist , int order);
	void print();
};

} // namespace

#endif

