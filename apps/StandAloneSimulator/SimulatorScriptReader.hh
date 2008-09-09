#ifndef SIMULATOR_SCRIPT_READER_HH
#define SIMULATOR_SCRIPT_READER_HH

#include <string>

class Simulator;

class SimulatorScriptReader
{
public:
  SimulatorScriptReader(Simulator* simulator=NULL);
  ~SimulatorScriptReader();
  bool readScript(const std::string& fName);

private:
  Simulator* m_Simulator;
};


#endif // SIMULATOR_SCRIPT_READER_HH
