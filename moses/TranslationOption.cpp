// $Id$
// vim:tabstop=2

/***********************************************************************
Moses - factored phrase-based language decoder
Copyright (C) 2006 University of Edinburgh

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
***********************************************************************/

#include "TranslationOption.h"
#include "WordsBitmap.h"
#include "moses/TranslationModel/PhraseDictionaryMemory.h"
#include "GenerationDictionary.h"
#include "LMList.h"
#include "LexicalReordering.h"
#include "StaticData.h"
#include "InputType.h"
#include "OSM-Feature/OpSequenceModel.h"
using namespace std;

namespace Moses
{

//TODO this should be a factory function!
TranslationOption::TranslationOption(const WordsRange &wordsRange
                                     , const TargetPhrase &targetPhrase
                                     , const InputType &inputType)
  : m_targetPhrase(targetPhrase)
  , m_sourceWordsRange(wordsRange)
  , m_scoreBreakdown(targetPhrase.GetScoreBreakdown())
{}

//TODO this should be a factory function!
TranslationOption::TranslationOption(const WordsRange &wordsRange
                                     , const TargetPhrase &targetPhrase
                                     , const InputType &inputType
                                     , const UnknownWordPenaltyProducer* up)
  : m_targetPhrase(targetPhrase)
  , m_sourceWordsRange	(wordsRange)
  , m_futureScore(0)
{
  if (up) {
		const ScoreProducer *scoreProducer = (const ScoreProducer *)up; // not sure why none of the c++ cast works
		vector<float> score(1);
		score[0] = FloorScore(-numeric_limits<float>::infinity());
		m_scoreBreakdown.Assign(scoreProducer, score);
	}
}

TranslationOption::TranslationOption(const TranslationOption &copy, const WordsRange &sourceWordsRange)
  : m_targetPhrase(copy.m_targetPhrase)
//, m_sourcePhrase(new Phrase(*copy.m_sourcePhrase)) // TODO use when confusion network trans opt for confusion net properly implemented
  , m_sourceWordsRange(sourceWordsRange)
  , m_futureScore(copy.m_futureScore)
  , m_scoreBreakdown(copy.m_scoreBreakdown)
  , m_cachedScores(copy.m_cachedScores)
{}

void TranslationOption::MergeNewFeatures(const Phrase& phrase, const ScoreComponentCollection& score, const std::vector<FactorType>& featuresToAdd)
{
  CHECK(phrase.GetSize() == m_targetPhrase.GetSize());
  if (featuresToAdd.size() == 1) {
    m_targetPhrase.MergeFactors(phrase, featuresToAdd[0]);
  } else if (featuresToAdd.empty()) {
    /* features already there, just update score */
  } else {
    m_targetPhrase.MergeFactors(phrase, featuresToAdd);
  }
  m_scoreBreakdown.PlusEquals(score);
}

bool TranslationOption::IsCompatible(const Phrase& phrase, const std::vector<FactorType>& featuresToCheck) const
{
  if (featuresToCheck.size() == 1) {
    return m_targetPhrase.IsCompatible(phrase, featuresToCheck[0]);
  } else if (featuresToCheck.empty()) {
    return true;
    /* features already there, just update score */
  } else {
    return m_targetPhrase.IsCompatible(phrase, featuresToCheck);
  }
}

bool TranslationOption::Overlap(const Hypothesis &hypothesis) const
{
  const WordsBitmap &bitmap = hypothesis.GetWordsBitmap();
  return bitmap.Overlap(GetSourceWordsRange());
}

void TranslationOption::CalcScore(const TranslationSystem* system)
{
  // LM scores
  float ngramScore = 0;
  float retFullScore = 0;
  float oovScore = 0;

  const LMList &allLM = system->GetLanguageModels();

  allLM.CalcScore(GetTargetPhrase(), retFullScore, ngramScore, oovScore, &m_scoreBreakdown);

  size_t phraseSize = GetTargetPhrase().GetSize();
  
/*
  // future score

  if (m_osmScore == 0 && GetTargetPhrase().GetWord(0).GetFactor(1) != NULL) // Getting OSM - Future cost for the unknown word ...
  {
	    CalcOSMFutureScore(system, m_targetPhrase, m_targetPhrase.GetSourcePhrase());
  }

  m_futureScore = retFullScore - ngramScore + oovScore
                  + m_scoreBreakdown.InnerProduct(StaticData::Instance().GetAllWeights()) - phraseSize *
                  system->GetWeightWordPenalty() + m_osmScore;

*/

m_futureScore = retFullScore - ngramScore + oovScore
                  + m_scoreBreakdown.InnerProduct(StaticData::Instance().GetAllWeights()) - phraseSize *
                  system->GetWeightWordPenalty();


//cerr<<"Yahan "<<GetSourcePhrase()->GetWord(0)<<" "<<GetTargetPhrase().GetWord(0).GetFactor(1)<<" "<<m_osmScore<<endl;

}

void TranslationOption::CalcOSMFutureScore(const TranslationSystem* system, const Phrase &source, const Phrase &target)
{

 // cerr<<" range=" << m_sourceWordsRange << " source=" <<source<< " target= "<<target<<endl;
  m_osmScore = 0;
  const ScoreComponentCollection &allWeights = StaticData::Instance().GetAllWeights();
  const vector<const StatefulFeatureFunction*> &ffs = system->GetStatefulFeatureFunctions();
  for (size_t i = 0; i < ffs.size(); ++i) {
    const StatefulFeatureFunction *ff = ffs[i];
    const OpSequenceModel *osmFF = dynamic_cast<const OpSequenceModel*>(ff);
    if (osmFF) {
      vector<float> scores = osmFF->GetFutureScores(source, target);
      std::vector<float> weights = allWeights.GetScoresForProducer(osmFF);
      CHECK(scores.size() == weights.size());

      for (size_t ind = 0; ind < scores.size(); ++ind) {
    	  m_osmScore += scores[ind] * weights[ind];
    //	  cerr<<m_osmScore<<" "<<scores[ind]<<" "<<weights[ind]<<endl;
      }

      //m_osmScore = scores[0] * weights[0];
    }
  }

//   cerr<<"I was inside this function "<<m_osmScore<<endl;
}

TO_STRING_BODY(TranslationOption);

// friend
ostream& operator<<(ostream& out, const TranslationOption& possibleTranslation)
{
  out << possibleTranslation.GetTargetPhrase()
      << " c=" << possibleTranslation.GetFutureScore()
      << " [" << possibleTranslation.GetSourceWordsRange() << "]"
      << possibleTranslation.GetScoreBreakdown();
  return out;
}

void TranslationOption::CacheScores(const ScoreProducer &producer, const Scores &score)
{
  m_cachedScores[&producer] = score;
}

}


