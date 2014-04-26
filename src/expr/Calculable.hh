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

#ifndef PLEXIL_CALCULABLE_HH
#define PLEXIL_CALCULABLE_HH

#include "ExpressionListener.hh"
#include "Mutable.hh"

namespace PLEXIL
{

  // TODO: Do we need cacheable derivative of this?

  /**
   * @class Calculable
   * @brief An abstract base class.
   * Represents an expression whose value depends on the value(s) of one or more subexpressions.
   */

  class Calculable : public Mutable
  {
  public:
    Calculable();
    ~Calculable();

    /**
     * @brief Add a subexpression to this expression.
     * @param exp The subexpression.
     * @param isGarbage True if this expression should delete the subexpression at destructor time.
     */
    void addSubexpression(const ExpressionId &exp, bool isGarbage = true);

    /**
     * @brief Indicate that this Calculable is complete,
     *        so that the implementation can perform any needed checks.
     * @note The default method does nothing.
     */
    virtual void finalize();

  protected:

    /**
     * @brief Make this expression active.
     * This implementation calls the activate() method on all subexpressions.
     */
    void handleActivate();

    /**
     * @brief Make this expression inactive.
     * This implementation calls the deactivate() method on all subexpressions.
     */
    void handleDeactivate();

  private:
    // Not implemented
    Calculable(const Calculable &);
    Calculable& operator=(const Calculable &);

    class SubexpressionListener : public ExpressionListener
    {
    public:
      SubexpressionListener(Calculable &parent)
      : ExpressionListener(),
        m_parent(parent)
      {
      }

      ~SubexpressionListener()
      {
      }

      void notifyChanged()
      {
        m_parent.notifyChanged();
      }

    private:
      Calculable& m_parent; // parent expression
    };

    SubexpressionListener m_listener;
    std::vector<ExpressionId> m_subexpressions;
    std::vector<bool> m_garbage;
  };

} // namespace PLEXIL

#endif // PLEXIL_CALCULABLE_HH
