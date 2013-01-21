#pragma once

#include <string>
#include <map>
#include "moses/FeatureFunction.h"

namespace Moses
{

class OpSequenceModel : public StatefulFeatureFunction
{
public:
	OpSequenceModel();

	void Load(const std::string &osmFeatureFile, const std::string &operationLM);

	FFState* Evaluate(
	    const Hypothesis& cur_hypo,
	    const FFState* prev_state,
	    ScoreComponentCollection* accumulator) const;

  virtual FFState* EvaluateChart(
    const ChartHypothesis& /* cur_hypo */,
    int /* featureID - used to index the state in the previous hypotheses */,
    ScoreComponentCollection* accumulator) const;

  virtual const FFState* EmptyHypothesisState(const InputType &input) const;

  virtual std::string GetScoreProducerWeightShortName(unsigned idx=0) const;

protected:
	typedef std::pair<std::string, std::string> ParallelPhrase;
	typedef std::vector<float> Scores;
	std::map<ParallelPhrase, Scores> m_coll;
};


} // namespace
