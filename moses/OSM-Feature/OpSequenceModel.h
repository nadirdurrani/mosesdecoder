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

	void Load(const std::string &filePath);

protected:
	typedef std::pair<std::string, std::string> ParallelPhrase;
	typedef std::vector<float> Scores;
	std::map<ParallelPhrase, Scores> m_coll;
};


} // namespace
