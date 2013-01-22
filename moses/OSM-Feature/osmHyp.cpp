#include "osmHyp.h"
#include <sstream>

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

void osmHypothesis :: calculateOSMProb(Api & ptrOp , vector <string> & history , int order)
{
	
	opProb = 0;
	vector <int> numbers;
	vector <int> context;
	int nonWordFlag  = 0;
	double temp;

	for (int i=0; i< operations.size(); i++)
		numbers.push_back(ptrOp.getLMID(const_cast <char *> (operations[i].c_str())));

	for (int i=0; i< history.size(); i++)
		context.push_back(ptrOp.getLMID(const_cast <char *> (history[i].c_str())));

	for (int i = 0; i<operations.size(); i++)
	{
		//cout<<opSeq[i]<<endl;
		context.push_back(numbers[i]);
		//cout<<"Context Size "<<context.size()<<endl;
		if (context.size() > order)
		{
			context.erase(context.begin());
		}
		
		temp = ptrOp.contextProbN(context,nonWordFlag);		   
		opProb = opProb + temp;
		
		 //cout<<temp<<" "<<opProb<<endl;
	
	}


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

void osmHypothesis :: generateOperations(int j1 , int contFlag , vector <int> & coverageVector , string english , string german , set <int> & targetNullWords , vector <string> & currF)
{
	
	int gFlag = 0;
	int gp = 0; 	
	int ans;
	
	if (english == "_INS_")	// Case : Delete the source word ...
	{
		operations.push_back("_INS_" + german);
		
		ans = firstOpenGap(coverageVector);
		
		if (ans != -1)
		 gapWidth += j - ans;
		
		coverageVector[j]=1;
		j++;

		if (E < j)
		   E = j;

		deletionCount++;
	}
	else
	{
		if ( j < j1) // j1 is the index of the source word we are about to generate ...
		{
			if(coverageVector[j]==0) // if source word at j is not genrated yet ...
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
			if(j < E && coverageVector[j]==0)
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

			ans = firstOpenGap(coverageVector);
		
			if (ans != -1)
		 		gapWidth += j - ans;

		}
		else
		{
			operations.push_back("_CONT_CEPT_");
		}

		coverageVector[j]=1;
		j+=1;
		
		if(E<j)
		  E=j;
	}

	if (gFlag > 0)
		gapCount++;

	openGapCount += getOpenGaps();

	if (coverageVector[j] == 0 && targetNullWords.find(j) != targetNullWords.end())
	{
		j1 = j;
		german = currF[j1];
		english = "_INS_";
		generateOperations(j1, 0 , coverageVector , english , german , targetNullWords , currF);
	}

	//print();
}

void osmHypothesis :: print()
{
	for (int i = 0; i< operations.size(); i++)
	{
		cout<<operations[i]<<" ";

	}

	cout<<endl<<endl;
	
	cout<<"Operation Probability "<<opProb<<endl;
	cout<<"Gap Count "<<gapCount<<endl;
	cout<<"Open Gap Count "<<openGapCount<<endl;
	cout<<"Gap Width "<<gapWidth<<endl;
	cout<<"Deletion Count "<<deletionCount<<endl;

	cout<<"_______________"<<endl;
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

