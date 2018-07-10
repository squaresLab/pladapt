#ifndef PLANDB_H_
#define PLANDB_H_

#include <string>
#include <vector>
#include <dartam/State.h>
#include <dartam/DartConfiguration.h>
#include <pladapt/EnvironmentDTMCPartitioned.h>

namespace dart {
namespace am2 {

enum TacticEnum {
	NONE,
	TICK,
	INC_ALT,
	DEC_ALT,
	GO_LOOSE,
	GO_TIGHT
};

class PlanDB {
public:
typedef std::pair<unsigned long, TacticEnum> Transition;
typedef std::vector<std::string> Plan;
typedef std::map<unsigned long, Transition> AdversaryMap;
typedef std::map<size_t, unsigned long> StateHashMap;

static PlanDB* get_instance() {
        if (m_db_object == NULL) {
            m_db_object = new PlanDB();
        }

        return m_db_object;
    }

bool populate_db(const char* dir);
void destroy_db();
void clean_db();
bool get_plan(const DartConfiguration* config, const pladapt::EnvironmentDTMCPartitioned* oldPredictions,
              const pladapt::EnvironmentDTMCPartitioned* newPredictions, Plan& plan);

Plan get_actions(State& state);
~PlanDB();
PlanDB();
bool populate_state_obj(const DartConfiguration* config,
	const pladapt::EnvironmentDTMCPartitioned* oldPredictions,
	const pladapt::EnvironmentDTMCPartitioned* newPredictions,
	State& state);

private:

size_t get_hash(string state_str);
unsigned long get_state(State& state);
string get_tactic_str(TacticEnum tactic);
TacticEnum get_tactic_code(string tactic_name);
bool populate_adv(const char* dir);
bool populate_states(const char* dir);

static PlanDB* m_db_object;
string m_db_dir;

AdversaryMap m_adversary_map;
StateHashMap m_state_hash_map;

const char* m_states_file;
const char* m_adversary_file;
};

} /* dart */
} /* am2 */


#endif
