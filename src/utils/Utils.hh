/* Copyright (c) 2006-2013, Universities Space Research Association (USRA).
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

#ifndef _H_Utils
#define _H_Utils

/**
 * @file   Utils.hh
 * @author Conor McGann
 * @date   Mon Dec 27 17:19:39 2004
 * @brief  
 * @ingroup Utils
 */

#include <list>
#include <set>
#include <string>
#include <vector>

#include "Id.hh"

namespace PLEXIL {

  /**
   * @brief Case insensitive string compare.
   */
  bool compareIgnoreCase(const std::string & s1,
                         const std::string & s2);

  template<class TYPE>
  void cleanup(std::set<Id<TYPE> >& objects){
    typedef typename std::set<Id<TYPE> >::const_iterator object_iterator;
    object_iterator it = objects.begin();
    while(it != objects.end()){
      check_error((*it).isValid());
      delete (TYPE*) (*it++);
    }
    objects.clear();
  }

  template<class TYPE>
  void cleanup(std::vector<Id<TYPE> >& objects){
    typedef typename std::vector<Id<TYPE> >::const_iterator object_iterator;
    object_iterator it = objects.begin();
    while(it != objects.end()){
      checkError((*it).isValid(), *it);
      delete (TYPE*) (*it++);
    }
    objects.clear();
  }


  template<class TYPE>
  void cleanup(std::list<Id<TYPE> >& objects){
    typedef typename std::list<Id<TYPE> >::const_iterator object_iterator;
    object_iterator it = objects.begin();
    while(it != objects.end()){
      Id<TYPE> object = *it;
      if(!object.isNoId()){
    check_error(object.isValid());
    delete (TYPE*) (*it++);
      }
      else
    ++it;
    }
    objects.clear();
  }

  template<class TYPE1, class TYPE2>
  void cleanup(std::map<TYPE1, Id<TYPE2> >& objects){
    typedef typename std::map<TYPE1, Id<TYPE2> >::const_iterator object_iterator;
    object_iterator it = objects.begin();
    while(it != objects.end()){
      Id<TYPE2> item = (it++)->second;
      check_error(item.isValid());
      delete (TYPE2*) item;
    }
    objects.clear();
  }

  template<class TYPE1, class TYPE2>
  void cleanup(std::multimap<TYPE1, Id<TYPE2> >& objects){
    typedef typename std::multimap<TYPE1, Id<TYPE2> >::const_iterator object_iterator;
    object_iterator it = objects.begin();
    while(it != objects.end()){
      Id<TYPE2> item = (it++)->second;
      check_error(item.isValid());
      delete (TYPE2*) item;
    }
    objects.clear();
  }

  template<class TYPE>
  void cleanup(std::list<TYPE*>& objects){
    typedef typename std::list<TYPE*>::const_iterator object_iterator;
    for(object_iterator it = objects.begin(); it != objects.end(); ++it){
      TYPE* element = *it;
      delete element;
    }
    objects.clear();
  }

  template<class TYPE>
  void cleanup(std::vector<TYPE*>& objects){
    typedef typename std::vector<TYPE*>::const_iterator object_iterator;
    for(object_iterator it = objects.begin(); it != objects.end(); ++it){
      TYPE* element = *it;
      delete element;
    }
    objects.clear();
  }

  template<class TYPE1, class TYPE2>
  void cleanup(std::map<TYPE1, TYPE2*>& objects){
    typedef typename std::map<TYPE1, TYPE2*>::const_iterator object_iterator;
    object_iterator it = objects.begin();
    while(it != objects.end()){
      TYPE2* item = (it++)->second;
      check_error(item != NULL);
      delete item;
    }
    objects.clear();
  }
}
#endif
