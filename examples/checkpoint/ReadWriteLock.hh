#ifndef _H__ReadWrite
#define _H__ReadWrite

#include <mutex>
using std::mutex;


// Implements Michel Raynal's read-write lock
class ReadWriteLock{
public:
  void begin_read(){
    r.lock();
    r_count++;
    if(r_count==1) g.lock();
    r.unlock();
  }

  void end_read(){
    r.lock();
    r_count--;
    if(r_count==0) g.unlock();
    r.unlock();
  }

  void begin_write(){
    g.lock();
  }

  void end_write(){
    g.unlock();
  }
private:
  int r_count = 0; // Count of readers
  mutex r; // Protects reader count
  mutex g; // Global exclusion of writers

};




#endif
