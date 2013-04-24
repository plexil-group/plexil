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

//  Copyright Notices

//  This software was developed for use by the U.S. Government as
//  represented by the Administrator of the National Aeronautics and
//  Space Administration. No copyright is claimed in the United States
//  under 17 U.S.C. 105.

//  This software may be used, copied, and provided to others only as
//  permitted under the terms of the contract or other agreement under
//  which it was acquired from the U.S. Government.  Neither title to nor
//  ownership of the software is hereby transferred.  This notice shall
//  remain on all copies of the software.

/**
 * @file IdTable.cc
 * @author Conor McGann
 * @brief Implements IdTable
 * @par Implementation notes
 * @li If system is compiled with PLEXIL_ID_FAST then this class is not used.
 * @li Use the size method as a check to ensure memory is deallocated correctly. On destruction, size should be 0.
 * @li Use the output function to display pointer address and key pairs that have not been deallocated.
 * @li Use debug messages this information in conjunction with the output.
 * @li A dangling pointer failure can be traced by looking for the removal event for a given <pointer, key> pair.
 * @date  July, 2003
 * @see Id<T>
 */

//
// The rest of this file may be ignored if PLEXIL_ID_FAST is defined.
//

#include "IdTable.hh"
#if !defined(PLEXIL_ID_FAST)

#ifdef ID_TABLE_DEBUG
#include "Debug.hh"
#endif

namespace PLEXIL {

  IdTable::IdTable() {
  }

  IdTable::~IdTable() {
  }

  ID_KEY_TYPE IdTable::insert(ID_POINTER_TYPE id, const char*
#ifdef ID_TABLE_DEBUG
                              baseType
#endif
                              )
  {
    static ID_KEY_TYPE* sl_nextId = NULL;
    
    IdTable& instance(getInstance());
    ThreadMutexGuard guard(instance.m_mutex);

    if (sl_nextId == NULL)
      sl_nextId = new ID_KEY_TYPE(1);

#ifdef ID_TABLE_DEBUG
    debugMsg("IdTable:insert", "id,key:" << id << ", " << *sl_nextId << ")");
#endif
    IdTableMap::iterator it = instance.m_collection.find(id);
    if (it != instance.m_collection.end())
      return(0); /* Already in table. */
    instance.m_collection.insert(IdTablePair(id, *sl_nextId));
#ifdef ID_TABLE_DEBUG
    PLEXIL_HASH_MAP(std::string, ID_SIZE_TYPE)::iterator tCit = instance.m_typeCnts.find(baseType);
    if (tCit == instance.m_typeCnts.end())
      instance.m_typeCnts.insert(std::pair<std::string, ID_SIZE_TYPE>(baseType, 1));
    else
      tCit->second++;
#endif
    return((*sl_nextId)++);
  }

  ID_KEY_TYPE IdTable::getKey(ID_POINTER_TYPE id) {
    IdTable& instance(getInstance());
    ThreadMutexGuard guard(instance.m_mutex);

    IdTableMap::iterator it = instance.m_collection.find(id);
    if (it != instance.m_collection.end())
      return(it->second);
    else
      return(0);
  }

  void IdTable::remove(ID_POINTER_TYPE id) {
    IdTable& instance(getInstance());
    ThreadMutexGuard guard(instance.m_mutex);

    // N.B. sl_key is only used in debug message output;
    // the risk of harm from thread collision here is very low.
#ifdef ID_TABLE_DEBUG
    static ID_KEY_TYPE sl_key;
    debugMsg("IdTable:remove", "<" << id << ", " << (sl_key = instance.m_collection.find(id)->second) << ">");
#endif
    instance.m_collection.erase(id);
  }

  size_t IdTable::size() {
    ThreadMutexGuard guard(getInstance().m_mutex);
    return(getInstance().m_collection.size());
  }

#ifdef ID_TABLE_DEBUG
  void IdTable::printTypeCnts(std::ostream& os) {
    os << "Id instances by type: ";
    for (PLEXIL_HASH_MAP(std::string, ID_SIZE_TYPE)::iterator it = getInstance().m_typeCnts.begin();
         it != getInstance().m_typeCnts.end();
         ++it)
      os << "  " << it->second << "  " << it->first << '\n';
    os << std::endl;
  }
#endif

  void IdTable::output(std::ostream& os) {
    IdTable& instance(getInstance());
    ThreadMutexGuard guard(instance.m_mutex);
    os << "Id Contents:";
    for (IdTableMap::iterator it = instance.m_collection.begin();
         it != instance.m_collection.end();
         ++it)
      os << " (" << it->first << ", " << it->second << ')';
    os << std::endl;
  }

  IdTable& IdTable::getInstance() {
    static IdTable* sl_instance = NULL;
    if (sl_instance == NULL)
      sl_instance = new IdTable();
    return *sl_instance;
  }
}

#endif // !defined(PLEXIL_ID_FAST)
