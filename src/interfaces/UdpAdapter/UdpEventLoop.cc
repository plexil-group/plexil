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

#include "UdpEventLoop.hh"

#include "Debug.hh"
#include "Error.hh" // warn(), assertTrue_1()
#include "ThreadSemaphore.hh"

#include <map>
#include <mutex>
#include <thread>

#if defined(HAVE_CERRNO)
#include <cerrno>
#elif defined(HAVE_ERRNO_H)
#include <cerrno>
#endif

#if defined(HAVE_CSTDIO)
#include <cstdio>
#elif defined(HAVE_STDIO_H)
#include <stdio.h>
#endif

#if defined(HAVE_CSTRING)
#include <cstring>
#elif defined(HAVE_STRING_H)
#include <string.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h> // pipe()
#endif

// Linux lacks FD_COPY, so emulate it here.
#ifndef FD_COPY
#define FD_COPY(from_ptr, to_ptr) memcpy(to_ptr, from_ptr, sizeof(fd_set))
#endif

namespace PLEXIL
{

  //! Structure to maintain the state of one listener.
  struct Listener
  {
    ListenerFunction func;
    size_t maxSize;
    std::unique_ptr<char[]> buffer;
    std::unique_ptr<struct sockaddr_storage> addrBuf;
    socklen_t addrSizeBuf;
    int socketFD;
    in_port_t port;
    bool active;

    Listener(int fd, in_port_t p, size_t maxLen, ListenerFunction fn)
      : func(fn),
        maxSize(maxLen),
        buffer(std::make_unique<char[]>(maxLen)),
        addrBuf(std::make_unique<struct sockaddr_storage>()),
        addrSizeBuf(),
        socketFD(fd),
        port(p),
        active(false)
    {
    }

    ~Listener() = default;
  };

  //! Control operations
  enum ControlOp : uint16_t {
    OP_NO_OP = 0, // invalid
    OP_ADD = 1,
    OP_REMOVE = 2,
    OP_MAX
  };

  //! Message format sent down control pipe.
  struct ControlMsg
  {
    ControlOp op;
    in_port_t port;
  };

  class UdpEventLoopImpl final : public UdpEventLoop
  {
  private:
    using ListenerMap = std::map<unsigned short, std::unique_ptr<Listener> >;
    using DescriptorMap = std::map<int, Listener *>;

    //! Port -> listener map
    ListenerMap m_listeners;
    //! FD -> listener map
    DescriptorMap m_descriptors;
    //! Lock for port -> listener map
    std::mutex m_listenerMutex;
    //! Semaphore for background 'task complete' notification
    ThreadSemaphore m_sem;
    //! Event loop thread.
    std::thread m_eventThread;
    //! File descriptors for control pipe.
    //! m_pipeFDs[0] is the event thread read port.
    //! m_pipeFDs[1] is the control write port.
    int m_pipeFDs[2];

  public:
    UdpEventLoopImpl()
      : UdpEventLoop(),
        m_listeners(),
        m_listenerMutex(),
        m_sem(),
        m_eventThread(),
        m_pipeFDs()
    {
      m_pipeFDs[0] = 0;
      m_pipeFDs[1] = 0;
    }

    virtual ~UdpEventLoopImpl()
    {
    }

    //! Listen for datagrams of no more than maxLen octets (bytes) on
    //! the given port. Call the ListenerFunction when one arrives.
    virtual bool openListener(in_port_t port,
                              size_t maxLen,
                              ListenerFunction fn)
    {
      debugMsg("UdpEventLoop:openListener", "(" << port << ")");
      if (!m_pipeFDs[1]) {
        warn("UdpEventLoop: must start event loop before opening listeners");
        return false;
      }

      if (!m_eventThread.joinable()) {
        warn("UdpEventLoop: Event loop thread has crashed!");
        return false;
      }

      // Create a socket and bind it to the port
      int fd = openSocketOnPort(port);
      if (fd < 0) {
        warn("UdpEventLoop::openListener: opening UDP socket on port " << port << " failed");
        return false;
      }

      // Allocate a Listener
      {
        Listener *l = new Listener(fd, port, maxLen, fn);
        std::lock_guard<std::mutex> guard(m_listenerMutex);
        m_listeners.emplace(std::make_pair(port, l));
      }

      // Tell event loop it has a new listener
      ControlMsg addMsg = {OP_ADD, port};
      ssize_t nbytes = write(m_pipeFDs[1], &addMsg, sizeof(ControlMsg));
      if (nbytes < 0) {
        warn("UdpEventLoop::openListener: unable to notify event thread: " << strerror(errno));
      }
      else if (sizeof(ControlMsg) != (size_t) nbytes) {
        warn("UdpEventLoop::openListener: control pipe: byte count mismatch.\n Expected "
             << sizeof(ControlMsg) << ", actual " << nbytes);
      }
      else {
        // Wait for completion of request
        m_sem.wait();
      }

      debugMsg("UdpEventLoop:openListener", " port " << port << " succeeded");
      return true;
    }

    //! Stop listening on the given port.
    virtual void closeListener(in_port_t port)
    {
      debugMsg("UdpEventLoop:closeListener", "(" << port << ")");

      if (!m_pipeFDs[1]) {
        // never started, so no listeners, guaranteed
        debugMsg("UdpEventLoop:closeListener", " not running, ignored");
        return;
      }

      if (m_eventThread.joinable()) {
        ControlMsg removeMsg = {OP_REMOVE, port};
        ssize_t nbytes = write(m_pipeFDs[1], &removeMsg, sizeof(ControlMsg));
        if (nbytes < 0) {
          warn("UdpEventLoop::closeListener: unable to notify event thread: " << strerror(errno));
        }
        else if (sizeof(ControlMsg) != (size_t) nbytes) {
          warn("UdpEventLoop::closeListener: control pipe: byte count mismatch.\n Expected "
               << sizeof(ControlMsg) << ", actual " << nbytes);
        }
        else {
          // Wait for completion of request
          m_sem.wait();
        }
      }
      else {
        warn("UdpEventLoop: Event loop thread has crashed!");
      }

      // Close socket
      if (close(m_listeners[port]->socketFD)) {
        warn("UdpEventLoop: close() failed for file descriptor "
             << m_listeners[port]->socketFD << " (port " << port << "):\n "
             << strerror(errno));
      }

      // Listener can now be deleted
      {
        std::lock_guard<std::mutex> guard(m_listenerMutex);
        m_listeners.erase(port);
      }
      debugMsg("UdpEventLoop:closeListener", " port " << port << " complete");
    }

    //! Start the event loop.
    virtual bool start()
    {
      if (m_pipeFDs[1]) {
        warn("UdpEventLoop::start: already running, ignored");
        return true;
      }

      if (pipe(m_pipeFDs)) {
        warn("UdpEventLoop: internal error: failed to create control pipe");
        return false;
      }

      debugMsg("UdpEventLoop:start", " entered");

      m_eventThread =
        std::thread([this](int fd) -> void { this->eventLoop(fd); },
                    m_pipeFDs[0]);
      if (!m_eventThread.joinable()) {
        warn("UdpEventLoop::start: Event loop thread failed to start");
        close(m_pipeFDs[1]);
        close(m_pipeFDs[0]);
        m_pipeFDs[0] = 0;
        m_pipeFDs[1] = 0;
        return false;
      }

      debugMsg("UdpEventLoop:start", " succeeded");
      return true;
    }

    //! Stop the event loop. Close any remaining listeners.
    virtual void stop()
    {
      if (!m_pipeFDs[1]) {
        warn("UdpEventLoop::stop: not running, ignored");
        return;
      }

      debugMsg("UdpEventLoop:stop", " entered");
      
      // Tell event thread to shut down
      if (close(m_pipeFDs[1])) {
        warn("UdpEventLoop: close() failed on control pipe: " << strerror(errno));
      }
      else {
        // Wait for it to finish cleaning up
        if (m_eventThread.joinable())
          m_eventThread.join();
      }

      // Close any remaining open sockets
      for (ListenerMap::value_type const &pr : m_listeners) {
        if (close(pr.second->socketFD)) {
          warn("UdpEventLoop: close() failed for file descriptor "
               << pr.second->socketFD << " (port " << pr.second->port << "):\n "
               << strerror(errno));
        }
      }
      // And forget all the listeners
      m_listeners.clear();
      debugMsg("UdpEventLoop:stop", " complete");
    }

  private:

    //! Open a datagram socket and bind it to the given port.
    //! @param port The port to bind to.
    //! @return The FD for the socket. If < 0, the operation failed.
    int openSocketOnPort(in_port_t port)
    {
      debugMsg("UdpEventLoop:openSocketOnPort", "(" << port << ")");

      // Open a datagram socket
      int sock = socket(AF_INET, SOCK_DGRAM, 0);
      if (sock < 0) {
        warn("UdpEventLoop: socket() failed: " << strerror(errno));
        return sock;
      }

      // Set up parameters for bind()
      struct sockaddr_in local_addr = {};
      memset((char *) &local_addr, 0, sizeof(local_addr));
      local_addr.sin_family = AF_INET;
      local_addr.sin_addr.s_addr = INADDR_ANY;
      local_addr.sin_port = htons(port);

      if (bind(sock, (struct sockaddr *) &local_addr, sizeof(local_addr))) {
        if (errno == EADDRINUSE) {
          warn("UpdEventLoop: port " << port << " is in use");
        }
        else {
          warn("UdpEventLoop: bind() failed: " << strerror(errno));
        }
        if (close(sock)) {
          warn("UdpEventLoop: close() failed while recovering from bind() failure:\n "
               << strerror(errno));
        }
        return -1;
      }

      // Success! Return the socket's file descriptor.
      debugMsg("UdpEventLoop:openSocketOnPort",
               " port " << port << " returning FD " << sock);
      return sock;
    }

    //! Event loop top level.
    //! @param pipeFD File descriptor on which to listen for commands.
    void eventLoop(int pipeFD)
    {
      debugMsg("UdpEventLoop:eventLoop", "(" << pipeFD << ")");
      
      // Set of all FDs watched by the event loop
      fd_set readSet;
      // FD set passed to select()
      fd_set readySet;

      // Startup
      FD_ZERO(&readSet);
      FD_SET(pipeFD, &readSet);
      FD_ZERO(&readySet);

      bool stopped = false;
      do {
        FD_COPY(&readSet, &readySet);
        int nReady = select(getMaxFD() + 1, &readySet, nullptr, nullptr, nullptr);
        if (nReady < 0) {
          warn("UdpEventLoop: select() failed: " << strerror(errno));
          break;
        }
        else if (nReady == 0) {
          // This would be the timeout case, if we had a timeout
          debugMsg("UdpEventLoop:eventLoop", " select() returned 0");
          continue;
        }

        // We know we have at least 1 FD ready
        if (FD_ISSET(pipeFD, &readySet)) {
          debugMsg("UdpEventLoop:eventLoop", " control event");
          // Handle control message or stop request
          ControlMsg request;
          ssize_t nbytes = read(pipeFD, &request, sizeof(ControlMsg));
          if (nbytes < 0) {
            warn("UdpEventLoop: read() from control pipe failed: " << strerror(errno));
            break;
          }
          else if (!nbytes) {
            // EOF on pipe = stop request
            debugMsg("UdpEventLoop:eventLoop", " stop requested");
            stopped = true;
            nReady = 0;
          }
          else if (nbytes != sizeof(ControlMsg)) {
            // OOPS
            warn("UdpEventLoop: control message was wrong size!");
            break;
          }
          else {
            switch(request.op) {
            case OP_ADD:
              addListener(request.port, &readSet);
              break;

            case OP_REMOVE:
              removeListener(request.port, &readSet);
              break;

            default:
              warn("UdpEventLoop: invalid control message!");
              break;
            }

            // Mark this one off and see if we have more FDs ready
            --nReady;
          }
        }
        if (nReady) {
          // Identify FD(s) which became ready
          // and dispatch the incoming datagrams
          for (DescriptorMap::value_type const &pr : m_descriptors) {
            if (FD_ISSET(pr.first, &readySet)) {
              handleFDReady(pr.first, pr.second);
              --nReady;
              if (!nReady)
                break; // from inner for loop
            }
          }
        }

        // Shouldn't be any left over
        if (nReady)
          warn("UdpEventLoop: internal error: failed to handle all ready FDs");

      } while (!stopped);

      if (!stopped) {
        warn("UdpEventLoop: shutting down on error");
      }

      // Close the command pipe
      if (close(pipeFD)) {
        warn("UdpEventLoop: closing control pipe failed: " << strerror(errno));
      }

      // Wipe the file descriptor map.
      // The listeners will be deleted in the foreground.
      m_descriptors.clear();
      debugMsg("UdpEventLoop:eventLoop", " exited");
    }

    // Return the highest FD number monitored by select()
    int getMaxFD()
    {
      if (!m_descriptors.empty() && m_descriptors.rbegin()->first > m_pipeFDs[0])
        return m_descriptors.rbegin()->first;
      return m_pipeFDs[0];
    }

    //! Add the listener registered for the given port.
    //! @param port The port.
    //! @note Must only be called synchronously from the event loop.
    void addListener(in_port_t port, fd_set *readSetPtr)
    {
      debugMsg("UdpEventLoop:addListener", "(" << port << ")");

      // Get the listener from the map.
      Listener *l;
      {
        std::lock_guard<std::mutex> guard(m_listenerMutex);
        l = m_listeners[port].get();
      }
      if (!l) {
        warn("UdpEventLoop::addListener: internal error: not found for port " << port
             << ", ignoring");
        m_sem.post(); // complete, though not successful
        return;
      }

      // Map the file descriptor to the listener
      int fd = l->socketFD;
      m_descriptors[fd] = l;
      // and add it to the wait set.
      FD_SET(fd, readSetPtr);
      // Mark it active
      l->active = true;
      // Notify foreground
      m_sem.post();
      debugMsg("UdpEventLoop:addListener", " port " << port << " succeeded");
    }

    //! Remove the listener on the given port.
    //! @param port The port.
    //! @note Must only be called synchronously from the event loop.
    void removeListener(in_port_t port, fd_set *readSetPtr)
    {
      debugMsg("UdpEventLoop:removeListener", "(" << port << ")");

      // Look up the listener
      Listener *l;
      {
        std::lock_guard<std::mutex> guard(m_listenerMutex);
        l = m_listeners[port].get();
      }
      if (!l) {
        warn("UdpEventLoop::removelistener: internal error: not found for port " << port);
        m_sem.post(); // complete, though not successful
        return;
      }
      
      // Remove the file descriptor from the wait set
      FD_CLR(l->socketFD, readSetPtr);

      // Remove the listener from the descriptor map
      m_descriptors.erase(l->socketFD);

      // Mark the listener inactive
      l->active = false;
      // Notify foreground
      m_sem.post();
      debugMsg("UdpEventLoop:removeListener", " port " << port << " succeeded");
    }

    //! Read from the given file descriptor and dispatch the datagram
    //! to the listener function.
    //! @param fd The file descriptor to read from.
    //! @param listener Pointer to the Listener for this port.
    //! @note Must only be called synchronously from the event loop.
    void handleFDReady(int fd, Listener *listener)
    {
      debugMsg("UdpEventLoop:handleFDReady", " FD " << fd << ", port " << listener->port);

      assertTrue_1(listener);
      ssize_t nbytes = recvfrom(fd, listener->buffer.get(), listener->maxSize,
                                0, // flags
                                reinterpret_cast<struct sockaddr *>(listener->addrBuf.get()),
                                &listener->addrSizeBuf);
      if (nbytes < 0) {
        warn("UdpEventLoop: recvfrom() failed on port " << listener->port << ": " << strerror(errno));
      }
      else if (!nbytes) {
        warn("UdpEventLoop: socket closed unexpectedly on port " << listener->port);
      }
      else {
        (listener->func)(listener->port, listener->buffer.get(), (size_t) nbytes,
                         reinterpret_cast<const struct sockaddr *>(listener->addrBuf.get()),
                         listener->addrSizeBuf);
      }
      debugMsg("UdpEventLoop:handleFDReady", " FD " << fd << " complete");
    }

  }; // class UdpEventLoopImpl

  std::unique_ptr<UdpEventLoop> makeUdpEventLoop()
  {
    return std::make_unique<UdpEventLoopImpl>();
  }

} // namespace PLEXIL
