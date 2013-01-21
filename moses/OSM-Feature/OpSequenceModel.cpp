#include <fstream>
#include "OpSequenceModel.h"
#include "util/check.hh"
#include "moses/Util.h"

using namespace std;

namespace Moses
{

OpSequenceModel::OpSequenceModel()
:StatefulFeatureFunction("OpSequenceModel", 1 )
{

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
  abort();
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
  abort();
}

std::string OpSequenceModel::GetScoreProducerWeightShortName(unsigned idx) const
{
  abort();
}

} // namespace
