#pragma once

#include "osmHyp.h"
#include "SRILM-API.h"
#include "moses/Manager.h"

namespace Moses
{

class OSMPhrase
{

	public :
	
	OSMPhrase(){currE.clear(), currF.clear(), ceptsInPhrase.clear(), targetNullWords.clear();};
	~OSMPhrase(){};
	//void getPhraseInFormat(int startIndex , string t);
	void computeOSMFeature(int startIndex , WordsBitmap & coverageVector , Api & ptrOp , vector <string> & history , int order);
	void constructCepts(vector <int> & align , int startIndex , int endIndex);
	void setPhrases(vector <string> & val1 , vector <string> & val2){currF = val1; currE = val2;}


	private :

	vector <string> currE;
	vector <string> currF;
	vector < pair < set <int> , set <int> > > ceptsInPhrase;
	set <int> targetNullWords;
	
	void getWords(string inp, vector <string> & currInput);
	void alignmentFormat(string input, vector <string> & a);
	void readPhrases(string input, string & e, string & f, vector <string> & a);
	void getMeCepts ( set <int> & eSide , set <int> & fSide , map <int , vector <int> > & tS , map <int , vector <int> > & sT);

	void print();
	int stringToInteger(string s);
	
};

}


