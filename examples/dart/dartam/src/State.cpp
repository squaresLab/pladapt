#include <dartam/State.h>
#include <iostream>

namespace dart {
  namespace am2 {
    State::~State() {
      // TODO Auto-generated destructor stub
    }

    string State::get_bool_str(bool var) {
      return var ? "true" : "false";
    }

    string State::get_state_str() {

      string state_str = "(";

      state_str += to_string(timestep) + ",";
      state_str += to_string(clockstep) + ",";
      state_str += to_string(env_state) + ",";
      state_str += to_string(altitude) + ",";
      state_str += to_string(formation) + ",";
      state_str += to_string(ecm) + ",";
      state_str += get_bool_str(ecmOn_used) + ",";
      state_str += get_bool_str(ecmOn_go) + ",";
      state_str += get_bool_str(ecmOff_used) + ",";
      state_str += get_bool_str(ecmOff_go) + ",";
      state_str += get_bool_str(goTight_used) + ",";
      state_str += get_bool_str(goTight_go) + ",";
      state_str += get_bool_str(goLoose_used) + ",";
      state_str += get_bool_str(goLoose_go) + ",";
      state_str += to_string(incAlt_state) + ",";
      state_str += get_bool_str(incAlt_go) + ",";
      state_str += to_string(decAlt_state) + ",";
      state_str += get_bool_str(decAlt_go) + ",";
      state_str += get_bool_str(satisfied) + ",";
      state_str += get_bool_str(targetDetected);

      state_str += ")";

      //cout << "State::get_state_str = " << state_str << endl;

      return state_str;
    }

  }
}
