/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#ifndef PLANNER_UPDATE_HANDLER_DEFS_HH
#define PLANNER_UPDATE_HANDLER_DEFS_HH

//
// Types and objects which perform PlannerUpdates in PLEXIL
//

#include <functional>
#include <memory>

namespace PLEXIL
{
  // Forward reference
  class AdapterExecInterface;
  class Update;

  //*
  // @brief A PlannerUpdateHandler function sends the contents of the
  //        Update to an external recipient, typically (but not always) a
  //        planner.  It reports success or failure via
  //        AdapterExecInterface::handleUpdateAck().
  //
  // @see AdapterExecInterface::handleUpdateAck
  //
  // @note This interface has largely been superseded by the
  // ExecListener abstract base class.
  // @see ExecListener
  //

  using PlannerUpdateHandler = std::function<void(Update *, AdapterExecInterface *)>;
  using PlannerUpdateHandlerPtr = std::unique_ptr<PlannerUpdateHandler>;
}

#endif // PLANNER_UPDATE_HANDLER_DEFS_HH
