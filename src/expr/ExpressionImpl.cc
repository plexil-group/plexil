#include "ExpressionImpl.hh"
#include "Error.hh"

namespace PLEXIL
{
  template <typename T>
  template <typename U>
  bool ExpressionImpl<T>::getValueImpl(U &result) const
  {
    check_error_2(ALWAYS_FAIL, "getValue type error");
    return false;
  }

  // More conversions can be added as required.
  template <>
  template <>
  bool ExpressionImpl<int32_t>::getValueImpl(double &result) const
  {
    int32_t temp;
    if (!this->getValueImpl(temp))
      return false;
    result = (double) temp;
    return true;
  }
}
