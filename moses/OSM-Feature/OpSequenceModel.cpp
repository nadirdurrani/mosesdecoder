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

void OpSequenceModel::Load(const std::string &filePath)
{
  //vector <string> input;
  ifstream sr (filePath.c_str());
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

} // namespace
