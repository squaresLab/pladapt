#include <dartam/PlanDB.h>
#include <boost/tokenizer.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

namespace dart {
namespace am2 {
PlanDB* PlanDB::m_db_object = NULL;

bool debug = false;

PlanDB::PlanDB() : m_states_file("result.sta"), m_adversary_file("result.adv") {
	// TODO Auto-generated constructor stub
}

PlanDB::~PlanDB() {
	// TODO Auto-generated destructor stub
}

void PlanDB::destroy_db() {
	clean_db();

	assert(m_db_object != NULL);
	delete m_db_object;

	m_db_object = NULL;
}

void PlanDB::clean_db() {
	/*AdversaryMap::iterator itr = m_adversary_map.begin();

	   while (itr != m_adversary_map.end()) {
	   delete itr->second;
	   ++itr;
	   }*/

	m_adversary_map.clear();
	m_state_hash_map.clear();
}

std::size_t PlanDB::get_hash(string state_str) {
	std::hash<std::string> hash_fn;
	std::size_t str_hash = hash_fn(state_str);

	return str_hash;
}

bool PlanDB::populate_states(const char* dir) {
	bool success = true;
	string states_path = dir + string("/") + m_states_file;
	ifstream states_fin(states_path);

	if (!states_fin) {
		cout << "Could not read input file " << states_path << endl;
		success = false;
	} else {
		string line;
		bool firstLine = true;

		while (getline(states_fin, line)) {
			if (firstLine) {
				firstLine = false;
				continue;
			}

			typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
			tokenizer tokens(line, boost::char_separator<char>(":"));
			tokenizer::iterator it = tokens.begin();

			if (it != tokens.end()) {
				//int state = atoi(it->c_str());
				unsigned long state = ULONG_MAX;
				stringstream(*it) >> state;

				assert (++it != tokens.end());
				size_t hash = get_hash(*it);

				if (debug) {
					//cout << "state = " << state << " hash = " << hash << endl;
				}

				m_state_hash_map[hash] = state;
			}
		}
	}

	states_fin.close();

	return success;
}

bool PlanDB::populate_adv(const char* dir) {
	bool success = true;
	string adv_path = dir + string("/") + m_adversary_file;
	ifstream adv_fin(adv_path);

	if (!adv_fin) {
		cout << "Could not read input file " << adv_path << endl;
		success = false;
	} else {
		string line;
		bool firstLine = true;
		bool secondLine = true;

		while (getline(adv_fin, line)) {
			if (firstLine) {
				firstLine = false;
				continue;
			}
			if (secondLine) {
				secondLine = false;
				continue;
			}

			typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
			tokenizer tokens(line, boost::char_separator<char>(" "));
			tokenizer::iterator it = tokens.begin();

			// First token is current state
			unsigned long curr_state = ULONG_MAX;         // = atoi(it->c_str());
			stringstream(*it) >> curr_state;
			//int curr_state = atoi(it->c_str());
			assert(++it != tokens.end());

			// Second token is next state
			unsigned long next_state = ULONG_MAX;         // = atoi(it->c_str());
			stringstream(*it) >> next_state;
			assert(++it != tokens.end());

			// Third token is probability. Ignoring it for now.
			// Can't assert that this is not the end, sometimes there is no action so
			//	tactic is blank
			// assert(++it != tokens.end());

			TacticEnum action;
			if(++it != tokens.end()) {
				// Fourth token is the action
				action = get_tactic_code(*it);
			} else {
				action = NONE;
			}

			PlanDB::Transition transition = PlanDB::Transition(next_state, action);
			PlanDB::Transition transition1(next_state, action);
			m_adversary_map[curr_state] = transition;

			if (debug) {
				//cout << "state = " << curr_state << " next_state = " << next_state
				//        << " action_str = " << *it << " action = " << action << endl;
			}
		}
	}

	adv_fin.close();

	return success;
}

bool PlanDB::populate_db(const char* dir) {
	bool success = true;

	success = populate_states(dir);

	if (success) {
		success = populate_adv(dir);
	}

	return success;
}

TacticEnum PlanDB::get_tactic_code(string tactic_name) {
	TacticEnum tactic = NONE;

	if (tactic_name.compare("tick") == 0) {
		tactic = TICK;
	} else if (tactic_name.compare("DecAlt_start") == 0) {
		tactic = DEC_ALT;
	} else if (tactic_name.compare("IncAlt_start") == 0) {
		tactic = INC_ALT;
	} else if (tactic_name.compare("GoLoose_start") == 0) {
		tactic = GO_LOOSE;
	} else if (tactic_name.compare("GoTight_start") == 0) {
		tactic = GO_TIGHT;
	}

	return tactic;
}

string PlanDB::get_tactic_str(TacticEnum tactic) {
	string tactic_str = "";

	switch (tactic) {
	case TICK:
		tactic_str = "tick";
		break;
	case DEC_ALT:
		tactic_str = "DecAlt_start";
		break;
	case INC_ALT:
		tactic_str = "IncAlt_start";
		break;
	case GO_LOOSE:
		tactic_str = "GoLoose_start";
		break;
	case GO_TIGHT:
		tactic_str = "GoTight_start";
		break;
	default:
		break;
	}

	return tactic_str;
}

// Get the current state object
bool PlanDB::populate_state_obj(const DartConfiguration* config,
                                const pladapt::EnvironmentDTMCPartitioned* oldPredictions,
                                const pladapt::EnvironmentDTMCPartitioned* newPredictions,
                                State& state) {
	bool valid_state = false;

	// TODO: Confirm that these are the correct components for target and threat #drew
	// Not serious if wrong. We just need to be consistent in the usage here
	double currentTargetProb = newPredictions->getStateValue(0).getComponent(0).asDouble();
	double currentThreatProb = newPredictions->getStateValue(0).getComponent(1).asDouble();


	// TODO: Fix this so the enviroments can accurately pull the numbers #drew
	// Loop through the states and find the closest match
	// Find the first state where both target and thread are within the bound
	double targetProb;
	double threatProb;
	double tolerance = 0.1f;

	// Define priority queue to keep track of env matches
	vector<pair<double, int>> matchingEnvStates;

	for (unsigned s = 0; s < oldPredictions->getNumberOfStates(); s++) {
		const auto& envValue = oldPredictions->getStateValue(s);
		targetProb = envValue.getComponent(0).asDouble();
		threatProb = envValue.getComponent(1).asDouble();

		// Compute the L1 distance of the predicted probabilities from the current
		double distanceL1 = (abs(threatProb - currentThreatProb) + abs(targetProb - currentTargetProb));
		if(distanceL1 < tolerance) {
			matchingEnvStates.push_back(pair<double,int>(distanceL1,s));
		}
	}

	/* Find the best matching state within the tolerance */

	// Closest matches are at the front of the vector
	sort(matchingEnvStates.begin(),matchingEnvStates.end(),less_than_key());

	// Set the other attributes to match against
	state.timestep = config->getTimestep();
	state.altitude = config->getAltitudeLevel();
	state.formation = config->getFormation() == DartConfiguration::LOOSE ? 0 : 1;
	state.incAlt_state = config->getTtcIncAlt();
	state.decAlt_state = config->getTtcDecAlt();
	state.targetDetected = config->getTargetDetected();

	// These are always the same in the states we start from
	// They are used to syncronize and handle tactics across the various tick and tack actions
	state.goTight_go = true;
	state.goLoose_go = true;
	state.goTight_used = false;
	state.goLoose_used = false;
	state.incAlt_go = true;
	state.decAlt_go = true;
	state.clockstep = 0;
	// TODO: May need to check both true and false for satisfied #drew
	state.satisfied = true;

	// Take the first state that hashes to a real state
	for(auto it = matchingEnvStates.begin(); it != matchingEnvStates.end(); it++){
		state.env_state = it->second;

		unsigned long stateNum = get_state(state);
		if(stateNum != ULONG_MAX){
			valid_state = true;
			return valid_state;
		}
	}

	// No state was valid so return failure
	state = State();
	return valid_state;
}

unsigned long PlanDB::get_state(State& state) {
	unsigned long state_no = ULONG_MAX;
	string state_str = state.get_state_str();
	size_t state_hash = get_hash(state_str);
	StateHashMap::iterator itr = m_state_hash_map.find(state_hash);

	if (itr != m_state_hash_map.end()) {
		state_no = (*itr).second;
	}

	return state_no;
}

bool PlanDB::get_plan(const DartConfiguration* config,const pladapt::EnvironmentDTMCPartitioned* oldPredictions,
                      const pladapt::EnvironmentDTMCPartitioned* newPredictions, Plan& plan) {
	bool plan_found = false;

	State state_obj;

	if (m_adversary_map.size() != 0 && populate_state_obj(config, oldPredictions, newPredictions, state_obj)) {
		unsigned long state = get_state(state_obj);

		if (debug) {
			cout << "state hash = " << state << endl;
		}
		if (state != ULONG_MAX) {
			AdversaryMap::iterator itr = m_adversary_map.find(state);
			assert(itr != m_adversary_map.end());
			unsigned long next_state = (*itr).second.first;
			string tactic = get_tactic_str((*itr).second.second);

			while (true) {
				if ((*itr).second.second == TICK) {
					break;
				}

				if (tactic != "") {
					if (debug) cout << "tactic = " << tactic << endl;
					plan.push_back(tactic);
				}

				itr = m_adversary_map.find(next_state);
				assert(itr != m_adversary_map.end());
				next_state = (*itr).second.first;
				tactic = get_tactic_str((*itr).second.second);
			}

			// remove sufixes from tactic names (everything after after (and including) an underscore)
	    for (auto& tacticName : plan) {
	    	auto pos = tacticName.find('_');
	    	if (pos != string::npos) {
	    		tacticName.erase(pos);
	    	}
	    }

			plan_found = true;
		}
	}

	return plan_found;
}

} /* am2 */
} /* dart */
