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

#include <string>
#include <vector>

namespace PLEXIL
{
  // Forward references
  template <typename T> class MutableArrayReference;

  /**
   * @class Array
   * @brief Abstract base class representing the PLEXIL API of an Array.
   * @note This is an initial implementation, internals subject to change.
   */
  class Array
  {
    friend class MutableArrayReference<bool>;
    friend class MutableArrayReference<int32_t>;
    friend class MutableArrayReference<double>;
    friend class MutableArrayReference<std::string>;

  public:
    Array();
    Array(Array const &);
    // For internal use
    Array(size_t size, bool known);

    virtual ~Array();

    Array &operator=(Array const &);

    // Generic accessors

    size_t size() const;
    bool elementKnown(size_t index) const;
    bool allElementsKnown() const;
    bool anyElementsKnown() const;
    inline std::vector<bool> const &getKnownVector() const
    {
      return m_known;
    }


    // Generic setters

    /**
     * @brief Expand the array to the requested size. 
     *        Mark the new elements as unknown.
     *        If already that size or larger, does nothing.
     * @param size The requested size.
     */
    virtual void resize(size_t size);
    void setElementUnknown(size_t index);

    // Typed accessors
    virtual bool getElement(size_t index, bool &result) const = 0;
    virtual bool getElement(size_t index, int32_t &result) const = 0;
    virtual bool getElement(size_t index, double &result) const = 0;
    virtual bool getElement(size_t index, std::string &result) const = 0;

    virtual bool getElementPointer(size_t index, std::string const *&result) const = 0;

    virtual void getContentsVector(std::vector<bool> const *&result) const = 0;
    virtual void getContentsVector(std::vector<int32_t> const *&result) const = 0;
    virtual void getContentsVector(std::vector<double> const *&result) const = 0;
    virtual void getContentsVector(std::vector<std::string> const *&result) const = 0;

    // Typed setters
    virtual void setElement(size_t index, bool const &newVal) = 0;
    virtual void setElement(size_t index, int32_t const &newVal) = 0;
    virtual void setElement(size_t index, double const &newVal) = 0;
    virtual void setElement(size_t index, std::string const &newVal) = 0;

    // Utility
    virtual void print(std::ostream &s) const = 0;

  protected:
    // For use by implementation classes
    inline bool checkIndex(size_t index) const
    {
      return index < m_known.size();
    }

    std::vector<bool> m_known;
  };

  bool operator==(Array const &a, Array const &b);

  //
  // Convenience typedefs
  //
  template <typename T> class ArrayImpl;
  typedef ArrayImpl<bool>        BooleanArray;
  typedef ArrayImpl<int32_t>     IntegerArray;
  typedef ArrayImpl<double>      RealArray;
  typedef ArrayImpl<std::string> StringArray;

} // namespace PLEXIL

#endif // PLEXIL_ARRAY_HH
