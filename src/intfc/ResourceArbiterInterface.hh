// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLEXIL_RESOURCE_ARBITER_INTERFACE_HH
#define PLEXIL_RESOURCE_ARBITER_INTERFACE_HH

#include <istream>
#include <string>

namespace PLEXIL 
{
  // Forward references
  class CommandImpl;
  template <typename T> class LinkedQueue;

  //! \class ResourceArbiterinterface
  //! \brief Stateless abstract base class for the command resource arbiter.
  class ResourceArbiterInterface
  {
  public:
    //! \brief Virtual destructor.
    virtual ~ResourceArbiterInterface() = default;

    //! \brief Read the resource hierarchy descriptor file.
    //! \param fName The file name, as a const reference to string.
    //! \return true if successful, false if not.
    virtual bool readResourceHierarchyFile(std::string const &fName) = 0;

    //! \brief Read the resource hierarchy from an input stream.
    //! \param s The stream.
    //! \return true if successful, false if not.
    virtual bool readResourceHierarchy(std::istream &s) = 0;

    //! \brief Partition a list of commands into accepted and rejected
    //!        requests by resources requested and priority.
    //! \param[in] cmds Reference to a LinkedQueue which is consumed by the function.
    //! \param[out] acceptCmds Reference to a LinkedQueue provided by the caller to receive accepted commands.
    //! \param[out] rejectCmds Reference to a LinkedQueue provided by the caller to receive rejected commands.
    virtual void arbitrateCommands(LinkedQueue<CommandImpl> &cmds,
                                   LinkedQueue<CommandImpl> &acceptCmds,
                                   LinkedQueue<CommandImpl> &rejectCmds) = 0;

    //! \brief Release the resources reserved by the given command, if any.
    //! \param[in] cmd Pointer to the command.
    virtual void releaseResourcesForCommand(CommandImpl *cmd) = 0;
  };

  //! \brief Construct a resource arbiter instance.
  //! \return Pointer to the new instance.
  extern ResourceArbiterInterface *makeResourceArbiter();
  
}
#endif // PLEXIL_RESOURCE_ARBITER_INTERFACE_HH
