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

#include "IdTable.hh"
#include "Debug.hh"

/**
 * @file IdTable.cc
 * @author Conor McGann
 * @brief Implements IdTable
 * @par Implementation notes
 * @li If system is compiled with PLEXIL_FAST then this class is not used.
 * @li Use the size method as a check to ensure memory is deallocated correctly. On destruction, size should be 0.
 * @li Use the output function to display pointer address and key pairs that have not been deallocated.
 * @li Use debug messages this information in conjunction with the output.
 * @li A dangling pointer failure can be traced by looking for the removal event for a given <pointer, key> pair.
 * @date  July, 2003
 * @see Id<T>
 */

namespace PLEXIL {

  IdTable::IdTable() {
  }

  IdTable::~IdTable() {
  }

  ID_KEY_TYPE IdTable::insert(ID_POINTER_TYPE id, const char* baseType) {
    static ID_KEY_TYPE* sl_nextId = NULL;
    if (sl_nextId == NULL)
      sl_nextId = new ID_KEY_TYPE(1);

    debugMsg("IdTable:insert", "id,key:" << id << ", " << *sl_nextId << ")");
    std::map<ID_POINTER_TYPE, ID_KEY_TYPE>::iterator it = getInstance().m_collection.find(id);
    if (it != getInstance().m_collection.end())
      return(0); /* Already in table. */
    getInstance().m_collection.insert(std::pair<ID_POINTER_TYPE, ID_KEY_TYPE>(id, *sl_nextId));
#ifdef ID_TABLE_DEBUG
    std::map<std::string, ID_SIZE_TYPE>::iterator tCit = getInstance().m_typeCnts.find(baseType);
    if (tCit == getInstance().m_typeCnts.end())
      getInstance().m_typeCnts.insert(std::pair<std::string, ID_SIZE_TYPE>(baseType, 1));
    else
      tCit->second++;
#endif
    return((*sl_nextId)++);
  }

  bool IdTable::allocated(ID_POINTER_TYPE id) {
    return(getInstance().m_collection.find(id) != getInstance().m_collection.end());
  }

  ID_KEY_TYPE IdTable::getKey(ID_POINTER_TYPE id) {
    std::map<uintptr_t, uintptr_t>::iterator it = getInstance().m_collection.find(id);
    if (it != getInstance().m_collection.end())
      return(it->second);
    else
      return(0);
  }

  void IdTable::remove(ID_POINTER_TYPE id) {
    // N.B. sl_key is only used in debug message output;
    // the risk of harm from thread collision here is very low.
    static ID_KEY_TYPE sl_key;
    debugMsg("IdTable:remove", "<" << id << ", " << (sl_key = getInstance().m_collection.find(id)->second) << ">");
    getInstance().m_collection.erase(id);
  }

  size_t IdTable::size() {
    return(getInstance().m_collection.size());
  }

  std::map<uintptr_t, uintptr_t> IdTable::getCollection() {
    return(getInstance().m_collection);
  }

#ifdef ID_TABLE_DEBUG
  void IdTable::printTypeCnts(std::ostream& os) {
    os << "Id instances by type: ";
    for (std::map<std::string, uintptr_t>::iterator it = getInstance().m_typeCnts.begin();
         it != getInstance().m_typeCnts.end();
         ++it)
      os << "  " << it->second << "  " << it->first << '\n';
    os << std::endl;
  }
#endif

  void IdTable::output(std::ostream& os) {
    os << "Id Contents:";
    for (std::map<uintptr_t, uintptr_t>::iterator it = getInstance().m_collection.begin();
         it != getInstance().m_collection.end();
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
