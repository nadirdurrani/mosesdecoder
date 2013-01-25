#include "osmHyp.h"
#include <sstream>

namespace Moses
{
osmState::osmState()
:j(0)
,E(0)
{
  history.push_back("<s>");
}

int osmState::Compare(const FFState& otherBase) const
{
  const osmState &other = static_cast<const osmState&>(otherBase);
  if (j != other.j)
    return (j < other.j) ? -1 : +1;
  if (E != other.E)
    return (E < other.E) ? -1 : +1;
  if (gap != other.gap)
    return (gap < other.gap) ? -1 : +1;
  if (history != other.history)
    return (history < other.history) ? -1 : +1;

  return 0;
}

osmHypothesis :: osmHypothesis()
{
	opProb = 0;
	gapWidth = 0;
	gapCount = 0;
	openGapCount = 0;
	deletionCount = 0;
	gapCount = 0;
	j = 0;
	E = 0;
}

void osmHypothesis :: calculateOSMProb(Api & ptrOp , vector <string> & hist , int order)
{
	
	opProb = 0;
	vector <int> numbers;
	vector <int> context;
	int nonWordFlag  = 0;
	double temp;

	for (int i=0; i< operations.size(); i++)
		numbers.push_back(ptrOp.getLMID(const_cast <char *> (operations[i].c_str())));

	for (int i=0; i< hist.size(); i++)
		context.push_back(ptrOp.getLMID(const_cast <char *> (hist[i].c_str())));

	for (int i = 0; i<operations.size(); i++)
	{
		//cout<<opSeq[i]<<endl;
		context.push_back(numbers[i]);
		hist.push_back(operations[i]);
		//cout<<"Context Size "<<context.size()<<endl;
		if (context.size() > order)
		{
			context.erase(context.begin());
			hist.erase(hist.begin());
		}
		
		temp = ptrOp.contextProbN(context,nonWordFlag);		   
		opProb = opProb + temp;
		
		 //cout<<temp<<" "<<opProb<<endl;
	
	}

	if (hist.size() > order-1)
	{
	      hist.erase(hist.begin());
	}

	  history = hist;
}


int osmHypothesis :: firstOpenGap(vector <int> & coverageVector)
{
	
	int firstOG =-1;

	for(int nd = 0; nd < coverageVector.size(); nd++)
	{
		if(coverageVector[nd]==0)
		{
		 firstOG = nd;
		 return firstOG;
		}
	}
	
	return firstOG;	

}

string osmHypothesis :: intToString(int num)
{
 	
       std::ostringstream stm;
       stm<<num;

    return stm.str();

}

void osmHypothesis :: generateOperations(int & startIndex , int j1 , int contFlag , WordsBitmap & coverageVector , string english , string german , set <int> & targetNullWords , vector <string> & currF)
{
	
	int gFlag = 0;
	int gp = 0; 	
	int ans;
	

		if ( j < j1) // j1 is the index of the source word we are about to generate ...
		{
			//if(coverageVector[j]==0) // if source word at j is not generated yet ...
			if(coverageVector.GetValue(j)==0) // if source word at j is not generated yet ...
			{
				operations.push_back("_INS_GAP_");
				gFlag++;
				gap[j]="Unfilled";
			}
			if (j == E)
			{
				j = j1;
			}
			else
			{
				operations.push_back("_JMP_FWD_");
				j=E;
			}
		}
		
		if (j1 < j)
		{
			// if(j < E && coverageVector[j]==0)
			if(j < E && coverageVector.GetValue(j)==0)
			{
				operations.push_back("_INS_GAP_");
				gFlag++;
				gap[j]="Unfilled";
			}

			j=closestGap(gap,j1,gp);
			operations.push_back("_JMP_BCK_"+ intToString(gp));

			//cout<<"I am j "<<j<<endl;
			//cout<<"I am j1 "<<j1<<endl;

			if(j==j1)
			  gap[j]="Filled";
		}

		if (j < j1)
		{
			operations.push_back("_INS_GAP_");
			gap[j] = "Unfilled";
			gFlag++;
			j=j1;
		}

		if(contFlag == 0) // First words of the multil-word cept ...
		{

			if(english == "_TRANS_SLF_") // Unknown word ...
			{
				operations.push_back("_TRANS_SLF_");
			}
			else
			{
				operations.push_back("_TRANS_" + english + "_TO_" + german);
			}

			//ans = firstOpenGap(coverageVector);
			ans = coverageVector.GetFirstGapPos();
		
			if (ans != -1)
		 		gapWidth += j - ans;

		}
		else if (contFlag == 2)
		{

			operations.push_back("_INS_" + german);
			ans = coverageVector.GetFirstGapPos();

			if (ans != -1)
				gapWidth += j - ans;
			deletionCount++;
		}
		else
		{
			operations.push_back("_CONT_CEPT_");
		}

		//coverageVector[j]=1;
		coverageVector.SetValue(j,1);
		j+=1;
		
		if(E<j)
		  E=j;

	if (gFlag > 0)
		gapCount++;

	openGapCount += getOpenGaps();

	//if (coverageVector[j] == 0 && targetNullWords.find(j) != targetNullWords.end())
	if (coverageVector.GetValue(j) == 0 && targetNullWords.find(j) != targetNullWords.end())
	{
		j1 = j;
		german = currF[j1-startIndex];
		english = "_INS_";
		generateOperations(startIndex, j1, 2 , coverageVector , english , german , targetNullWords , currF);
	}

	//print();
}

void osmHypothesis :: print()
{
	for (int i = 0; i< operations.size(); i++)
	{
		cerr<<operations[i]<<" ";

	}

	cerr<<endl<<endl;
	
	cerr<<"Operation Probability "<<opProb<<endl;
	cerr<<"Gap Count "<<gapCount<<endl;
	cerr<<"Open Gap Count "<<openGapCount<<endl;
	cerr<<"Gap Width "<<gapWidth<<endl;
	cerr<<"Deletion Count "<<deletionCount<<endl;

	cerr<<"_______________"<<endl;
}

int osmHypothesis :: closestGap(map <int,string> gap, int j1, int & gp)
{

	int dist=1172;
	int value=-1;
	int temp=0;
	gp=0;
	int opGap=0;

	map <int,string> :: iterator iter;

	iter=gap.end();
		
		do
		{
			iter--;
			//cout<<"Trapped "<<iter->first<<endl;

		   	if(iter->first==j1 && iter->second== "Unfilled")
			{
				opGap++;
				gp = opGap;
				return j1;

			}
		
		   	if(iter->second =="Unfilled")
		   	{
				opGap++;
				temp = iter->first - j1;

				if(temp<0)
			 	temp=temp * -1;
			
				if(dist>temp && iter->first < j1)
				{
					dist=temp;
					value=iter->first;
					gp=opGap;
				}
		  	}
			 

		}
		while(iter!=gap.begin());

	return value;
}



int osmHypothesis :: getOpenGaps()
{
	map <int,string> :: iterator iter;

	int nd = 0;
	for (iter = gap.begin(); iter!=gap.end(); iter++)
	{
		if(iter->second == "Unfilled")
		 nd++;
	}

	return nd;

}

} // namespace

