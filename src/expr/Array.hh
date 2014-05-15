/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_ARRAY_HH
#define PLEXIL_ARRAY_HH

#include <vector>

namespace PLEXIL
{
  // Forward references
  template <class T> class MutableArrayReference;

  /**
   * @class Array
   * @brief Templatized class implementing the PLEXIL notion of an Array.
   * @note This is an initial implementation, internals subject to change.
   */
  template <typename T>
  class Array
  {
    friend class MutableArrayReference<T>;

  public:
    Array();
    Array(Array const &);
    Array(size_t size);
    Array(std::vector<T> const &initval);

    ~Array();

    Array &operator=(Array const &);

    // Accessors

    size_t size() const;
    bool elementKnown(size_t index) const;
    bool getElement(size_t index, T &result) const;

    // Setters

    /**
     * @brief Expand the array to the requested size. 
     *        Mark the new elements as unknown.
     *        If already that size or larger, does nothing.
     * @param size The requested size.
     */
    void resize(size_t size);
    void setElement(size_t index, T const &newVal);
    void setElementUnknown(size_t index);

    template <typename U>
    friend bool operator==(Array<U> const &a, Array<U> const &b);

    // Const accessors to data
    inline std::vector<T> const &getContentsVector() const
    {
      return m_contents;
    }

    inline std::vector<bool> const &getKnownVector() const
    {
      return m_known;
    }

  private:
    inline bool checkIndex(size_t index) const
    {
      return index < m_contents.size();
    }

    std::vector<T> m_contents;
    std::vector<bool> m_known;
  };

  template <typename T>
  bool operator==(Array<T> const &a, Array<T> const &b);

  template <typename T>
  bool operator!=(Array<T> const &a, Array<T> const &b);

  //
  // Convenience typedefs
  //
  typedef Array<bool>        BooleanArray;
  typedef Array<int32_t>     IntegerArray;
  typedef Array<double>      RealArray;
  typedef Array<std::string> StringArray;


} // namespace PLEXIL

#endif // PLEXIL_ARRAY_HH
