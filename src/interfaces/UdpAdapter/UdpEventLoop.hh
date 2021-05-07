// Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_UDP_EVENT_LOOP_HH
#define PLEXIL_UDP_EVENT_LOOP_HH

#include "plexil-config.h"

#include <functional> // std::function
#include <memory>     // std::unique_ptr

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h> // in_port_t
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h> // socklen_t
#endif

namespace PLEXIL
{

  //! Function to be called when a datagram arrives.
  //! @param port Port on which the datagram was received.
  //! @param buffer Pointer to the datagram, in event loop allocated storage.
  //! @param length Size of the received datagram.
  //! @param address Pointer to event loop allocated source address buffer.
  //! @param address_len Length of the address.
  using ListenerFunction =
    std::function<void(in_port_t port,
                       const void *buffer,
                       size_t length,
                       const struct sockaddr *address,
                       socklen_t address_len)>;

  //! @class UdpEventLoop
  //! A simplified interface to open a datagram socket, bind it to a
  //! port, and delegate processing of received datagrams to a
  //! caller-defined function on a background thread.
  class UdpEventLoop
  {
  public:
    virtual ~UdpEventLoop() = default;

    //! Listen for datagrams of no more than maxLen octets (bytes) on
    //! the given port. Call the ListenerFunction when one arrives.
    virtual bool openListener(in_port_t port,
                              size_t maxLen,
                              ListenerFunction fn) = 0;

    //! Stop listening on the given port.
    virtual void closeListener(in_port_t port) = 0;

    //! Start the event loop.
    virtual bool start() = 0;

    //! Stop the event loop. Close any remaining listeners.
    virtual void stop() = 0;

  protected:
    UdpEventLoop() = default;

  private:
    UdpEventLoop(const UdpEventLoop &) = delete;
    UdpEventLoop(UdpEventLoop &&) = delete;
    UdpEventLoop &operator=(const UdpEventLoop &) = delete;
    UdpEventLoop &operator=(UdpEventLoop &&) = delete;
  };

  std::unique_ptr<UdpEventLoop> makeUdpEventLoop();

} // namespace PLEXIL

#endif // PLEXIL_UDP_EVENT_LOOP_HH
