#include <fstream>
#include "OpSequenceModel.h"
#include "osmHyp.h"
#include "util/check.hh"
#include "moses/Util.h"

using namespace std;

namespace Moses
{

OpSequenceModel::OpSequenceModel()
:StatefulFeatureFunction("OpSequenceModel", 5 )
{

	LanguageModel = NULL;
}

void OpSequenceModel :: readLanguageModel( char *lmFile,int order)
{
	
  setlocale(LC_CTYPE, "");
  setlocale(LC_COLLATE, "");

  Vocab *vocab = new Vocab;
   vocab->unkIsWord() = true; /* vocabulary contains unknown word tag */

  LanguageModel = new Ngram( *vocab,order );
  assert(LanguageModel != 0);
  // LanguageModel->debugme(0);

  File file( lmFile, "r" );
  if (!LanguageModel->read( file )) {
    cerr << "format error in lm file\n";
    exit(1);
  }

  file.close();
}


void OpSequenceModel::Load(const std::string &osmFeatureFile, const std::string &operationLM)
{
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
}

FFState* OpSequenceModel::Evaluate(
    const Hypothesis& cur_hypo,
    const FFState* prev_state,
    ScoreComponentCollection* accumulator) const
{
  //cur_hypo.GetCurrTargetPhrase();
  //const Sentence &sentence = static_cast<const Sentence&>(curr_hypo.GetManager().GetSource());

  cerr << "OpSequenceModel::Evaluate()" << endl;
  return NULL;
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
