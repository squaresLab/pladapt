#include <pladapt/AdaptationManager.h>
#include <pladapt/EnvironmentDTMCPartitioned.h>
#include <pladapt/UtilityFunction.h>
#include <pladapt/Configuration.h>
#include <pladapt/ConfigurationManager.h>

using namespace std;

namespace pladapt {

class SASSAdaptationManager : public AdaptationManager
{
    public:
	    TacticList evaluate(const Configuration& currentConfigObj, const EnvironmentDTMCPartitioned& envDTMC,
    		const UtilityFunction& utilityFunction, unsigned horizon);
	    void initialize(std::shared_ptr<const ConfigurationManager> configMgr, const YAML::Node& params);
	    ~SASSAdaptationManager();

};

}
