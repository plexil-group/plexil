#ifndef PLEXIL_EXPRESSION_IMPL_HH
#define PLEXIL_EXPRESSION_IMPL_HH

#include "Expression.hh"

namespace PLEXIL
{

  //
  // Wrapper providing type conversions for getValue for templatized Expression derivatives
  //
  // If you wonder what's going on here, search for "Curiously Recurring Template Pattern".
  //

  template <class C>
  class ExpressionWrapper : public virtual Expression
  {
  public:
    // The only boilerplate code in this approach.
    bool getValue(bool &result) const
    {
      return static_cast<const C *>(this)->getValueImpl(result);
    }

    bool getValue(uint16_t &result) const
    {
      return static_cast<const C *>(this)->getValueImpl(result);
    }

    bool getValue(int32_t &result) const
    {
      return static_cast<const C *>(this)->getValueImpl(result);
    }

    bool getValue(double &result) const
    {
      return static_cast<const C *>(this)->getValueImpl(result);
    }

    bool getValue(std::string &result) const
    {
      return static_cast<const C *>(this)->getValueImpl(result);
    }
  };

  //
  // Templatized Expression implementations should derive from this template
  //

  template <typename T>
  class ExpressionImpl : public ExpressionWrapper<ExpressionImpl<T> >
  {
  public:
    virtual bool getValueImpl(T &result) const = 0; 

    // Conversion wrapper
    template <typename U>
    bool getValueImpl(U &result) const;
  };
  
}

#endif // PLEXIL_EXPRESSION_IMPL_HH
