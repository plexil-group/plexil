/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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

#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <math.h>

#include "IpcRobotAdapter.hh"
#include "MyOpenGL.hh"
#include "Macros.hh"
#include "Robot.hh"

#include "Debug.hh"
#include "Error.hh"

Robot::Robot(const TerrainBase* _terrain,
	     EnergySources* _resources,
             Goals* _goals,
	     RobotPositionServer* _posServer,
	     IpcRobotAdapter& adapter,
	     const std::string& _name, 
             int initRow,
	     int initCol,
	     double red, 
	     double green,
             double blue)
    : RobotBase(_terrain, _resources, _goals, _posServer),
      m_DirOffset(5, std::vector<int>(2)),
      m_Name(_name),
      m_RobotPositionMutex(),
      m_RobotEnergyLevelMutex(),
      m_Red(red),
      m_Green(green),
      m_Blue(blue),
      m_EnergyLevel(1.0), 
      m_BeamWidth(0.01),
      m_ScanScale(0.0),
      m_Row(initRow), 
      m_Col(initCol)
  {
    adapter.registerRobot(m_Name, this);
    m_RobotPositionServer->setRobotPosition(m_Name, m_Row, m_Col);
    m_DirOffset[0][0] = -1; // N
    m_DirOffset[0][1] = 0;
    m_DirOffset[1][0] = 0;  // E
    m_DirOffset[1][1] = 1;
    m_DirOffset[2][0] = 1;  // S
    m_DirOffset[3][1] = 0;
    m_DirOffset[3][0] = 0;  // W
    m_DirOffset[3][1] = -1;
    m_DirOffset[4][0] = 0;  // Current Position
    m_DirOffset[4][1] = 0;
  }

Robot::~Robot()
{
  std::cout << "Deleting robot: " << m_Name << std::endl;
}

void Robot::displayRobot(void)
{
  double rWidth = 2.0 / static_cast<double>(m_Terrain->getWidth());
  double rWidthBy2 = rWidth / 2.0;
  double wallThickness = 0.005;
  
  // Use the locally cached value here since we do not want to overload 
  // the position server.
  int row, col;
  getRobotPositionLocal(row, col);
  
  double xCenter = -1.0+col*rWidth+rWidthBy2;
  double yCenter = 1.0-row*rWidth-rWidthBy2;
  double xUB = xCenter + rWidthBy2 - wallThickness;
  double xLB = xCenter - rWidthBy2 + wallThickness;
  double yUB = yCenter - rWidthBy2 + wallThickness;
  double yLB = yCenter + rWidthBy2 - wallThickness;
  
  /* draw unit square polygon */
  glColor3f(m_EnergyLevel*m_Red, m_EnergyLevel*m_Green, m_EnergyLevel*m_Blue);
  
  // Square
  glBegin(GL_POLYGON);
  glVertex2f(xLB, yLB);
  glVertex2f(xLB, yUB);
  glVertex2f(xUB, yUB);
  glVertex2f(xUB, yLB);
  glEnd();
  
  // The East sensors
  glBegin(GL_TRIANGLE_FAN);
  glVertex2f(xUB, yCenter);
  glVertex2f(xUB+m_ScanScale*rWidthBy2, yCenter-m_ScanScale*m_BeamWidth);
  glVertex2f(xUB+m_ScanScale*rWidthBy2, yCenter+m_ScanScale*m_BeamWidth);
  glEnd();
  
  // The South sensors
  glBegin(GL_TRIANGLE_FAN);
  glVertex2f(xCenter, yUB);
  glVertex2f(xCenter-m_ScanScale*m_BeamWidth, yUB-m_ScanScale*rWidthBy2);
  glVertex2f(xCenter+m_ScanScale*m_BeamWidth, yUB-m_ScanScale*rWidthBy2);
  glEnd();
  
  // The West sensors
  glBegin(GL_TRIANGLE_FAN);
  glVertex2f(xLB, yCenter);
  glVertex2f(xLB-m_ScanScale*rWidthBy2, yCenter-m_ScanScale*m_BeamWidth);
  glVertex2f(xLB-m_ScanScale*rWidthBy2, yCenter+m_ScanScale*m_BeamWidth);
  glEnd();
  
  // The North sensors
  glBegin(GL_TRIANGLE_FAN);
  glVertex2f(xCenter, yLB);
  glVertex2f(xCenter-m_ScanScale*m_BeamWidth, yLB+m_ScanScale*rWidthBy2);
  glVertex2f(xCenter+m_ScanScale*m_BeamWidth, yLB+m_ScanScale*rWidthBy2);
  glEnd();
  
  
  m_ScanScale += 0.025;
  if (m_ScanScale > 1.0) m_ScanScale = 0.0;
}

// Purely for demo to have a robot moving in the scene
void Robot::updateRobotPosition()
{
  std::vector<std::vector<int> > dirOffset(4, std::vector<int>(2));
  dirOffset[0][0] = 0;
  dirOffset[0][1] = 1;
  dirOffset[1][0] = 1;
  dirOffset[1][1] = 0;
  dirOffset[2][0] = 0;
  dirOffset[2][1] = -1;
  dirOffset[3][0] = -1;
  dirOffset[3][1] = 0;
  
  std::vector<std::vector<int> >::const_iterator dIter = dirOffset.begin();
  bool done = false;
  
  while (!done)
    {
      std::vector<int> dir = dirOffset[RANDOM_NUMBER_INT(0, 3)];
      int rowNext = m_Row + dir[0];
      int colNext = m_Col + dir[1];
      if (m_Terrain->isTraversable(m_Row, m_Col, rowNext, colNext) &&
          m_RobotPositionServer->setRobotPosition(m_Name, rowNext, colNext))
        {
          done = true;
          setRobotPositionLocal(rowNext, colNext); // local cache for display purposes only
          updateRobotEnergyLevel(m_EnergySources->acquireEnergySource(rowNext, colNext)-0.025);
        }
      ++dIter;
    }
}

double Robot::determineEnergySourceLevel()
{
  int row, col;
  //    readRobotPosition(row, col);
  m_RobotPositionServer->getRobotPosition(m_Name, row, col);
  return m_EnergySources->determineEnergySourceLevel(row, col);
}

double Robot::determineGoalLevel()
{
  int row, col;
  //    readRobotPosition(row, col);
  m_RobotPositionServer->getRobotPosition(m_Name, row, col);
  return m_Goals->determineGoalLevel(row, col);
}

const std::vector<double> Robot::processCommand(const std::string& cmd)
{
  std::cout << "Received " << cmd << std::endl;
  sleep(1);

  if ((cmd == "MoveUp")
      || (cmd == "MoveDown")
      || (cmd == "MoveRight")
      || (cmd == "MoveLeft"))
    return moveRobot(cmd);
  else if (cmd == "QueryEnergySensor")
    return queryEnergySensor();
  else if (cmd == "QueryGoalSensor")
    return queryGoalSensor();
  else if (cmd == "QueryVisibilitySensor")
    return queryVisibility();
  else if (cmd == "QueryRobotState")
    return queryRobotState();

  // fall-thru return
  debugMsg("Robot:processCommand",
	   " Ignoring unknown command \"" << cmd << "\"");
  return std::vector<double>(0);
}

void Robot::getRobotPositionLocal(int& row, int& col)
{
  PLEXIL::ThreadMutexGuard mg(m_RobotPositionMutex);
  row = m_Row;
  col = m_Col;
}

void Robot::setRobotPositionLocal(int row, int col)
{
  PLEXIL::ThreadMutexGuard mg(m_RobotPositionMutex);
  m_Row = row;
  m_Col = col;
}

const double& Robot::readRobotEnergyLevel()
{
  PLEXIL::ThreadMutexGuard mg(m_RobotEnergyLevelMutex);
  return m_EnergyLevel;
}

void Robot::updateRobotEnergyLevel(double energyLevel)
{
  PLEXIL::ThreadMutexGuard mg(m_RobotEnergyLevelMutex);
  m_EnergyLevel = std::max(0.0, std::min(1.0, m_EnergyLevel + energyLevel));
}

const std::vector<double> Robot::queryRobotState()
{
  std::vector<double> result;
  int row, col;
  m_RobotPositionServer->getRobotPosition(m_Name, row, col);
  result.push_back((double) row);
  result.push_back((double) col);
  double energyLevel = readRobotEnergyLevel();
  result.push_back(energyLevel);
  debugMsg("Robot:queryRobotState",
	   " returning " << row << ", " << col << ", " << energyLevel);
  return result;
}

const std::vector<double> Robot::queryEnergySensor()
{
  int row, col;
  m_RobotPositionServer->getRobotPosition(m_Name, row, col);
  
  std::vector<double> result;
  for (std::vector<std::vector<int> >::const_iterator dIter = m_DirOffset.begin();
       dIter != m_DirOffset.end();
       ++dIter)
    {
      result.push_back(m_EnergySources->determineEnergySourceLevel(row+(*dIter)[0], 
								   col+(*dIter)[1]));
    }
  
  return result;
}

const std::vector<double> Robot::queryGoalSensor()
{
  int row, col;
  m_RobotPositionServer->getRobotPosition(m_Name, row, col);
  
  std::vector<double> result;
  for (std::vector<std::vector<int> >::const_iterator dIter = m_DirOffset.begin();
       dIter != m_DirOffset.end();
       ++dIter)
    {
      result.push_back(m_Goals->determineGoalLevel(row+(*dIter)[0], 
						   col+(*dIter)[1]));
      
    }

  return result;
}

const std::vector<double> Robot::queryVisibility()
{
  int row, col;
  m_RobotPositionServer->getRobotPosition(m_Name, row, col);
  
  std::vector<std::vector<int> >::const_iterator dIter = m_DirOffset.begin();
  int currRow = row;
  int currCol = col;
  int iter =  m_DirOffset.size();

  std::vector<double> result;
  // The last (row, col) pair is the curr location. No need to consider it
  // for visibility
  while (iter > 1)
    {
      bool noWall = m_Terrain->isTraversable(currRow, 
					     currCol, 
					     currRow+(*dIter)[0], 
                                             currCol+(*dIter)[1]);
      // If no wall make sure there are no other robots occupying the location.
      if (noWall && m_RobotPositionServer->gridOccupied(currRow+(*dIter)[0],
                                                        currCol+(*dIter)[1]))
        result.push_back(-1.0);
      else
        result.push_back(noWall ? 1.0 : 0.0);
      
      ++dIter;
      --iter;
    }

  return result;
}

const std::vector<double> Robot::moveRobot(const std::string& str)
{
  int rowDirOffset = 0;
  int colDirOffset = 0;
  
  if (str == "MoveUp") 
    {
      rowDirOffset = -1;
    }
  else if (str == "MoveDown")
    {
      rowDirOffset = 1;
    }
  else if (str == "MoveRight")
    {
      colDirOffset = 1;
    }
  else if (str == "MoveLeft")
    {
      colDirOffset = -1;
    }
  else
    assertTrueMsg(ALWAYS_FAIL,
		  "moveRobot: Unknown direction \"" << str << "\"");
  
  int rowCurr, colCurr;
  m_RobotPositionServer->getRobotPosition(m_Name, rowCurr, colCurr);

  int rowNext = rowCurr + rowDirOffset;
  int colNext = colCurr + colDirOffset;
  std::vector<double> result;
  bool traversible = false;
  if ((traversible = m_Terrain->isTraversable(rowCurr, colCurr, rowNext, colNext))
      && m_RobotPositionServer->setRobotPosition(m_Name, rowNext, colNext))
    {
      setRobotPositionLocal(rowNext, colNext);// local cache for display purposes only
      updateRobotEnergyLevel(m_EnergySources->acquireEnergySource(rowNext, colNext) - 0.025);
      result.push_back(1.0);
    }
  else if (!traversible)
    {
      debugMsg("Robot:moveRobot", " Cannot move to desired location due to a fixed obstacle.");
      result.push_back(0.0);
    }
  else
    {
      debugMsg("Robot:moveRobot", " Cannot move to desired location due to a dynamic obstacle.");
      result.push_back(-1.0);
    }
  
  return result;
}
