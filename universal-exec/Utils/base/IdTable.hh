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

#ifndef _H_IdTable
#define _H_IdTable

#include <map>
#include <iostream>
#include <string>

/**
 * @file IdTable.hh
 * @author Conor McGann
 * @brief Defines a singleton class managing allocation and deallocation of ids for pointers.
 * @date  July, 2003
 * @see Id, IdManager
*/

namespace PLEXIL {

  /**
   * @class IdTable
   * @brief Provides a singleton which manages <pointer,key> pairs.
   *
   * Main data structure is a map of integer pairs. The map is accessed
   * by an integer which should be the address of an object managed by an Id. A key is used to
   * check for allocations of an Id to a previously allocated address. This is necessary so that dangling
   * Ids can be detected even if the address has been recycled.
   * @see Id
   */
  class IdTable {
  public:
    static unsigned int size();
    static std::map<unsigned int, unsigned int> getCollection();
    static IdTable& getInstance();

    /**
     * Print the number of times each type has been allocated.
     */
    static void printTypeCnts(std::ostream& os);

    static void output(std::ostream& os);
    static unsigned int insert(unsigned int id, const char* baseType);
    static bool allocated(unsigned int id);
    static unsigned int getKey(unsigned int id);
    static void remove(unsigned int id);

    ~IdTable(); // deallocating statics requires public access on beos
  private:
    IdTable();
    std::map<unsigned int, unsigned int> m_collection;
    std::map<std::string, unsigned int> m_typeCnts;
  };
}

#endif
