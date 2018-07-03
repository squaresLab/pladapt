#include <darteval/SASSAdaptationManager.h>
#include <pladapt/EnvironmentDTMCPartitioned.h>
#include <pladapt/UtilityFunction.h>
#include <pladapt/Configuration.h>
#include <pladapt/ConfigurationManager.h>
//#include <boost/process.hpp>

using namespace std;

//namespace bp = boost::process;

namespace pladapt {

	TacticList SASSAdaptationManager::evaluate(const Configuration& currentConfigObj, const EnvironmentDTMCPartitioned& envDTMC,
        const UtilityFunction& utilityFunction, unsigned horizon){
		set<string> str { "GoTight" };
    	return str;
     }
        void initialize(std::shared_ptr<const ConfigurationManager> configMgr, const YAML::Node& params){
		cout << "initializing SASSAdaptationManager";
	}
	SASSAdaptationManager::~SASSAdaptationManager() {
	
	}

}
