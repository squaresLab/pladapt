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
#include <dartam/HybridAdaptationManager.h>
#include <iostream>
#include <sstream>
#include <dartam/State.h>


using namespace std;

namespace dart {
namespace am2 {
const char* HybridAdaptationManager::NO_LATENCY = "nolatency";
const char* HybridAdaptationManager::TEMPLATE_PATH = "templatePath";
const std::string PCTL = "Rmax=? [ F \"final\" ]";

HybridAdaptationManager::HybridAdaptationManager() : savedDTMC(0){

}

void HybridAdaptationManager::initialize(std::shared_ptr<const pladapt::ConfigurationManager> configMgr, const YAML::Node& params,
                                         std::shared_ptr<const DartPMCHelper> helper) {
	pConfigMgr = configMgr;
	pMcHelper = helper;
	this->params = params;
}

pladapt::TacticList HybridAdaptationManager::evaluate(const pladapt::Configuration& currentConfigObj, const pladapt::EnvironmentDTMCPartitioned& envDTMC,
                                                      const pladapt::UtilityFunction& utilityFunction, unsigned horizon) {

	// QUESTION: Is it possible for the model to be open at this point but not
	//  be loaded into the PlanDB? If so it needs to be accounted for here
  State currentState;
	// Check PRISM for a plan existance
	if(deliberativeWrapper.isModelOpen()) {

		// Check PlanDB for a the existance of the current state
		//PlanDB::get_instance()->populate_state_obj(&dynamic_cast<const DartConfiguration&>(currentConfigObj), &savedDTMC, &envDTMC, currentState);
	}

	// If there is no plan created or loaded -> Replan
	//if(currentState.env_state == UINT_MAX) {
		/* check if we need to adjust the horizon to the environment size */
		if ((envDTMC.getNumberOfParts() - 1) < horizon) {
			if (envDTMC.getNumberOfParts() > 1 && envDTMC.isLastPartFinal()) {
				horizon = envDTMC.getNumberOfParts() - 1;
				cout << "warning: environment is shorter than horizon" << endl;
			}
		}

		string initialState = pMcHelper->generateInitializations(currentConfigObj, utilityFunction, horizon);

		string environmentModel = generateEnvironmentDTMC(envDTMC);

		string templatePath = params[TEMPLATE_PATH].as<string>();
		if (params[NO_LATENCY].as<bool>()) {
			templatePath += "-nl";
		}

		templatePath += ".prism";

		deliberativeWrapper.setModelTemplatePath(templatePath);

		string* pPath = 0;
		if (debug) {
			pPath = new string;
		}

		// Generates the prism model and adversary transition model
		deliberativeWrapper.generatePersistentPlan(environmentModel, initialState, PCTL, pPath);

		// Load the plan into PlanDB
		PlanDB::get_instance()->populate_db(deliberativeWrapper.getModelDirectory().c_str());

    // Clean up PRISM output
    deliberativeWrapper.closeModel();

	//}


	// Use the plan
	PlanDB::Plan p;
	PlanDB::get_instance()->get_plan(&dynamic_cast<const DartConfiguration&>(currentConfigObj),&savedDTMC,&envDTMC, p);

	// Convert the vector of strings into a set of strings to remain compatable
	//  with Gabe's existing code
	pladapt::TacticList result(p.begin(),p.end());

	//TODO: Use Reactive plan here
	// remove sufixes from tactic names (everything after after (and including) an underscore)
	// for (auto& tactic : tactics) {
	//      auto pos = tactic.find('_');
	//      if (pos != string::npos) {
	//              tactic.erase(pos);
	//      }
	//      result.insert(tactic);
	// }

	// Debugging code
	// if (pPath) {
	// 	cout << "*debug path " << *pPath << endl;
	// 	delete pPath;
	// }

	return result;
}


// these constants depend on the PRISM model
const string STATE_VAR = "s";
const string GUARD = "[tick] ";

std::string HybridAdaptationManager::generateEnvironmentDTMC(const pladapt::EnvironmentDTMCPartitioned& envDTMC) {
	const string STATE_VALUE_FORMULA = "formula stateValue";

	// TODO: Check if this needs a deeper copy bc of the shared ptr
  savedDTMC = EnvironmentDTMCPartitioned(envDTMC);

	string result;

	// generate state value formulas
	const int numComponents = envDTMC.getStateValue(0).getNumberOfComponents();
	stringstream stateValueFormulas[numComponents];
	for (int c = 0; c < numComponents; c++) {
		stateValueFormulas[c] << STATE_VALUE_FORMULA;
		if (c > 0) {
			stateValueFormulas[c] << c;
		}
		stateValueFormulas[c] << " = ";
	}

	const string padding(stateValueFormulas[0].str().length(), ' ');
	for (unsigned s = 0; s < envDTMC.getNumberOfStates(); s++) {
		const auto& envValue = envDTMC.getStateValue(s);
		for (int c = 0; c < numComponents; c++) {
			if (s > 0) {
				stateValueFormulas[c] << " + " << endl << padding;
			}
			stateValueFormulas[c] << "(" << STATE_VAR << " = " << s << " ? "
			                      << envValue.getComponent(c).asDouble() << " : 0)";
		}
	}

	for (int c = 0; c < numComponents; c++) {
		stateValueFormulas[c] << ';' << endl;
	}

	// generate state transitions
	stringstream out;

	out << STATE_VAR << " : [0.." << envDTMC.getNumberOfStates() - 1 << "] init 0;" << endl;

	for (unsigned from = 0; from < envDTMC.getNumberOfStates(); from++) {
		bool firstTransition = true;
		for (unsigned to = 0; to < envDTMC.getNumberOfStates(); to++) {
			double probability = envDTMC.getTransitionMatrix() (from, to);
			if (probability > 0.0) {
				if (firstTransition) {
					firstTransition = false;
					out << GUARD << STATE_VAR << " = "
					    << from << " -> " << endl;
					out << '\t';
				} else {
					out << endl << "\t+ ";
				}
				out << probability << " : (" << STATE_VAR << "' = " << to
				    << ")";
			}
		}
		if (!firstTransition) {
			out << ';' << endl;
		}
	}

	out << "endmodule" << endl;

	// append all the state value formulas
	out << endl << "// environment has " << numComponents << " components" << endl;
	for (int c = 0; c < numComponents; c++) {
		out << endl << stateValueFormulas[c].str();
	}

	return out.str();
}

HybridAdaptationManager::~HybridAdaptationManager(){

}

}   /* am2 */
} /* dart */
