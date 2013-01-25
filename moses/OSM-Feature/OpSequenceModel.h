#pragma once

#include <string>
#include <map>
#include <vector>
#include "moses/FeatureFunction.h"
#include "Ngram.h"
#include "moses/Manager.h"
#include "moses/OSM-Feature/osmHyp.h"
#include "moses/OSM-Feature/SRILM-API.h"

namespace Moses
{

class OpSequenceModel : public StatefulFeatureFunction
{
public:

	//LM *LanguageModel;
	Api * ptrOp;
	int lmOrder;

	OpSequenceModel();

	void readLanguageModel(const char *);
	void Load(const std::string &osmFeatureFile, const std::string &operationLM , int orderVal);

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

	std::vector < std::pair < std::set <int> , std::set <int> > > ceptsInPhrase;
	std::set <int> targetNullWords;



};


} // namespace
