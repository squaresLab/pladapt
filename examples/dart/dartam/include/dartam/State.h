#ifndef STATE_H_
#define STATE_H_

#include <climits>
#include <string>
using namespace std;

namespace dart {
namespace am2 {
class State {
public:
unsigned timestep;
unsigned clockstep;
unsigned env_state;
unsigned altitude;
unsigned formation;
bool goTight_used;
bool goTight_go;
bool goLoose_used;
bool goLoose_go;
unsigned incAlt_state;
bool incAlt_go;
unsigned decAlt_state;
bool decAlt_go;
bool satisfied;
bool targetDetected;


State() :
	timestep(0), clockstep(0), env_state(UINT_MAX), altitude(1), formation(0),
	goTight_used(false), goTight_go(false), goLoose_used(false),
	goLoose_go(false), incAlt_state(0), incAlt_go(false), decAlt_state(0),
	decAlt_go(false), satisfied(false), targetDetected(false)
{
}

State (unsigned timestep,
              unsigned clockstep,
              unsigned env_state,
              unsigned altitude,
              unsigned formation,
              bool goTight_used,
              bool goTight_go,
              bool goLoose_used,
              bool goLoose_go,
              unsigned incAlt_state,
              bool incAlt_go,
              unsigned decAlt_state,
              bool decAlt_go,
              bool satisfied,
              bool targetDetected) :
	// TODO Auto-generated constructor stub
	timestep(timestep), clockstep(clockstep), env_state(env_state), altitude(altitude),
  formation(formation), goTight_used(goTight_used), goTight_go(goTight_go),
  goLoose_used(goLoose_used), goLoose_go(goLoose_go), incAlt_state(incAlt_state),
  incAlt_go(incAlt_go), decAlt_state(decAlt_state),decAlt_go(decAlt_go),
  satisfied(satisfied), targetDetected(targetDetected) {
}

virtual ~State();
std::string get_state_str();
std::string get_bool_str(bool var);
};

}
}

#endif /* STATE_H_ */
