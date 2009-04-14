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

#ifndef _H_LuvListener
#define _H_LuvListener

#include "ExecListener.hh"
#include "PlexilPlan.hh"
#include "LabelStr.hh"
#include "ClientSocket.h"
#include "ServerSocket.h"
#include "tinyxml.h"

#include <pthread.h>

#define LUV_DEFAULT_PORT 9787
#define LUV_DEFAULT_HOST "localhost"
#define LUV_END_OF_MESSAGE ((char)4)
#define MESSAGE_ACKNOWLEDGE "<acknowledge/>"
#define VULTURE_DELAY 10000

namespace PLEXIL 
{
   void* handleConnection(void* threadId);

   class LuvServer
   {
      public:
         LuvServer(const int port, const bool block, PlexilExecId exec);

         int getPort() {return m_port;}
         bool getBlock() {return m_block;}
         PlexilExecId getExec() {return m_exec;}

      private:
         
         pthread_t m_thread;
         int m_port;
         bool m_block;
         PlexilExecId m_exec;
   };

   class LuvListener : public ExecListener
   {
   public:

     LuvListener(const TiXmlElement* xml);
     LuvListener(const std::string& host, const int port, 
                 const bool block = false);
     LuvListener(Socket* socket, const bool block = false);

     ~LuvListener();

     /**
      * @brief Perform listener-specific initialization.
      */
     virtual void initialize();

     /**
      * @brief Perform listener-specific startup.
      */
     virtual void start();

     /**
      * @brief Perform listener-specific actions to stop.
      */
     virtual void stop();

     /**
      * @brief Perform listener-specific actions to reset to initialized state.
      */
     virtual void reset();

     /**
      * @brief Perform listener-specific actions to shut down.
      */
     virtual void shutdown();

    /**
     * @brief Notify that a node has changed state.
     * @param prevState The old state.
     * @param node The node that has transitioned.
     * @note The current state is accessible via the node.
     */
     void notifyOfTransition(const LabelStr& prevState, 
                             const NodeId& node) const;

    /**
     * @brief Notify that a plan has been received by the Exec.
     * @param plan The intermediate representation of the plan.
     * @param parent The name of the parent node under which this plan will be inserted.
     */
     void notifyOfAddPlan(const PlexilNodeId& plan, 
                          const LabelStr& parent) const;

    /**
     * @brief Notify that a library node has been received by the Exec.
     * @param libNode The intermediate representation of the plan.
     * @note The default method is deprecated and will go away in a future release.
     */
     void notifyOfAddLibrary(const PlexilNodeId& plan) const;

   protected:

     //
     // Static helper methods
     //

     static TiXmlNode& constructNodePath(TiXmlNode& path,
                                         const NodeId& node);
     static TiXmlNode& constructConditions(TiXmlNode& conditions,
                                           const NodeId& node);

     //
     // Internal methods
     //

     void sendPlanInfo() const;
     void sendMessage(const TiXmlNode& xml) const;
     void sendMessage(const std::string& message) const;
     void waitForAcknowledge() const;

     // *** these don't seem to be used anywhere ***
     void sendTaggedStream(std::istream& stream, 
                           const std::string& tag) const;
     void sendStream(std::istream& stream) const;

   private:
     //
     // Deliberately unimplemented
     //
     LuvListener();
     LuvListener(const LuvListener&);
     LuvListener& operator=(const LuvListener&);

     //
     // Member variables
     //
     Socket* m_socket;
     bool    m_block;
   };
}

#endif


