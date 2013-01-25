#include <cstdlib>
#include <map>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
#include <set>
#include <sstream>
#include "osmTester.h"

using namespace std;
using namespace Moses;


void OSMPhrase :: computeOSMFeature(int startIndex , WordsBitmap & coverageVector , Api & ptrOp , vector <string> & history , int order)
{

	set <int> eSide;
	set <int> fSide;
	set <int> :: iterator iter;
	string english;
	string source;
	int j1;
	int start = 0;

	osmHypothesis * ptr = new osmHypothesis();

	if (targetNullWords.size() != 0)
	{
		iter = targetNullWords.begin();

		if (*iter == startIndex)
		{

			j1 = startIndex;
			source = currF[j1-startIndex];
			english = "_INS_";
			ptr->generateOperations(startIndex, j1, 2 , coverageVector , english , source , targetNullWords , currF);
		}
	}
	

	for (int i = 0; i < ceptsInPhrase.size(); i++)
	{
		source = "";
		english = "";

		fSide = ceptsInPhrase[i].first;
		eSide = ceptsInPhrase[i].second;

		iter = eSide.begin();
		english += currE[*iter];	
		iter++;	
	
		for (; iter != eSide.end(); iter++)
		{
			english += "^_^";
			english += currE[*iter];	
		} 
		
		iter = fSide.begin(); 
		source += currF[*iter];
		iter++;

		for (; iter != fSide.end(); iter++)
		{
			source += "^_^";
			source += currF[*iter];	
		}

		iter = fSide.begin();
		j1 = *iter + startIndex;
		iter++; 

		ptr->generateOperations(startIndex, j1, 0 , coverageVector , english , source , targetNullWords , currF);

		
		for (; iter != fSide.end(); iter++)
		{
		     j1 = *iter + startIndex;
		     ptr->generateOperations(startIndex, j1, 1 , coverageVector , english , source , targetNullWords , currF);
		}
				
	}	

	ptr->calculateOSMProb(ptrOp , history , order);
	ptr->print();
	delete ptr;

}

void OSMPhrase :: getMeCepts ( set <int> & eSide , set <int> & fSide , map <int , vector <int> > & tS , map <int , vector <int> > & sT)
{
	set <int> :: iterator iter;

	int sz = eSide.size();
	vector <int> t;
	
	for (iter = eSide.begin(); iter != eSide.end(); iter++)
	{
	   t = tS[*iter];

	   for (int i = 0; i < t.size(); i++)
	   {
		fSide.insert(t[i]);
	   }
	    
	}

	for (iter = fSide.begin(); iter != fSide.end(); iter++)
	{
		
		t = sT[*iter];

		for (int i = 0 ; i<t.size(); i++)
		{	
				eSide.insert(t[i]);
		}
			
	}

	if (eSide.size () > sz)
	{
		getMeCepts(eSide,fSide,tS,sT);
	}

}

void OSMPhrase :: constructCepts(vector <int> & align , int startIndex , int endIndex)
{
	
		std::map <int , vector <int> > sT;
		std::map <int , vector <int> > tS;
		std::set <int> eSide;
		std::set <int> fSide;
		std::set <int> :: iterator iter;
		std :: map <int , vector <int> > :: iterator iter2;
		std :: pair < set <int> , set <int> > cept;
		int src;
		int tgt;


		for (int i = 0;  i < align.size(); i+=2)
		{
			src = align[i];
			tgt = align[i+1];
			tS[tgt].push_back(src);
			sT[src].push_back(tgt);
		}

		for (int i = startIndex; i<= endIndex; i++)
		{
			if (sT.find(i-startIndex) == sT.end())
			{
				targetNullWords.insert(i);
			}
		}


		while (tS.size() != 0 && sT.size() != 0)
		{

			iter2 = tS.begin();

			eSide.clear();
			fSide.clear();
			eSide.insert (iter2->first);

			getMeCepts(eSide, fSide, tS , sT);

			for (iter = eSide.begin(); iter != eSide.end(); iter++)
			{
				iter2 = tS.find(*iter);
				tS.erase(iter2);
			}

			for (iter = fSide.begin(); iter != fSide.end(); iter++)
			{
				iter2 = sT.find(*iter);
				sT.erase(iter2);
			}

			cept = make_pair (fSide , eSide);
			ceptsInPhrase.push_back(cept);
		}


		for (int i = 0; i < ceptsInPhrase.size(); i++)
			{

				fSide = ceptsInPhrase[i].first;
				eSide = ceptsInPhrase[i].second;

				for (iter = eSide.begin(); iter != eSide.end(); iter++)
				{
			   		cerr<<*iter<<" ";
				}
			    	cerr<<"<---> ";

				for (iter = fSide.begin(); iter != fSide.end(); iter++)
				{
					cerr<<*iter<<" ";
				}

				cerr<<endl;
			}
			cerr<<endl;

}

int OSMPhrase :: stringToInteger(string s)
{

	istringstream buffer(s);
	int some_int;
	buffer >> some_int;
	return some_int;
}

/*
void OSMPhrase :: getPhraseInFormat(int startIndex , string t)
{

	string e;
	string f;
	vector <string> a;
	
	readPhrases(t,e,f,a);
	getWords(e,currE);
	getWords(f,currF);
	constructCepts(startIndex, a);
	print();
	//int xx;
	//cin>>xx;
}
*
void OSMPhrase :: getWords(string inp, vector <string> & currInput)
{
	currInput.clear();

	int a=0;
	a = inp.find(' ', inp.length()-1);

	if( a == -1)
		inp.append(" ");

	a=0;
	int b=0;

	for (int j=0; j<inp.length(); j++)
	{
			
		a=inp.find(' ',b);
	
		if(a != -1)
		{	
			currInput.push_back(inp.substr(b,a-b));
			
			b=a+1;
			j=b;
		}
		else
		{
			j=inp.length();
		}
	
	}

}


void OSMPhrase :: alignmentFormat(string input, vector <string> & a)
{
	
	vector <string> words;
	getWords(input,words);
	string t;
	string s;
	int res;

	for (int i = 0; i< words.size(); i++)
	{
		res = words[i].find("-");
		s = words[i].substr(0,res);
		t = words[i].substr(res+1);
		//cout<<s<<endl;
		//cout<<t<<endl;
		//int xx;
		//cin>>xx;
		
		a.push_back(t);
		a.push_back(s);
	}
}

void OSMPhrase :: readPhrases(string input, string & e, string & f, vector <string> & a)
{
	a.clear();
	string t;
	int res = input.find("|||");
	t=input.substr(0,res-1);
	f = t;
	//cout<<t<<endl;
	t=input.substr(res+4);
	input = t;
	res = input.find("|||");
	t = input.substr(0,res-1);
	e = t;
	//cout<<t<<endl;
	//int xx;
	//cin>>xx;
	t=input.substr(res+4);
	input = t;
	res = input.find("|||");
	t=input.substr(res+4);
	input = t;
	res = input.find("|||");
	t = input.substr(0,res-1);
	alignmentFormat(t,a);
}


void OSMPhrase :: print()
{
	
	cout<<endl<<"String F : ";

	for(int i=0; i<currF.size(); i++)
		cout<<currF[i]<<" ";
	cout<<endl<<endl;

	cout<<"String E : ";
	for(int i=0; i<currE.size(); i++)
		cout<<currE[i]<<" ";

	cout<<endl<<endl;

	cout<<"Mapping Function : "<<endl;

	set <int> eSide;
	set <int> fSide;
	set <int> :: iterator iter;

		
	for (int i = 0; i < ceptsInPhrase.size(); i++)
	{

		fSide = ceptsInPhrase[i].first;
		eSide = ceptsInPhrase[i].second;

		for (iter = eSide.begin(); iter != eSide.end(); iter++)
		{
	   		cout<<currE[*iter]<<" ";
		}
	    	cout<<"<---> ";

		for (iter = fSide.begin(); iter != fSide.end(); iter++)
		{
			cout<<currF[*iter]<<" ";
		}

		cout<<endl;
	}
	cout<<endl;


}


/*
void loadInput(char * fileName, vector <string> & input)
{

	ifstream sr (fileName);
	char* tmp;
	
	if(sr.is_open())
	{
		while(! sr.eof() )
		{
						
			tmp= new char[5000];
			sr.getline (tmp,5000);
			input.push_back(tmp);
			//cout<<tmp<<input.size()<<endl;
			delete [] tmp;
		}
	
		sr.close();
	}
	else
	{
		cout<<"Unable to read "<<fileName<<endl;
		exit(1);
	}


}


void lmInit(int order , Api & ptrOp)
{

	char * LM = new char[strlen("operationLM9")+1];
	strcpy(LM,"operationLM9");
	//ptrOp = new Api;
	ptrOp.read_lm(LM,order);
	//delete [] LM;
}


int main(int argc, char * argv[])
{

	Api ptrOp;
	
	vector <string> input;
	vector <string> history;
	history.push_back("_INS_daran");
	history.push_back("_INS_zu");

	string t;
	int order = 9;

	loadInput(argv[1],input);
	lmInit(order , ptrOp);

	map <string , Phrase> phrases;
	
	vector <int> coverageVector;

	for(int i=0; i<input.size()-1; i++)
	{

		Phrase * ptr = new Phrase;
		coverageVector.clear();
		coverageVector.resize(50,0);
		t = input[i];
		cout<<i<<" "<<t<<endl<<endl;
		
		ptr->getPhraseInFormat(0 , t);
		ptr->computeOSMFeature(0,coverageVector, ptrOp , history , order);
		
		delete ptr;
	}	

}
*/
