#include <cstdlib>
#include <map>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
#include <set>
#include <sstream>
#include "osmHyp.cpp"

using namespace std;

class Phrase
{

	public :
	
	Phrase(){currE.clear(), currF.clear(), ceptsInPhrase.clear(), targetNullWords.clear();};
	~Phrase(){};
	void getPhraseInFormat(int startIndex , string t);
	void computeOSMFeature(int startIndex , vector <int> & coverageVector);
	
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


void Phrase :: computeOSMFeature(int startIndex , vector <int> & coverageVector)
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
			source = currF[j1];
			english = "_INS_";
			ptr->generateOperations(j1, 0 , coverageVector , english , source , targetNullWords , currF);
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

		ptr->generateOperations(j1, 0 , coverageVector , english , source , targetNullWords , currF);

		
		for (; iter != fSide.end(); iter++)
		{
		     j1 = *iter + startIndex;
		     ptr->generateOperations(j1, 1 , coverageVector , english , source , targetNullWords , currF);		
		}
				
	}	

	ptr->print();
	delete ptr;

}

void Phrase :: getMeCepts ( set <int> & eSide , set <int> & fSide , map <int , vector <int> > & tS , map <int , vector <int> > & sT)
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

void Phrase :: constructCepts(int startIndex, vector <string> & alignment)
{
	
	ceptsInPhrase.clear();	

	map <int , vector <int> > sT;
	map <int , vector <int> > tS;
	set <int> eSide;
	set <int> fSide;
	set <int> :: iterator iter;
	map <int , vector <int> > :: iterator iter2;
	pair < set <int> , set <int> > cept;
	
	int index1;
	int index2;
	

	for (int i =0 ; i < alignment.size(); i+=2)
	{
		index1=stringToInteger(alignment[i]);
		index2=stringToInteger(alignment[i+1]);
		
		tS[index1].push_back(index2);
		sT[index2].push_back(index1);
	}

	for (int i = 0; i< currF.size(); i++)
	{
		if (sT.find(i) == sT.end())
		{
			targetNullWords.insert(startIndex + i);
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

}

int Phrase :: stringToInteger(string s)
{

	istringstream buffer(s);
	int some_int;
	buffer >> some_int;
	return some_int;
}


void Phrase :: getPhraseInFormat(int startIndex , string t)
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

void Phrase :: getWords(string inp, vector <string> & currInput)
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


void Phrase :: alignmentFormat(string input, vector <string> & a)
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

void Phrase :: readPhrases(string input, string & e, string & f, vector <string> & a)
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


void Phrase :: print()
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


int main(int argc, char * argv[])
{

	vector <string> input;
	string t;
	
	loadInput(argv[1],input);
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
		ptr->computeOSMFeature(0,coverageVector);

		delete ptr;
	}	
	
	
}
