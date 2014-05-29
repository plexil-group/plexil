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

#ifndef PLEXIL_ASSIGNMENT_HH
#define PLEXIL_ASSIGNMENT_HH

#include "ExecDefs.hh"
#include "UserVariable.hh"
#include "Value.hh"

namespace PLEXIL
{

  class Assignment 
  {
  public:
    Assignment(const AssignableId lhs,
               const ExpressionId rhs, 
               const bool deleteLhs, 
               const bool deleteRhs,
               const std::string &lhsName,
               const std::string &nodeId);
    ~Assignment();
    const AssignmentId& getId() const {return m_id;}
    ExpressionId getDest() const {return (ExpressionId) m_dest;}
    const ExpressionId& getAck() const {return m_ack.getId();}
    const ExpressionId& getAbortComplete() const {return m_abortComplete.getId();}

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

    BooleanVariable m_ack;
    BooleanVariable m_abortComplete;
    AssignmentId m_id;
    AssignableId m_dest;
    ExpressionId m_rhs;
    const std::string m_destName;
    Value m_value; // TODO: templatize by assignable type?
    bool m_deleteLhs, m_deleteRhs;
  };

}

#endif // PLEXIL_ASSIGNMENT_HH
