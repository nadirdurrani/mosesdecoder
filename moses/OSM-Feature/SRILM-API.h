#ifndef API
#define API

#include "Ngram.h"

using namespace std;

class Api
{

	public:

	Api();
	~Api();
	void read_lm(char *,int);
	float lm_logprobContext( const VocabString word, const VocabString *context );
	float lm_logprobSent( const VocabString *sentence );
	double contextProb(char *, int & );
	double contextProbN (vector <int> , int &);
	unsigned backOffLength (vector <int>);

	double sentProb(char *) ;
	int getLMID(char *);

	private :
		
	LM *LanguageModel;

};

#endif
