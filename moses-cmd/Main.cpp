// $Id: MainMT.cpp 3045 2010-04-05 13:07:29Z hieuhoang1972 $

/***********************************************************************
Moses - factored phrase-based language decoder
Copyright (C) 2009 University of Edinburgh

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

/**
 * Moses main, for single-threaded and multi-threaded.
 **/

#include <exception>
#include <fstream>
#include <sstream>
#include <vector>

#include "util/usage.hh"

#ifdef WIN32
// Include Visual Leak Detector
//#include <vld.h>
#endif

#include "TranslationAnalysis.h"
#include "IOWrapper.h"
#include "mbr.h"

#include "moses/Hypothesis.h"
#include "moses/Manager.h"
#include "moses/StaticData.h"
#include "moses/Util.h"
#include "moses/Timer.h"
#include "moses/ThreadPool.h"
#include "moses/OutputCollector.h"

#ifdef HAVE_PROTOBUF
#include "hypergraph.pb.h"
#endif

using namespace std;
using namespace Moses;
using namespace MosesCmd;

namespace MosesCmd
{
// output floats with three significant digits
static const size_t PRECISION = 3;

/** Enforce rounding */
void fix(std::ostream& stream, size_t size)
{
  stream.setf(std::ios::fixed);
  stream.precision(size);
}

/** Translates a sentence.
  * - calls the search (Manager)
  * - applies the decision rule
  * - outputs best translation and additional reporting
  **/
class TranslationTask : public Task
{

public:

  TranslationTask(size_t lineNumber,
                  InputType* source, OutputCollector* outputCollector, OutputCollector* nbestCollector,
                  OutputCollector* latticeSamplesCollector,
                  OutputCollector* wordGraphCollector, OutputCollector* searchGraphCollector,
                  OutputCollector* detailedTranslationCollector,
                  OutputCollector* alignmentInfoCollector,
                  OutputCollector* unknownsCollector) :
    m_source(source), m_lineNumber(lineNumber),
    m_outputCollector(outputCollector), m_nbestCollector(nbestCollector),
    m_latticeSamplesCollector(latticeSamplesCollector),
    m_wordGraphCollector(wordGraphCollector), m_searchGraphCollector(searchGraphCollector),
    m_detailedTranslationCollector(detailedTranslationCollector),
    m_alignmentInfoCollector(alignmentInfoCollector),
    m_unknownsCollector(unknownsCollector) {}

	/** Translate one sentence
   * gets called by main function implemented at end of this source file */
  void Run() {

    // report thread number
#if defined(WITH_THREADS) && defined(BOOST_HAS_PTHREADS)
    TRACE_ERR("Translating line " << m_lineNumber << "  in thread id " << pthread_self() << std::endl);
#endif

    Timer translationTime;
    translationTime.start();
    // shorthand for "global data"
    const StaticData &staticData = StaticData::Instance();
    // input sentence
    Sentence sentence();
    // set translation system
    const TranslationSystem& system = staticData.GetTranslationSystem(TranslationSystem::DEFAULT);

    // execute the translation
    // note: this executes the search, resulting in a search graph
    //       we still need to apply the decision rule (MAP, MBR, ...)
    Manager manager(m_lineNumber, *m_source,staticData.GetSearchAlgorithm(), &system);
    manager.ProcessSentence();

    // output word graph
    if (m_wordGraphCollector) {
      ostringstream out;
      fix(out,PRECISION);
      manager.GetWordGraph(m_lineNumber, out);
      m_wordGraphCollector->Write(m_lineNumber, out.str());
    }

    // output search graph
    if (m_searchGraphCollector) {
      ostringstream out;
      fix(out,PRECISION);
      manager.OutputSearchGraph(m_lineNumber, out);
      m_searchGraphCollector->Write(m_lineNumber, out.str());

#ifdef HAVE_PROTOBUF
      if (staticData.GetOutputSearchGraphPB()) {
        ostringstream sfn;
        sfn << staticData.GetParam("output-search-graph-pb")[0] << '/' << m_lineNumber << ".pb" << ends;
        string fn = sfn.str();
        VERBOSE(2, "Writing search graph to " << fn << endl);
        fstream output(fn.c_str(), ios::trunc | ios::binary | ios::out);
        manager.SerializeSearchGraphPB(m_lineNumber, output);
      }
#endif
    }		

    // apply decision rule and output best translation(s)
    if (m_outputCollector) {
      ostringstream out;
      ostringstream debug;
      fix(debug,PRECISION);

      // all derivations - send them to debug stream
      if (staticData.PrintAllDerivations()) {
        manager.PrintAllDerivations(m_lineNumber, debug);
      }

      // MAP decoding: best hypothesis
      const Hypothesis* bestHypo = NULL;

      if (!staticData.UseMBR()) 
			{
        bestHypo = manager.GetBestHypothesis();
        if (bestHypo) {
        	bestHypo->outputOSMSequence();
          if (staticData.IsPathRecoveryEnabled()) {
            OutputInput(out, bestHypo);
            out << "||| ";
          }
          OutputBestSurface(
            out,
            bestHypo,
            staticData.GetOutputFactorOrder(),
            staticData.GetReportSegmentation(),
            staticData.GetReportAllFactors());
          OutputAlignment(m_alignmentInfoCollector, m_lineNumber, bestHypo);
          IFVERBOSE(1) {
            debug << "BEST TRANSLATION: " << *bestHypo << endl;
          }
        }
        out << endl;
			}

      // MBR decoding (n-best MBR, lattice MBR, consensus)
      else 
			{
        // we first need the n-best translations
        size_t nBestSize = staticData.GetMBRSize();
        if (nBestSize <= 0) {
          cerr << "ERROR: negative size for number of MBR candidate translations not allowed (option mbr-size)" << endl;
          exit(1);
        }
        TrellisPathList nBestList;
        manager.CalcNBest(nBestSize, nBestList,true);
        VERBOSE(2,"size of n-best: " << nBestList.GetSize() << " (" << nBestSize << ")" << endl);
        IFVERBOSE(2) {
          PrintUserTime("calculated n-best list for (L)MBR decoding");
        }

        // lattice MBR
        if (staticData.UseLatticeMBR()) {
          if (m_nbestCollector) {
            //lattice mbr nbest
            vector<LatticeMBRSolution> solutions;
            size_t n  = min(nBestSize, staticData.GetNBestSize());
            getLatticeMBRNBest(manager,nBestList,solutions,n);
            ostringstream out;
            OutputLatticeMBRNBest(out, solutions,m_lineNumber);
            m_nbestCollector->Write(m_lineNumber, out.str());
          } else {
            //Lattice MBR decoding
            vector<Word> mbrBestHypo = doLatticeMBR(manager,nBestList);
            OutputBestHypo(mbrBestHypo, m_lineNumber, staticData.GetReportSegmentation(),
                           staticData.GetReportAllFactors(),out);
            IFVERBOSE(2) {
              PrintUserTime("finished Lattice MBR decoding");
            }
          }
        }

        // consensus decoding
				else if (staticData.UseConsensusDecoding()) {
          const TrellisPath &conBestHypo = doConsensusDecoding(manager,nBestList);
          OutputBestHypo(conBestHypo, m_lineNumber,
                         staticData.GetReportSegmentation(),
                         staticData.GetReportAllFactors(),out);
          OutputAlignment(m_alignmentInfoCollector, m_lineNumber, conBestHypo);
          IFVERBOSE(2) {
            PrintUserTime("finished Consensus decoding");
          }
				}
				
        // n-best MBR decoding
        else {
          const Moses::TrellisPath &mbrBestHypo = doMBR(nBestList);
          OutputBestHypo(mbrBestHypo, m_lineNumber,
                         staticData.GetReportSegmentation(),
                         staticData.GetReportAllFactors(),out);
          OutputAlignment(m_alignmentInfoCollector, m_lineNumber, mbrBestHypo);
          IFVERBOSE(2) {
            PrintUserTime("finished MBR decoding");
          }
        }
      }

      // report best translation to output collector
      m_outputCollector->Write(m_lineNumber,out.str(),debug.str());
    }

    // output n-best list
    if (m_nbestCollector && !staticData.UseLatticeMBR()) {
      TrellisPathList nBestList;
      ostringstream out;
      manager.CalcNBest(staticData.GetNBestSize(), nBestList,staticData.GetDistinctNBest());
      OutputNBest(out, nBestList, staticData.GetOutputFactorOrder(), manager.GetTranslationSystem(), m_lineNumber,
		  staticData.GetReportSegmentation());
      m_nbestCollector->Write(m_lineNumber, out.str());
    }

    //lattice samples
    if (m_latticeSamplesCollector) {
      TrellisPathList latticeSamples;
      ostringstream out;
      manager.CalcLatticeSamples(staticData.GetLatticeSamplesSize(), latticeSamples);
      OutputNBest(out,latticeSamples, staticData.GetOutputFactorOrder(), manager.GetTranslationSystem(), m_lineNumber,
		  staticData.GetReportSegmentation());
      m_latticeSamplesCollector->Write(m_lineNumber, out.str());
    }

    // detailed translation reporting
    if (m_detailedTranslationCollector) {
      ostringstream out;
      fix(out,PRECISION);
      TranslationAnalysis::PrintTranslationAnalysis(manager.GetTranslationSystem(), out, manager.GetBestHypothesis());
      m_detailedTranslationCollector->Write(m_lineNumber,out.str());
    }

    //list of unknown words
    if (m_unknownsCollector) {
      const vector<Phrase*>& unknowns = manager.getSntTranslationOptions()->GetUnknownSources();
      ostringstream out;
      for (size_t i = 0; i < unknowns.size(); ++i) {
        out << *(unknowns[i]);
      }
      out << endl;
      m_unknownsCollector->Write(m_lineNumber, out.str());
    }

    // report additional statistics
    IFVERBOSE(2) {
      PrintUserTime("Sentence Decoding Time:");
    }
    manager.CalcDecoderStatistics();

    VERBOSE(1, "Line " << m_lineNumber << ": Translation took " << translationTime << " seconds total" << endl);
  }

  ~TranslationTask() {
    delete m_source;
  }

private:
  InputType* m_source;
  size_t m_lineNumber;
  OutputCollector* m_outputCollector;
  OutputCollector* m_nbestCollector;
  OutputCollector* m_latticeSamplesCollector;
  OutputCollector* m_wordGraphCollector;
  OutputCollector* m_searchGraphCollector;
  OutputCollector* m_detailedTranslationCollector;
  OutputCollector* m_alignmentInfoCollector;
  OutputCollector* m_unknownsCollector;
  std::ofstream *m_alignmentStream;


};

static void PrintFeatureWeight(const FeatureFunction* ff)
{
  size_t numScoreComps = ff->GetNumScoreComponents();
  if (numScoreComps != ScoreProducer::unlimited) {
    vector<float> values = StaticData::Instance().GetAllWeights().GetScoresForProducer(ff);
    for (size_t i = 0; i < numScoreComps; ++i) 
      cout << ff->GetScoreProducerDescription() <<  " "
           << ff->GetScoreProducerWeightShortName() << " "
           << values[i] << endl;
  }
  else {
  	if (ff->GetSparseProducerWeight() == 1)
  		cout << ff->GetScoreProducerDescription() << " " <<
  		ff->GetScoreProducerWeightShortName() << " sparse" <<  endl;
  	else
  		cout << ff->GetScoreProducerDescription() << " " <<
  		ff->GetScoreProducerWeightShortName() << " " << ff->GetSparseProducerWeight() << endl;
  }
}

static void ShowWeights()
{
  //TODO: Find a way of ensuring this order is synced with the nbest
  fix(cout,6);
  const StaticData& staticData = StaticData::Instance();
  const TranslationSystem& system = staticData.GetTranslationSystem(TranslationSystem::DEFAULT);
  const vector<const StatelessFeatureFunction*>& slf =system.GetStatelessFeatureFunctions();
  const vector<const StatefulFeatureFunction*>& sff = system.GetStatefulFeatureFunctions();
  for (size_t i = 0; i < sff.size(); ++i) {
    PrintFeatureWeight(sff[i]);
  }
  for (size_t i = 0; i < slf.size(); ++i) {
    if (slf[i]->GetScoreProducerWeightShortName() != "u" &&
          slf[i]->GetScoreProducerWeightShortName() != "tm" &&
          slf[i]->GetScoreProducerWeightShortName() != "I" &&
          slf[i]->GetScoreProducerWeightShortName() != "g")
    {
  	  PrintFeatureWeight(slf[i]);
    }
  }
  const vector<PhraseDictionaryFeature*>& pds = system.GetPhraseDictionaries();
  for( size_t i=0; i<pds.size(); i++ ) {
    PrintFeatureWeight(pds[i]);
  }
  const vector<GenerationDictionary*>& gds = system.GetGenerationDictionaries();
  for( size_t i=0; i<gds.size(); i++ ) {
    PrintFeatureWeight(gds[i]);
  }

}

} //namespace

/** main function of the command line version of the decoder **/
int main(int argc, char** argv)
{
  try {
  
#ifdef HAVE_PROTOBUF
    GOOGLE_PROTOBUF_VERIFY_VERSION;
#endif

    // echo command line, if verbose
    IFVERBOSE(1) {
      TRACE_ERR("command: ");
      for(int i=0; i<argc; ++i) TRACE_ERR(argv[i]<<" ");
      TRACE_ERR(endl);
    }

    // set number of significant decimals in output
    fix(cout,PRECISION);
    fix(cerr,PRECISION);

    // load all the settings into the Parameter class
    // (stores them as strings, or array of strings)
    Parameter* params = new Parameter();
    if (!params->LoadParam(argc,argv)) {
      exit(1);
    }


    // initialize all "global" variables, which are stored in StaticData
    // note: this also loads models such as the language model, etc.
    if (!StaticData::LoadDataStatic(params, argv[0])) {
      exit(1);
    }

    // setting "-show-weights" -> just dump out weights and exit
    if (params->isParamSpecified("show-weights")) {
      ShowWeights();
      exit(0);
    }

    // shorthand for accessing information in StaticData
    const StaticData& staticData = StaticData::Instance();


    //initialise random numbers
    srand(time(NULL));

    // set up read/writing class
    IOWrapper* ioWrapper = GetIOWrapper(staticData);
    if (!ioWrapper) {
      cerr << "Error; Failed to create IO object" << endl;
      exit(1);
    }

    // check on weights
    const ScoreComponentCollection& weights = staticData.GetAllWeights();
    IFVERBOSE(2) {
      TRACE_ERR("The global weight vector looks like this: ");
      TRACE_ERR(weights);
      TRACE_ERR("\n");
    }

    // initialize output streams
    // note: we can't just write to STDOUT or files
    // because multithreading may return sentences in shuffled order
    auto_ptr<OutputCollector> outputCollector; // for translations
    auto_ptr<OutputCollector> nbestCollector;  // for n-best lists
    auto_ptr<OutputCollector> latticeSamplesCollector; //for lattice samples
    auto_ptr<ofstream> nbestOut;
    auto_ptr<ofstream> latticeSamplesOut;
    size_t nbestSize = staticData.GetNBestSize();
    string nbestFile = staticData.GetNBestFilePath();
    bool output1best = true;
    if (nbestSize) {
      if (nbestFile == "-" || nbestFile == "/dev/stdout") {
        // nbest to stdout, no 1-best
        nbestCollector.reset(new OutputCollector());
        output1best = false;
      } else {
        // nbest to file, 1-best to stdout
        nbestOut.reset(new ofstream(nbestFile.c_str()));
        if (!nbestOut->good()) {
          TRACE_ERR("ERROR: Failed to open " << nbestFile << " for nbest lists" << endl);
          exit(1);
        }
        nbestCollector.reset(new OutputCollector(nbestOut.get()));
      }
    }
    size_t latticeSamplesSize = staticData.GetLatticeSamplesSize();
    string latticeSamplesFile = staticData.GetLatticeSamplesFilePath();
    if (latticeSamplesSize) {
      if (latticeSamplesFile == "-" || latticeSamplesFile == "/dev/stdout") {
        latticeSamplesCollector.reset(new OutputCollector());
        output1best = false;
      } else {
        latticeSamplesOut.reset(new ofstream(latticeSamplesFile.c_str()));
        if (!latticeSamplesOut->good()) {
          TRACE_ERR("ERROR: Failed to open " << latticeSamplesFile << " for lattice samples" << endl);
          exit(1);
        }
        latticeSamplesCollector.reset(new OutputCollector(latticeSamplesOut.get()));
      }
    }
    if (output1best) {
      outputCollector.reset(new OutputCollector());
    }
  
    // initialize stream for word graph (aka: output lattice)
    auto_ptr<OutputCollector> wordGraphCollector;
    if (staticData.GetOutputWordGraph()) {
      wordGraphCollector.reset(new OutputCollector(&(ioWrapper->GetOutputWordGraphStream())));
    }
  
    // initialize stream for search graph
    // note: this is essentially the same as above, but in a different format
    auto_ptr<OutputCollector> searchGraphCollector;
    if (staticData.GetOutputSearchGraph()) {
      searchGraphCollector.reset(new OutputCollector(&(ioWrapper->GetOutputSearchGraphStream())));
    }
  
    // initialize stram for details about the decoder run
    auto_ptr<OutputCollector> detailedTranslationCollector;
    if (staticData.IsDetailedTranslationReportingEnabled()) {
      detailedTranslationCollector.reset(new OutputCollector(&(ioWrapper->GetDetailedTranslationReportingStream())));
    }
  
    // initialize stram for word alignment between input and output
    auto_ptr<OutputCollector> alignmentInfoCollector;
    if (!staticData.GetAlignmentOutputFile().empty()) {
      alignmentInfoCollector.reset(new OutputCollector(ioWrapper->GetAlignmentOutputStream()));
    }

    //initialise stream for unknown (oov) words
    auto_ptr<OutputCollector> unknownsCollector;
    auto_ptr<ofstream> unknownsStream;
    if (!staticData.GetOutputUnknownsFile().empty()) {
      unknownsStream.reset(new ofstream(staticData.GetOutputUnknownsFile().c_str()));
      if (!unknownsStream->good()) {
        TRACE_ERR("Unable to open " << staticData.GetOutputUnknownsFile() << " for unknowns");
        exit(1);
      }
      unknownsCollector.reset(new OutputCollector(unknownsStream.get()));
    }
  
#ifdef WITH_THREADS
    ThreadPool pool(staticData.ThreadCount());
#endif
  
    // main loop over set of input sentences
    InputType* source = NULL;
    size_t lineCount = 0;
    while(ReadInput(*ioWrapper,staticData.GetInputType(),source)) {
      IFVERBOSE(1) {
        ResetUserTime();
      }
      // set up task of translating one sentence
      TranslationTask* task =
        new TranslationTask(lineCount,source, outputCollector.get(),
                            nbestCollector.get(),
                            latticeSamplesCollector.get(),
                            wordGraphCollector.get(),
                            searchGraphCollector.get(),
                            detailedTranslationCollector.get(),
                            alignmentInfoCollector.get(),
                            unknownsCollector.get() );
      // execute task
#ifdef WITH_THREADS
    pool.Submit(task);
#else
      task->Run();
      delete task;
#endif
  
      source = NULL; //make sure it doesn't get deleted
      ++lineCount;
    }
  
  // we are done, finishing up
#ifdef WITH_THREADS
    pool.Stop(true); //flush remaining jobs
#endif

  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  IFVERBOSE(1) util::PrintUsage(std::cerr);

#ifndef EXIT_RETURN
  //This avoids that destructors are called (it can take a long time)
  exit(EXIT_SUCCESS);
#else
  return EXIT_SUCCESS;
#endif
}
