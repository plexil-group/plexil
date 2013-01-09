/* Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_ASSIGNMENT_HH
#define PLEXIL_ASSIGNMENT_HH

#include "ExecDefs.hh"
#include "LabelStr.hh"

namespace PLEXIL
{

  class Assignment 
  {
  public:
    Assignment(const VariableId lhs,
               const ExpressionId rhs, 
               const bool deleteLhs, 
               const bool deleteRhs,
               const LabelStr& lhsName,
               const LabelStr& nodeId);
    ~Assignment();
    const AssignmentId& getId() const {return m_id;}
    const VariableId& getDest() const {return m_dest;}
    const VariableId& getAck() const {return m_ack;}
    const VariableId& getAbortComplete() const {return m_abortComplete;}

    void activate();
    void deactivate();
    void fixValue();
    void execute();
    void retract();
    void reset();
    const std::string& getDestName() const;

  private:
    // Explicitly not implemented
    Assignment();
    Assignment(const Assignment&);
    Assignment& operator=(const Assignment&);

    AssignmentId m_id;
    VariableId m_ack;
    VariableId m_abortComplete;
    VariableId m_dest;
    ExpressionId m_rhs;
    LabelStr m_destName;
    double m_value;
    double m_previousValue;
    bool m_deleteLhs, m_deleteRhs;
  };

}

#endif // PLEXIL_ASSIGNMENT_HH
