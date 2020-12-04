/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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


#include "ArrayImpl.hh"
#include "LookupReceiver.hh"
#include "StateCacheEntry.hh"
#include "Value.hh"

namespace PLEXIL
{

  class LookupReceiverImpl final : public LookupReceiver
  {
  public:

    LookupReceiverImpl(StateCacheEntry &e) :
      LookupReceiver(),
      m_entry(e)
    {
    }

    virtual ~LookupReceiverImpl() = default;

    virtual void update(Value const &v)
    {
      m_entry.update(v);
    }

    virtual void setUnknown()
    {
      m_entry.setUnknown();
    }

    //
    // Convenience overloads
    //

    virtual void update(Boolean b)
    {
      m_entry.update(b);
    }

    virtual void update(Integer i)
    {
      m_entry.update(i);
    }

    virtual void update(Real x)
    {
      m_entry.update(x);
    }

    virtual void update(std::string const &s)
    {
      m_entry.update(s);
    }

    virtual void update(char const *s)
    {
      m_entry.update(String(s));
    }

    virtual void update(Boolean const ary[], size_t size)
    {
      std::vector<Boolean> v(size);
      for (size_t i = 0; i < size; ++i)
        v[i] = ary[i];
      BooleanArray array(v);
      m_entry.updatePtr(&array);
    }

    virtual void update(Integer const ary[], size_t size)
    {
      std::vector<Integer> v(size);
      for (size_t i = 0; i < size; ++i)
        v[i] = ary[i];
      IntegerArray array(v);
      m_entry.updatePtr(&array);
    }

    virtual void update(Real const ary[], size_t size)
    {
      std::vector<Real> v(size);
      for (size_t i = 0; i < size; ++i)
        v[i] = ary[i];
      RealArray array(v);
      m_entry.updatePtr(&array);
    }

    virtual void update(String const ary[], size_t size)
    {
      std::vector<String> v(size);
      for (size_t i = 0; i < size; ++i)
        v[i] = ary[i];
      StringArray array(v);
      m_entry.updatePtr(&array);
    }

  private:
    // Not implemented
    LookupReceiverImpl() = delete;
    LookupReceiverImpl(const LookupReceiverImpl &) = delete;
    LookupReceiverImpl(LookupReceiverImpl &&) = delete;

    LookupReceiverImpl &operator=(const LookupReceiverImpl &) = delete;
    LookupReceiverImpl &operator=(LookupReceiverImpl &&) = delete;

    // Reference to parent cache entry
    StateCacheEntry &m_entry;
  };

  LookupReceiver *makeLookupReceiver(StateCacheEntry &e)
  {
    return new LookupReceiverImpl(e);
  }

} // namespace PLEXIL
