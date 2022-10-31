// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLEXIL_UPDATE_IMPL_HH
#define PLEXIL_UPDATE_IMPL_HH

#include "SimpleBooleanVariable.hh"
#include "Update.hh"

namespace PLEXIL
{
  // Forward declarations
  class NodeConnector;
  struct Pair;

  //! \class UpdateImpl
  //! \brief Implements the Update API.
  //! \see Update
  //! \see UpdateNode
  class UpdateImpl final : public Update
  {
  public:

    //! \brief Constructor.
    //! \param node Pointer to the owning UpdateNode.
    UpdateImpl(NodeConnector *node);

    //! \brief Virtual destructor.
    virtual ~UpdateImpl();

    //
    // API to external interfaces
    //

    //! \brief Get the map of name-value pairs.
    //! \return Const reference to the map.
    virtual const Update::PairValueMap &getPairs() const;

    //! \brief Get the node ID of the owning node.
    //! \return Const pointer to the node, as a NodeConnector.
    virtual std::string const &getNodeId() const;

    //! \brief Return an acknowledgement value for the update.
    //! \param ack The value.
    virtual void acknowledge(bool ack);

    //
    // LinkedQueue item API
    //

    //! \brief Get the pointer to the next Update in the queue.
    //! \return Pointer to the Update.
    virtual Update *next() const;

    //! \brief Get the pointer to the pointer to the next Update in the queue.
    //! \return Pointer to the next poitner.
    virtual Update **nextPtr();

    //
    // Parser API
    //

    //! \brief Reserve space in the map for the given number of name-expression pairs.
    //! \param n The requested size.
    void reservePairs(size_t n);

    //! \brief Add a name-expression pair to be reported.
    //! \param name Const reference to the name.
    //! \param exp Pointer to the expression.
    //! \param expIsGarbage true if the expression should be deleted with the Update, false if not.
    void addPair(std::string const &name, Expression *exp, bool expIsGarbage);

    //
    // API to UpdateNode
    //

    //! \brief Get the acknowledgement variable for this Update.
    //! \return Pointer to the variable, as an Expression.
    Expression *getAck();

    //! \brief Make the Update active.
    void activate();

    //! \brief Make the Update inactive.
    void deactivate();

    //! \brief Evaluate all the expressions and fix their result values.
    //! \note Public for use by parser unit test.
    virtual void fixValues();

    //! \brief Delete all owned expressions, erase the pointers to the ones not owned.
    void cleanUp();

  private:

    // Default, copy, move constructors, assignment operators unimplemented
    UpdateImpl() = delete;
    UpdateImpl(UpdateImpl const &) = delete;
    UpdateImpl(UpdateImpl &&) = delete;
    UpdateImpl& operator=(UpdateImpl const &) = delete;
    UpdateImpl& operator=(UpdateImpl &&) = delete;

    //! \brief Map of name-value pairs.
    Update::PairValueMap m_valuePairs;

    //! \brief the acknowledgement variable.
    SimpleBooleanVariable m_ack;

    //! \brief Next pointer for LinkedQueue.
    Update *m_next;

    //! \brief Pointer to linked list of name-expression pairs.
    Pair *m_pairs;

    //! \brief The owning Node.
    NodeConnector *m_node;
  };

} // namespace PLEXIL

#endif // PLEXIL_UPDATE_IMPL_HH
