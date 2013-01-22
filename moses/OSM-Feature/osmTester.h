#pragma once

#include "osmHyp.h"
#include "SRILM-API.h"

class Phrase
{

	public :
	
	Phrase(){currE.clear(), currF.clear(), ceptsInPhrase.clear(), targetNullWords.clear();};
	~Phrase(){};
	void getPhraseInFormat(int startIndex , string t);
	void computeOSMFeature(int startIndex , vector <int> & coverageVector , Api & opPtr , vector <string> & history , int order);
	
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
	void constructCepts(int startIndex , vector <string> & alignment);	
	int stringToInteger(string s);
	
};



