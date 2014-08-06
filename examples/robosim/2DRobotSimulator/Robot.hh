/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
*  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Universities Space Research Association nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
* TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROBOT_HH
#define ROBOT_HH

#include "RobotBase.hh"

#include "ThreadMutex.hh"

class IpcRobotAdapter;

class Robot : public RobotBase
{
public:
  Robot(const TerrainBase* _terrain,
	EnergySources* _resources,
        Goals* _goals,
	RobotPositionServer* _posServer,
	IpcRobotAdapter& adapter,
	const std::string& _name = "Robot0",
        int initRow=0,
	int initCol=0,
	double red=1.0,
	double green=1.0,
        double blue=1.0);

  ~Robot();

  virtual void displayRobot(void);

  // Purely for demo to have a robot moving in the scene
  void updateRobotPosition();

  double determineEnergySourceLevel();
  double determineGoalLevel();

  const std::vector<double> processCommand(const std::string& cmd, int32_t parameter = 0);

private:

  // Deliberately not implemented
  Robot();
  Robot(const Robot&);
  Robot& operator=(const Robot&);

  void getRobotPositionLocal(int& row, int& col);

  void setRobotPositionLocal(int row, int col);

  const double& readRobotEnergyLevel();

  void updateRobotEnergyLevel(double energyLevel);

  const std::vector<double> queryRobotState();

  const std::vector<double> queryEnergySensor();

  const std::vector<double> queryGoalSensor();

  const std::vector<double> queryVisibility();

  const std::vector<double> moveRobot(const std::string& cmd);

  const std::vector<double> moveRobotParameterized(int direction);

  const std::vector<double> moveRobotInternal(int rowDirOffset, int colDirOffset);

  const std::vector<double> moveRandom();

  std::vector<std::vector<int> > m_DirOffset;
  const std::string m_Name;

  PLEXIL::ThreadMutex m_RobotPositionMutex;
  PLEXIL::ThreadMutex m_RobotEnergyLevelMutex;

  double m_Red;
  double m_Green;
  double m_Blue;
  double m_EnergyLevel;
  double m_BeamWidth;
  double m_ScanScale;

  // The locally cached row and col values are used only for display. The actual values
  // for navigation puposes are obtained from the position server.
  int m_Row;
  int m_Col;
};

#endif // ROBOT_HH
