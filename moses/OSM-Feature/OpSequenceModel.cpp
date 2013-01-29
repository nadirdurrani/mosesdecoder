#include <fstream>
#include "OpSequenceModel.h"
#include "osmHyp.h"
#include "util/check.hh"
#include "moses/Util.h"
#include "moses/OSM-Feature/osmHyp.h"



using namespace std;

namespace Moses
{

OpSequenceModel::OpSequenceModel()
:StatefulFeatureFunction("OpSequenceModel", 5 )
{



	//LanguageModel = NULL;
}

void OpSequenceModel :: readLanguageModel(const char *lmFile)
{


	ptrOp = new Api;
	ptrOp -> read_lm(lmFile,lmOrder);
/*
  setlocale(LC_CTYPE, "");
  setlocale(LC_COLLATE, "");

  Vocab *vocab = new Vocab;
   vocab->unkIsWord() = true; // vocabulary contains unknown word tag

  LanguageModel = new Ngram( *vocab,order );
  assert(LanguageModel != 0);
  // LanguageModel->debugme(0);

  File file( lmFile, "r" );
  if (!LanguageModel->read( file )) {
    cerr << "format error in lm file\n";
    exit(1);
  }

  file.close();
  */
}


void OpSequenceModel::Load(const std::string &osmFeatureFile, const std::string &operationLM , int orderVal)
{

   lmOrder= orderVal;
  //vector <string> input;
  ifstream sr (osmFeatureFile.c_str());
  char* tmp;

  CHECK(sr.is_open());

  string line;
  while (std::getline(sr, line))
  {
    std::vector<std::string> tokens;
    tokens = TokenizeMultiCharSeparator(line, "|||");
    CHECK(tokens.size() == 3);

    ParallelPhrase pp(tokens[0], tokens[1]);
    Scores scores = Tokenize<float>(tokens[2], " ");
    m_coll[pp] = scores;
  }

  readLanguageModel(operationLM.c_str());

}



FFState* OpSequenceModel::Evaluate(
    const Hypothesis& cur_hypo,
    const FFState* prev_state,
    ScoreComponentCollection* accumulator) const
{
  const TargetPhrase &target = cur_hypo.GetCurrTargetPhrase();
  const WordsBitmap &bitmap = cur_hypo.GetWordsBitmap();
  WordsBitmap myBitmap = bitmap;
  const Manager &manager = cur_hypo.GetManager();
  const InputType &source = manager.GetSource();
  const Sentence &sourceSentence = static_cast<const Sentence&>(source);
  osmHypothesis obj;
  vector <string> mySourcePhrase;
  vector <string> myTargetPhrase;
  vector<float> scores(5);


  //target.GetWord(0)

  /*cerr << target <<" --- "<<target.GetSourcePhrase()<< endl;  // English ...*/

  //cerr << align << endl;   // Alignments ...
  //cerr << cur_hypo.GetCurrSourceWordsRange() << endl;

  //cerr << source <<endl;

 // int a = sourceRange.GetStartPos();
 // cerr << source.GetWord(a);
  //cerr <<a<<endl;

  //const Sentence &sentence = static_cast<const Sentence&>(curr_hypo.GetManager().GetSource());


   const WordsRange & sourceRange = cur_hypo.GetCurrSourceWordsRange();
   int startIndex  = sourceRange.GetStartPos();
   int endIndex = sourceRange.GetEndPos();
   const AlignmentInfo &align = cur_hypo.GetCurrTargetPhrase().GetAlignTerm();
   osmState * statePtr;

   vector <int> alignments;



   AlignmentInfo::const_iterator iter;

      for (iter = align.begin(); iter != align.end(); ++iter) {
        //cerr << iter->first << "----" << iter->second << " ";
    	 alignments.push_back(iter->first);
    	 alignments.push_back(iter->second);
      }


   //cerr<<bitmap<<endl;
   //cerr<<startIndex<<" "<<endIndex<<endl;


  for (int i = startIndex; i <= endIndex; i++)
  {
	  myBitmap.SetValue(i,0); // resetting coverage of this phrase ...
	 mySourcePhrase.push_back(source.GetWord(i).GetFactor(0)->GetString());
	 // cerr<<mySourcePhrase[i]<<endl;
  }

  for (int i = 0; i < target.GetSize(); i++)
  {

	  if (target.GetWord(i).GetFactor(1) != NULL)
		  myTargetPhrase.push_back("_TRANS_SLF_");
	  else
		  myTargetPhrase.push_back(target.GetWord(i).GetFactor(0)->GetString());

	  //cerr<<myTargetPhrase[i]<<endl;
  }

  //cerr<<myBitmap<<endl;






  obj.setState(prev_state);
  obj.constructCepts(alignments,startIndex,endIndex);
  obj.setPhrases(mySourcePhrase , myTargetPhrase);
  obj.computeOSMFeature(startIndex,myBitmap,*ptrOp,lmOrder);
  obj.populateScores(scores);

/*
  if (bitmap.GetFirstGapPos() == NOT_FOUND)
  {

    int xx;
	 cerr<<bitmap<<endl;
	 int a = bitmap.GetFirstGapPos();
	 obj.print();
    cin>>xx;
  }
  */

/*
  vector<float> scores(5);
  scores[0] = 0.343423f;
  scores[1] = 1.343423f;
  scores[2] = 2.343423f;
  scores[3] = 3.343423f;
  scores[4] = 4.343423f;
  */

  accumulator->PlusEquals(this, scores);

  return obj.saveState();




  //return statePtr;
 // return NULL;
}

FFState* OpSequenceModel::EvaluateChart(
  const ChartHypothesis& /* cur_hypo */,
  int /* featureID - used to index the state in the previous hypotheses */,
  ScoreComponentCollection* accumulator) const
{
  abort();

}

const FFState* OpSequenceModel::EmptyHypothesisState(const InputType &input) const
{
  cerr << "OpSequenceModel::EmptyHypothesisState()" << endl;
  return new osmState();
}

std::string OpSequenceModel::GetScoreProducerWeightShortName(unsigned idx) const
{
  return "osm";
}

} // namespace
