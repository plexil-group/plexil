#ifndef _H__ReadWrite
#define _H__ReadWrite

#include "ThreadMutex.hh"

// Implements a write-favoring read-write lock
class ReadWriteLock{
public:

  ReadWriteLock() : r_count(0){}
  
  void begin_read(){
    // Block if there is a writer writing
    turn_lock.lock();
    turn_lock.unlock();
    r_lock.lock();
    r_count++;
    if(r_count == 1){ // First in
      w_lock.lock();
    }
    r_lock.unlock();
  }

  void end_read(){
    r_lock.lock();
    r_count--;
    if(r_count == 0){ // Last out
      w_lock.unlock();
    }
    r_lock.unlock();
  }

  void begin_write(){
    turn_lock.lock();
    w_lock.lock();
  }

  void end_write(){
    turn_lock.unlock();
    w_lock.unlock();
  }
private:
  int r_count; // Count of readers
  PLEXIL::ThreadMutex r_lock; // Protects access to r_count
  PLEXIL::ThreadMutex w_lock; // Protects writes to data
  PLEXIL::ThreadMutex turn_lock; // Write is awaiting a turn
};




#endif
