/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

//
// Default methods for LookupHandler
//

#include "LookupHandler.hh"

#include "AdapterExecInterface.hh"
#include "Debug.hh"
#include "LookupReceiver.hh"
#include "State.hh"

namespace PLEXIL
{

  bool LookupHandler::initialize()
  {
    return true;
  }
  
  void LookupHandler::lookupNow(const State &state, LookupReceiver * /* rcvr */)
  {
    debugMsg("LookupHandler:defaultLookupNow", ' ' << state);
  }

  void LookupHandler::setThresholds(const State &state, Real hi, Real lo)
  {
    debugMsg("LookupHandler:defaultSetThresholds",
             ' ' << state << " (Real) " << hi << "," << lo);
  }

  void LookupHandler::setThresholds(const State &state, Integer hi, Integer lo)
  {
    debugMsg("LookupHandler:defaultSetThresholds",
             ' ' << state << " (Integer) " << hi << "," << lo);
  }

  void LookupHandler::clearThresholds(const State &state)
  {
    debugMsg("LookupHandler:defaultClearThresholds", ' ' << state);
  }

}
