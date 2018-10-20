#include <darteval/ExtendedDartAdaptationManager.h>
#include <dartam/DartAdaptationManager.h>
#include <dartam/DartConfigurationManager.h>
#include <pladapt/Utils.h>
#include <darteval/SASSAdaptationManager.h>
#include <pladapt/SDPRAAdaptationManager.h>
#include <pladapt/PMCRAAdaptationManager.h>
#include <dartam/DartPMCHelper.h>
#include <math.h>

// include headers that implement a archive in simple text format
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <fstream>
// include this header to serialize vectors
#include <boost/serialization/vector.hpp>

#if DART_USE_CE
#include <pladapt/CEAdaptationManager.h>
#endif

using namespace std;

namespace dart {
namespace am2 {

const string ADAPT_MGR_SDP = "sdp";
const string ADAPT_MGR_SDPRA = "sdpra";
const string ADAPT_MGR_PMC = "pmc";
const string ADAPT_MGR_SASS = "sass";
#if DART_USE_CE
const string ADAPT_MGR_CE = "ce";
#endif

void DartAdaptationManager::instantiateAdaptationMgr(const Params& params) {
	cout << "Initializing adapt mgr...";

	// initialize config manager
	configManager = std::make_shared<DartConfigurationManager>(
			params.configurationSpace.ALTITUDE_LEVELS,
			pladapt::tacticLatencyToPeriods(params.tactics.changeAltitudeLatency, params.adaptationManager.adaptationPeriod),
			params.configurationSpace.hasEcm, params.adaptationManager.twoLevelTactics);

	auto changeAltitudePeriods = pladapt::tacticLatencyToPeriods(params.tactics.changeAltitudeLatency, params.adaptationManager.adaptationPeriod);

	// instantiate and initialize appropriate adapt mgr
	if (params.adaptationManager.mgr == ADAPT_MGR_PMC) {
	    YAML::Node amParams;
	    amParams[pladapt::PMCAdaptationManager::NO_LATENCY] = (params.adaptationManager.nonLatencyAware || changeAltitudePeriods == 0);
	    amParams[pladapt::PMCAdaptationManager::TEMPLATE_PATH] = params.adaptationManager.PRISM_TEMPLATE;
	    amParams[pladapt::PMCRAAdaptationManager::PROBABILITY_BOUND] = params.adaptationManager.probabilityBound;

	    auto pAdaptMgr = new pladapt::PMCAdaptationManager;
		pAdaptMgr->initialize(configManager, amParams, std::make_shared<const DartPMCHelper>(params));
		adaptMgr.reset(pAdaptMgr);
	} else { // SDP or derived
	    YAML::Node amParams;
	    amParams[pladapt::SDPAdaptationManager::NO_LATENCY] = (params.adaptationManager.nonLatencyAware || changeAltitudePeriods == 0);
	    amParams[pladapt::SDPAdaptationManager::REACH_OPTIONS] = "-c ConfigDart2";
		amParams[pladapt::SDPAdaptationManager::REACH_PATH] = params.adaptationManager.REACH_PATH;
		if (!params.adaptationManager.REACH_PREFIX.empty()) {
			amParams[pladapt::SDPAdaptationManager::REACH_PREFIX] = params.adaptationManager.REACH_PREFIX;
		}
	    if (params.adaptationManager.nonLatencyAware && changeAltitudePeriods > 0) {
	    	amParams[pladapt::SDPAdaptationManager::REACH_MODEL] = params.adaptationManager.REACH_MODEL + "-nla";
	    } else {
	    	amParams[pladapt::SDPAdaptationManager::REACH_MODEL] = params.adaptationManager.REACH_MODEL;
	    }

	    stringstream scope;
	    scope << "A=" << params.configurationSpace.ALTITUDE_LEVELS;
	    scope << " F=2";
	    if (changeAltitudePeriods > 0) {
	        scope << " TPIA#=" <<  changeAltitudePeriods << " TPDA#=" <<  changeAltitudePeriods;
	        if (params.adaptationManager.twoLevelTactics) {
	        	scope << " TPIA2#=" <<  changeAltitudePeriods << " TPDA2#=" <<  changeAltitudePeriods;
	        }
	    }

	    amParams[pladapt::SDPAdaptationManager::REACH_SCOPE] = scope.str();

	    amParams[pladapt::SDPRAAdaptationManager::PROBABILITY_BOUND] = params.adaptationManager.probabilityBound;

#if DART_USE_CE
	    if (params.adaptationManager.mgr == ADAPT_MGR_CE) {
	    	amParams[pladapt::CEAdaptationManager::CE_INCREMENTAL] = params.adaptationManager.ce_incremental;
	    	amParams[pladapt::CEAdaptationManager::CE_HINT_WEIGHT] = params.adaptationManager.ce_hintWeight;
	    	amParams[pladapt::CEAdaptationManager::CE_SAMPLES] = params.adaptationManager.ce_samples;
			amParams[pladapt::CEAdaptationManager::CE_ALPHA] = params.adaptationManager.ce_alpha;
			amParams[pladapt::CEAdaptationManager::CE_PRECISION] = params.adaptationManager.ce_precision;
			amParams[pladapt::CEAdaptationManager::CE_MAX_ITERATIONS] = params.adaptationManager.ce_maxIterations;

			auto pAdaptMgr = new pladapt::CEAdaptationManager;
			pAdaptMgr->initialize(configManager, amParams);
			adaptMgr.reset(pAdaptMgr);
		} else
#endif
		if (params.adaptationManager.mgr == ADAPT_MGR_SDPRA) {
			auto pAdaptMgr = new pladapt::SDPRAAdaptationManager;
			pAdaptMgr->initialize(configManager, amParams);
			adaptMgr.reset(pAdaptMgr);
		} else if (params.adaptationManager.mgr == ADAPT_MGR_SDP) {
			auto pAdaptMgr = new pladapt::SDPAdaptationManager;
			pAdaptMgr->initialize(configManager, amParams);
			adaptMgr.reset(pAdaptMgr);
		}else if (params.adaptationManager.mgr == ADAPT_MGR_SASS){
			auto pAdaptMgr = new pladapt::SASSAdaptationManager;
			//pAdaptMgr->initialize(configManager, amParams);
			adaptMgr.reset(pAdaptMgr);
		} else {
			ostringstream msg;
			msg << "Error: adaptation manager ";
			msg << params.adaptationManager.mgr;
			msg << " not supported.";
			throw std::invalid_argument(msg.str());
		}
	}

	cout << "done" << endl;
}

static std::string exec(const char* cmd) {
    		std::array<char, 128> buffer;
    		std::string result;
    		std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    		if (!pipe) throw std::runtime_error("popen() failed!");
    		while (!feof(pipe.get())) {
        		if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            		result += buffer.data();
    		}
    		return result;
	}

pladapt::TacticList DartAdaptationManager::decideAdaptation(
		const DartMonitoringInfo& monitoringInfo) {

	/* update environment */
        // this is type EnvironmentMonitor
	pEnvThreatMonitor->update(monitoringInfo.threatSensing);
	pEnvTargetMonitor->update(monitoringInfo.targetSensing);

	/* build env model with information collected so far */
	Route senseRoute(monitoringInfo.position, monitoringInfo.directionX, monitoringInfo.directionY, params.adaptationManager.HORIZON);
	DartDTMCEnvironment threatDTMC(*pEnvThreatMonitor, senseRoute, params.adaptationManager.distributionApproximation);
	DartDTMCEnvironment targetDTMC(*pEnvTargetMonitor, senseRoute, params.adaptationManager.distributionApproximation);
	pladapt::EnvironmentDTMCPartitioned jointEnv = pladapt::EnvironmentDTMCPartitioned::createJointDTMC(threatDTMC, targetDTMC);

        // we must serialize the system state, aka monitoring ingo
        // and we need to pass the true enviroment state somehow
        // create and open a character archive for output
        std::ofstream ofs("monitoring.ser");

        // save data to archive
        {
            boost::archive::text_oarchive oa(ofs);
            // write class instance to archive
            oa << monitoringInfo;
            // archive and stream closed when destructors are called
        }
        // ... some time later restore the class instance to its orginal state
        DartMonitoringInfo newm;
        {
            // create and open an archive for input
            std::ifstream ifs("monitoring.ser");
            boost::archive::text_iarchive ia(ifs);
            // read class state from archive
            ia >> newm;
            // archive and stream closed when destructors are called
        }
        
        cout << exec("./runSASS.sh");
        
	/* make adaptation decision */
	//adaptMgr->setDebug(monitoringInfo.position.x == 4);
	return adaptMgr->evaluate(convertToDiscreteConfiguration(monitoringInfo), jointEnv, *pUtilityFunction, params.adaptationManager.HORIZON);
}

} /* namespace am2 */
} /* namespace dart */
