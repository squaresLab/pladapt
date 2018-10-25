#include "SASSAdaptationManager.h"
#include <pladapt/EnvironmentDTMCPartitioned.h>
#include <pladapt/UtilityFunction.h>
#include <pladapt/Configuration.h>
#include <pladapt/ConfigurationManager.h>

using namespace std;

namespace pladapt {

	std::string exec(const char* cmd) {
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

	TacticList SASSAdaptationManager::evaluate(const Configuration& currentConfigObj, const EnvironmentDTMCPartitioned& envDTMC,
        const UtilityFunction& utilityFunction, unsigned horizon){

		//cout << pladapt::exec("echo testing");
		set<string> str { "DecAlt" };
    	return str;
     }
        void initialize(std::shared_ptr<const ConfigurationManager> configMgr, const YAML::Node& params){
		//cout << "initializing SASSAdaptationManager";
	}
	SASSAdaptationManager::~SASSAdaptationManager() {
	
	}

}
