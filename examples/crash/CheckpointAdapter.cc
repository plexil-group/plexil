#include "CheckpointAdapter.hh"

#include "subscriber.hh"
#include "checkpoint_system.hh"

#include "AdapterConfiguration.hh"
#include "AdapterFactory.hh"
#include "AdapterExecInterface.hh"
#include "Debug.hh"
#include "StateCacheEntry.hh"

using std::cerr;
using std::endl;
using std::map;
using std::string;
using std::vector;
using std::copy;


///////////////////////////// Conveniences //////////////////////////////////

// A preamble for error messages.
static string error = "Error in CheckpointAdapter: ";

// A prettier name for the "unknown" value.
static Value Unknown;

// A localized handle on the adapter, which allows a
// decoupling between the checkpoint system and adapter.
static CheckpointAdapter * Adapter;

// An empty argument vector.
static vector<Value> EmptyArgs;


///////////////////////////// State support //////////////////////////////////

// Queries the system for the value of a state and its arguments.
//
static Value fetch (const string& state_name, const vector<Value>& args)
{
  debugMsg("SampleAdapter:fetch",
           "Fetch called on " << state_name << " with " << args.size() << " args");
  Value retval;

  
  if (state_name == "DidCrash"){
    retval = did_crash();
  }
  
  else if (state_name == "NumberOfActiveCrashes"){
    retval = num_active_crashes();
  }
  else if (state_name == "NumberOfTotalCrashes"){
    retval = num_total_crashes();
  }

  else{
    // Default is checkpoint in current boot (iteration 0)
    int32_t which_boot = 0;
    if(!args.empty()){
      args[1].getValue(which_boot);
    }

    
    if (state_name == "Checkpoint"){
      string which_checkpoint;
      args[0].getValue(which_checkpoint);
      retval = get_checkpoint_state(which_checkpoint,which_boot);
    }
    else if (state_name == "TimeOfCheckpoint"){
      string which_checkpoint;
      args[0].getValue(which_checkpoint);
      retval = get_checkpoint_time(which_checkpoint,which_boot);
    }
    else if (state_name == "TimeOfCrash"){
      retval = get_time_of_crash(which_boot);
    }
    else if (state_name == "TimeOfBoot"){
      retval = get_time_of_boot(which_boot);
    }
    //No match
    else {
      cerr << error << "invalid state: " << state_name << endl;
      retval = Unknown;
    }

  }

  debugMsg("SampleAdapter:fetch", "Fetch returning " << retval);
  return retval;
}

// The 'receive' functions are the subscribers for system state updates.  They
// receive the name of the state whose value has changed in the system.  Then
// they propagate the state's new value to the executive.

static void propagate (const State& state, const vector<Value>& value)
{
  Adapter->propagateValueChange (state, value);
}

static State createState (const string& state_name, const vector<Value>& value)
{
  State state(state_name, value.size());
  if (value.size() > 0)
  {
    for(size_t i=0; i<value.size();i++)
    {
      state.setParameter(i, value[i]);
    }
  }
  return state;
}

static void receiveBool (const string& state_name, bool val)
{
  propagate (createState(state_name, EmptyArgs),
             vector<Value> (1, val));
}

static void receiveInt (const string& state_name, int val)
{
  propagate (createState(state_name, EmptyArgs),
             vector<Value> (1, val));
}

static void receiveBoolInt (const string& state_name, bool val,  int arg)
{
  propagate (createState(state_name, vector<Value> (1, arg)),
             vector<Value> (1, val));
}

static void receiveIntInt (const string& state_name, int val,  int arg)
{
  propagate (createState(state_name, vector<Value> (1, arg)),
             vector<Value> (1, val));
}

static void receiveBoolStringInt (const string& state_name, bool val, const string& arg1, int arg2)
{
  vector<Value> vec;
  vec.push_back (arg1);
  vec.push_back (arg2);
  propagate (createState(state_name, vec), vector<Value> (1, val));
}

static void receiveIntStringInt (const string& state_name, int val, const string& arg1, int arg2)
{
  vector<Value> vec;
  vec.push_back (arg1);
  vec.push_back (arg2);
  propagate (createState(state_name, vec), vector<Value> (1, val));
}


///////////////////////////// Member functions //////////////////////////////////


SampleAdapter::SampleAdapter(AdapterExecInterface& execInterface,
                             const pugi::xml_node& configXml) :
    InterfaceAdapter(execInterface, configXml)
{
  debugMsg("SampleAdapter", " created.");
}

bool SampleAdapter::initialize()
{
  g_configuration->defaultRegisterAdapter(this);
  Adapter = this;
  setSubscriber (receiveInt);
  setSubscriber (receiveBool);
  setSubscriber (receiveBoolInt);
  setSubscriber (receiveIntInt);
  setSubscriber (receiveBoolStringInt);
  setSubscriber (receiveIntStringInt);
  debugMsg("CheckpointAdapter", " initialized.");
  return true;
}

bool SampleAdapter::start()
{
  debugMsg("SampleAdapter", " started.");
  return true;
}

bool SampleAdapter::stop()
{
  debugMsg("SampleAdapter", " stopped.");
  return true;
}

bool SampleAdapter::reset()
{
  debugMsg("SampleAdapter", " reset.");
  return true;
}

bool SampleAdapter::shutdown()
{
  debugMsg("SampleAdapter", " shut down.");
  return true;
}


// Sends a command (as invoked in a Plexil command node) to the system and sends
// the status, and return value if applicable, back to the executive.
//
void SampleAdapter::executeCommand(Command *cmd)
{
  const string &name = cmd->getName();
  debugMsg("SampleAdapter", "Received executeCommand for " << name);  

  Value retval = Unknown;
  const vector<Value>& args = cmd->getArgValues();
  
  if (name == "SetCheckpoint") {
    string &checkpoint_name;
    args[0].getValue(checkpoint_name);
    set_checkpoint(checkpoint_name);
  }
  else if (name == "SetSafeReboot") {
    bool b;
    args[0].getValue(b);
    set_safe_reboot(b);
  }
  else if (name == "DeleteCrash"){
    int32_t crash_number = num_active_crashes();
    if(!args.empty()){
      args[0].getValue(crash_number);
    }
    delete_crash(crash_number);
  }
  else{ 
    cerr << error << "invalid command: " << name << endl;
  }
  
  // This sends a command handle back to the executive.

  //TODO: incorporate COMMAND_SUCCEEDED after IO completed
  m_execInterface.handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
  // This sends the command's return value (if expected) to the executive.
  if (retval != Unknown){
    m_execInterface.handleCommandReturn(cmd, retval);
  }
  m_execInterface.notifyOfExternalEvent();
}

void SampleAdapter::lookupNow(State const &state, StateCacheEntry &entry)
{
  // This is the name of the state as given in the plan's LookupNow
  string const &name = state.name();
  const vector<Value>& args = state.parameters();
  entry.update(fetch(name, args));
}


void SampleAdapter::subscribe(const State& state)
{
  debugMsg("SampleAdapter:subscribe", " processing state "
           << state.name());
  m_subscribedStates.insert(state);
}


void SampleAdapter::unsubscribe (const State& state)
{
  debugMsg("SampleAdapter:subscribe", " from state "
           << state.name());
  m_subscribedStates.erase(state);
}

// Does nothing.
void SampleAdapter::setThresholds (const State& state, double hi, double lo)
{
}

void SampleAdapter::setThresholds (const State& state, int32_t hi, int32_t lo)
{
}


void SampleAdapter::propagateValueChange (const State& state,
                                          const vector<Value>& vals) const
{
  if (!isStateSubscribed(state))
    return; 
  m_execInterface.handleValueChange(state, vals.front());
  m_execInterface.notifyOfExternalEvent();
}


bool SampleAdapter::isStateSubscribed(const State& state) const
{
  return m_subscribedStates.find(state) != m_subscribedStates.end();
}

// Necessary boilerplate
extern "C" {
  void initSampleAdapter() {
    REGISTER_ADAPTER(SampleAdapter, "SampleAdapter");
  }
}
