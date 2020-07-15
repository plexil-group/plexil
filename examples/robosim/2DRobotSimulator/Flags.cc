/* Copyright (c) 2006-2015, Universities Space Research Association (USRA).
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

#include "Debug.hh"

#include <climits>
#include <cmath>

#include "MyOpenGL.hh"
#include "Macros.hh"
#include "Flags.hh"

Flags::Flags(int _size, double _radius) : m_TerrainSize(_size), m_Radius(_radius), m_AreaVisibility(false)
{
  readFlagLocations();
}

Flags::~Flags()
{
}

bool Flags::acquireFlag(int row, int col)
{
  // if available return eliminate from m_Flags and return
  // true
  // else false
  
  bool found = false;
  //PLEXIL::ThreadMutexGuard mg(m_EnergySourceListMutex);
  
  for (std::vector<std::vector<int> >::iterator iter = m_FlagLocations.begin();
       (iter != m_FlagLocations.end()) && !found; ++iter)
    {
      if (((*iter)[0] == row) && (*iter)[1] == col)
        {
          m_FlagLocations.erase(iter);
          found = true;
        }
    }
  
  return found;
}

void Flags::drawFlag(int row, int col)
{
    double rWidth = 2.0 / static_cast<double>(m_TerrainSize);
    double radius = m_Radius * rWidth;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    
    //Draw flag visibility
    if(m_AreaVisibility)
    {
      glBegin(GL_TRIANGLE_FAN);
      glColor4f(0.0, 1.0, 0.8, 1.0);
      
      glVertex2f(-1.0+col*rWidth+rWidth/2.0, 1.0-row*rWidth-rWidth/2.0);
      
      glColor4f(0.0, 1.0, 0.8, 0.1);
      for (double theta = 0; theta <= 360; theta += 10.0)
        {
          glVertex2f(-1.0+col*rWidth+rWidth/2.0 + radius*cos(theta*PI/180.0), 
                     1.0-row*rWidth-rWidth/2.0-radius*sin(theta*PI/180.0));
        }
      glEnd();
    }   
    // Draw flag icon
    glBegin(GL_QUADS);  
    glColor4f(.078, .699, .336, 1);
    glVertex2f(-1.0+(col-.3)*rWidth+rWidth/2.0, 1.0-(row-.25)*rWidth-rWidth/2.0);
    glVertex2f(-1.0+(col-.3)*rWidth+rWidth/2.0, 1.0-(row)*rWidth-rWidth/2.0);
    glVertex2f(-1.0+(col+.15)*rWidth+rWidth/2.0, 1.0-(row)*rWidth-rWidth/2.0);
    glVertex2f(-1.0+(col+.3)*rWidth+rWidth/2.0, 1.0-(row-.25)*rWidth-rWidth/2.0);
    glColor4f(.078, .699, .336, 1);
    glVertex2f(-1.0+(col-.3)*rWidth+rWidth/2.0, 1.0-(row)*rWidth-rWidth/2.0);
    glVertex2f(-1.0+(col-.3)*rWidth+rWidth/2.0, 1.0-(row+.25)*rWidth-rWidth/2.0);
    glVertex2f(-1.0+(col+.3)*rWidth+rWidth/2.0, 1.0-(row+.25)*rWidth-rWidth/2.0);
    glVertex2f(-1.0+(col+.15)*rWidth+rWidth/2.0, 1.0-(row)*rWidth-rWidth/2.0);
    glEnd();
}

void Flags::displayFlags()
{
    for (unsigned int i = 0; i < m_FlagLocations.size(); ++i)
   {
        int row = m_FlagLocations[i][0];
        int col = m_FlagLocations[i][1];
        drawFlag(row, col);
    }
}

double Flags::determineFlagLevel(int rowCurr, int colCurr) const
{
  // Loop through each goal and return the max.
  
  double maxValue = -1.0 * static_cast<double>(INT_MAX);
  int bestRow = 0, bestCol = 0;
  
  for (unsigned int i = 0; i < m_FlagLocations.size(); ++i) {
    int row = m_FlagLocations[i][0];
    int col = m_FlagLocations[i][1];
      
    // Get Euclidean distance
    double d = EUCLIDEAN_DISTANCE(rowCurr, colCurr, row, col);
      
    // linearly interpolate value at distance if < m_Radius. Else = 0;
    double rValue = (d < m_Radius) ? 1.0 - d / m_Radius : 0.0;
      
    if (rValue > maxValue) {
      maxValue = rValue;
      bestRow = row;
      bestCol = col;
    }
  }

  debugMsg("Flags:determineFlagLevel",
           " at " << rowCurr << ", " << colCurr
           << " best goal is at " << bestRow << ", " << bestCol
           << ", value = " << maxValue);
  
  return maxValue;
}

void Flags::readFlagLocations()
{
  std::vector<int> vTmp(2);
  vTmp[0] = 5;
  vTmp[1] = 15;
  m_FlagLocations.push_back(vTmp);
}
