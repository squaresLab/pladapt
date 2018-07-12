/*******************************************************************************
 * PLA Adaptation Manager
 *
 * Copyright 2017 Carnegie Mellon University. All Rights Reserved.
 *
 * NO WARRANTY. THIS CARNEGIE MELLON UNIVERSITY AND SOFTWARE ENGINEERING
 * INSTITUTE MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. CARNEGIE MELLON
 * UNIVERSITY MAKES NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR IMPLIED, AS
 * TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF FITNESS FOR PURPOSE
 * OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS OBTAINED FROM USE OF THE
 * MATERIAL. CARNEGIE MELLON UNIVERSITY DOES NOT MAKE ANY WARRANTY OF ANY KIND
 * WITH RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT INFRINGEMENT.
 *
 * Released under a BSD-style license, please see license.txt or contact
 * permission@sei.cmu.edu for full terms.
 *
 * [DISTRIBUTION STATEMENT A] This material has been approved for public release
 * and unlimited distribution. Please see Copyright notice for non-US Government
 * use and distribution.
 ******************************************************************************/

#ifndef HYBRIDADAPTATIONMANAGER_H_
#define HYBRIDADAPTATIONMANAGER_H_

#include "pladapt/AdaptationManager.h"
#include <pladapt/ConfigurationManager.h>
#include <pladapt/PRISMWrapper.h>
#include <dartam/PlanDB.h>
#include <dartam/DartPMCHelper.h>
#include <memory>
#include <yaml-cpp/yaml.h>

namespace dart {
namespace am2 {

/**
 * Adaptation manager using Probabilistic Model Checking approach
 */
class HybridAdaptationManager : public pladapt::AdaptationManager
{
public:

HybridAdaptationManager(void);

/**
 * params must include:
 *   TEMPLATE_PATH: path to the PRISM template file, not including the
 *      extension ".prism"
 *   NO_LATENCY: boolean, true if no tactic has latency. In that case,
 *      "-nl" will be appended the TEMPLATE_PATH before loading it
 */
virtual void initialize(std::shared_ptr<const pladapt::ConfigurationManager> configMgr, const YAML::Node& params,
                        std::shared_ptr<const DartPMCHelper> helper);
virtual pladapt::TacticList evaluate(const pladapt::Configuration& currentConfigObj, const pladapt::EnvironmentDTMCPartitioned& envDTMC,
                            const pladapt::UtilityFunction& utilityFunction, unsigned horizon);

std::string generateEnvironmentDTMC(const pladapt::EnvironmentDTMCPartitioned& envDTMC);

static const char* NO_LATENCY;
static const char* TEMPLATE_PATH;
virtual ~HybridAdaptationManager();

protected:
std::shared_ptr<const pladapt::ConfigurationManager> pConfigMgr;
std::shared_ptr<const pladapt::PMCHelper> pMcHelper;
pladapt::EnvironmentDTMCPartitioned savedDTMC;
pladapt::PRISMWrapper deliberativeWrapper;
PlanDB plan;
unsigned planStartTime;

YAML::Node params;
};

} /* am2 */
} /* dart */

#endif
