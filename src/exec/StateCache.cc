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

#include "StateCache.hh"

#include "Debug.hh"
#include "Expression.hh"
#include "Expressions.hh"
#include "ExternalInterface.hh"

#include <cmath> // for fabs()
#include <iomanip> // for setprecision()
#include <limits>

namespace PLEXIL
{

  //
  // Helper function
  //
  double differenceMagnitude(double x, double y)
  {
	if (x == Expression::UNKNOWN()) {
	  if (y == Expression::UNKNOWN())
		return 0;
	  else
		return std::numeric_limits<double>::max();
	}
	else if (y == Expression::UNKNOWN())
	  return std::numeric_limits<double>::max();
	else
	  return fabs(x - y);
  }

  //
  // Helper classes
  //

  class CacheEntry
  {
  public:
	CacheEntry(const State& s)
	: state(s),
	  value(Expression::UNKNOWN()),
	  highThreshold(Expression::UNKNOWN()),
	  lowThreshold(Expression::UNKNOWN()),
	  lastQuiescence(-1),
	  m_id(this)
	{
	}

	~CacheEntry()
	{
	  m_id.remove();
	}

	const CacheEntryId & getId() const
	{
	  return m_id; 
	}


	/**
	 * @brief Returns true if any change lookups are active on this state.
	 */
	bool activeChangeLookups() const
	{
	  for (std::set<LookupDescId>::const_iterator it = lookups.begin();
		   it != lookups.end();
		   it++) {
		if ((*it)->changeLookup)
		  return true;
	  }
	  return false;
	}

	/**
	 * @brief Update all lookups with the new value.
	 * @param newValue The latest value for this state.
	 * @param quiescenceCount The current quiescence count.
	 * @return True if the thresholds have changed, false otherwise.
	 */
	bool update(double newValue, int quiescenceCount)
	{
	  value = newValue;
	  lastQuiescence = quiescenceCount;

	  for (std::set<LookupDescId>::const_iterator lit = lookups.begin();
		   lit != lookups.end();
		   lit++) {
		LookupDescId lookup = *lit;
		// check_error(lookup.isValid()); // *** only if paranoid
		updateLookup(lookup, value);
	  }

	  return calculateThresholds();
	}

	// N.B. Lookup need not be a change lookup.
	void updateLookup(LookupDescId lookup, double value)
	{
	  if (differenceMagnitude(lookup->previousValue, value) < lookup->tolerance) {
		debugMsg("StateCache:updateState", 
				 "Not updating. All changes are within the tolerance.");
		return;
	  }
	  if (lookup->previousValue == Expression::UNKNOWN()
		  && value != Expression::UNKNOWN()) {
		  debugMsg("StateCache:updateState", "Updating because the previous value is UNKNOWN.");
	  }
	  else {
		debugMsg("StateCache:updateState",
				 "Updating because the change in value exceeds tolerance " << 
				 Expression::valueToString(lookup->tolerance));
	  }
	  check_error(lookup->dest.isValid());
	  debugMsg("StateCache:updateState", "Updating change lookup " <<
			   lookup->dest->toString() << " from " <<
			   Expression::valueToString(lookup->previousValue)
			   << " to " << Expression::valueToString(value));
	  lookup->dest->setValue(value);
	  lookup->previousValue = value;
	}

	bool calculateThresholds()
	{
	  debugMsg("CacheEntry:calculateThresholds",
			   " for " << LabelStr(state.first).toString() << ", " << lookups.size() << " active lookups");
	  double newHi = Expression::UNKNOWN();
	  double newLo = Expression::UNKNOWN();

	  for (std::set<LookupDescId>::const_iterator lit = lookups.begin();
		   lit != lookups.end();
		   lit++) {
		LookupDescId lookup = *lit;
		// check_error(lookup.isValid()); // *** only if paranoid
		condDebugMsg(!lookup->changeLookup,
					 "CacheEntry:calculateThresholds",
					 " entry is not a change lookup");
		if (lookup->changeLookup && lookup->tolerance != 0.0) {
		  // Update thresholds based on lookup's last value
		  double hi = lookup->previousValue + lookup->tolerance;
		  if (newHi == Expression::UNKNOWN()
			  || hi < newHi) {
			debugMsg("CacheEntry:calculateThresholds",
					 " updating high threshold to " << Expression::valueToString(hi));
			newHi = hi;
		  }
		  double lo = lookup->previousValue - lookup->tolerance;
		  if (newLo == Expression::UNKNOWN()
			  || lo > newLo) {
			debugMsg("CacheEntry:calculateThresholds",
					 " updating low threshold to " << Expression::valueToString(lo));
			newLo = lo;
		  }
		}
	  }

	  bool changed = false;
	  if (newHi != Expression::UNKNOWN()
		  && newHi != highThreshold) {
		highThreshold = newHi;
		changed = true;
	  }
	  if (newLo != Expression::UNKNOWN()
		  && newLo != lowThreshold) {
		lowThreshold = newLo;
		changed = true;
	  }
	  return changed;
	}

	State state;
	std::set<LookupDescId> lookups;
	double value;
	double highThreshold;
	double lowThreshold;
	int lastQuiescence;

  private:
	CacheEntryId m_id;
  };

  StateCache::StateCache() 
	: m_id(this),
	  m_inQuiescence(false), 
	  m_quiescenceCount(0)
  {
	// Initialize the time state
	m_timeState.first = LabelStr("time").getKey();
	m_timeEntry = ensureCacheEntry(m_timeState);
	m_timeEntry->value = 0.0;
  }

  // FIXME: should this unsubscribe from interface?
  StateCache::~StateCache()
  {
	// Delete states
	m_timeEntry = CacheEntryId::noId();
	for (StateCacheMap::const_iterator it = m_states.begin();
		 it != m_states.end();
		 it++) {
	  delete (CacheEntry*) it->second;
	}
	m_states.clear();

	// Delete lookups
	for (ExpressionToLookupMap::iterator it = 
		   m_lookupsByExpression.begin(); 
		 it != m_lookupsByExpression.end();
		 ++it) {
	  delete (LookupDesc*) it->second;
	}
	m_lookupsByExpression.clear();

	m_id.remove();
  }
   
  void StateCache::registerLookupNow(const ExpressionId& expr,
									 const State& state)
  {
	check_error(m_inQuiescence, "Lookup outside of quiescence!");
	debugMsg("StateCache:lookupNow",
			 "Looking up value for state " << toString(state)
			 << " because of " << expr->toString());

	// Register the lookup for updates as long as it's active
	CacheEntryId entry = ensureCacheEntry(state);
	LookupDescId lookup = (new LookupDesc(entry, expr, 0.0, false))->getId();
	m_lookupsByExpression[expr] = lookup;
	entry->lookups.insert(lookup);

	if (entry->lastQuiescence < m_quiescenceCount) {
	  debugMsg("StateCache:registerLookupNow",
			   (entry->lastQuiescence == -1 ? "New" : "Stale")
			   << " state, so performing external lookup.");
	  // Perform the lookup and propagate to anyone else listening to this state
	  // Tell interface if change thresholds moved in the process
	  if (internalStateUpdate(entry, m_interface->lookupNow(entry->state))
		  && entry->activeChangeLookups())
		m_interface->setThresholds(state, entry->highThreshold, entry->lowThreshold);
	}
	// state is known and the cached value is current - return it
	else {
	  debugMsg("StateCache:lookupNow", 
			   "Already have up-to-date value for state, so using that ("
			   << Expression::valueToString(entry->value) << ")");

	  expr->setValue(entry->value);
	}
  }

  void StateCache::registerChangeLookup(const ExpressionId& expr,
										const State& state,
										double tolerance)
  {
	check_error(m_inQuiescence, "Lookup outside of quiescence!");
	debugMsg("StateCache:registerChangeLookup", "Registering change lookup " <<
			 expr->toString() << " for state " << toString(state) <<
			 " with tolerance (" << Expression::valueToString(tolerance) << ")");

	CacheEntryId entry = ensureCacheEntry(state);
	bool wasSubscribed = entry->activeChangeLookups();
	LookupDescId lookup = (new LookupDesc(entry, expr, tolerance, true))->getId();
	m_lookupsByExpression[expr] = lookup;
	entry->lookups.insert(lookup);
	if (!wasSubscribed) {
	  // Tell the external interface to listen for changes on this state
	  // FIXME: allow for exception here?
	  m_interface->subscribe(entry->state);
	}

	bool thresholdUpdate = false;
	// Update the current value of the expression if needed
	if (entry->lastQuiescence < m_quiescenceCount) {
	  debugMsg("StateCache:registerChangeLookup",
			   (entry->lastQuiescence == -1 ? "New" : "Stale")
			   << " state, so performing external lookup.");
	  thresholdUpdate = 
		internalStateUpdate(entry, m_interface->lookupNow(entry->state));
	}
	else {
	  debugMsg("StateCache:registerChangeLookup",
			   "Already have up-to-date value for state, so using that (" <<
			   Expression::valueToString(entry->value) << ")");
	  lookup->previousValue = entry->value;
	  expr->setValue(entry->value);
	  thresholdUpdate = entry->calculateThresholds();
	}
	// Update thresholds if required
	if (!wasSubscribed || thresholdUpdate) {
	  m_interface->setThresholds(state, entry->highThreshold, entry->lowThreshold);
	}
	else {
	  debugMsg("StateCache:registerChangeLookup", " not updating thresholds");
	}
  }

  // FIXME: potential for race conditions on CacheEntry lookups?

  CacheEntryId StateCache::internalUnregisterLookup(const ExpressionId& source)
  {
	check_error(m_inQuiescence, "Lookup outside of quiescence!");
	check_error(source.isValid());
	ExpressionToLookupMap::iterator eit = m_lookupsByExpression.find(source);
	checkError(eit != m_lookupsByExpression.end(),
			   "No stored lookup for " << source->toString());
	LookupDescId lookup = eit->second;
	CacheEntryId entry = lookup->entry;
	checkError(entry.isId(), "No state entry for " << source->toString());
	entry->lookups.erase(lookup);
	m_lookupsByExpression.erase(eit);
	delete (LookupDesc*) lookup;
	return entry;
  }

  void StateCache::unregisterChangeLookup(const ExpressionId& source)
  {
	debugMsg("StateCache:unregisterChangeLookup",
			 " Removing change lookup " << source->toString());
	CacheEntryId entry = internalUnregisterLookup(source);
	check_error(entry.isId());
	if (!entry->activeChangeLookups()) {
	  m_interface->unsubscribe(entry->state);
	  entry->highThreshold = entry->lowThreshold = Expression::UNKNOWN();
	}
  }

  void StateCache::unregisterLookupNow(const ExpressionId& source)
  {
	internalUnregisterLookup(source);
  }

  void StateCache::updateState(const State& state, double value)
  {
	check_error(!m_inQuiescence);
	StateCacheMap::const_iterator it = m_states.find(state);
	if (it == m_states.end()) {
	  debugMsg("StateCache:updateState",
			   "Received update for unknown state " << toString(state));
	  return;
	}
	CacheEntryId entry = it->second;
	if (internalStateUpdate(entry, value) && entry->activeChangeLookups())
	  m_interface->setThresholds(state, entry->highThreshold, entry->lowThreshold);
  }

  bool StateCache::internalStateUpdate(const CacheEntryId& entry,
									   double value)
  {
	debugMsg("StateCache:updateState",
			 "Updating state " << toString(entry->state) <<
			 " with value " << Expression::valueToString(value));
	return entry->update(value, m_quiescenceCount);
  }


  /**
   * @brief Find or create the cache entry for this state.
   * @param state The state being looked up.
   * @return Pointer to the CacheEntryId for the state.
   */

  CacheEntryId StateCache::ensureCacheEntry(const State& state)
  {
	StateCacheMap::const_iterator it = m_states.find(state);
	if (it != m_states.end()) {
	  debugMsg("StateCache:ensureCacheEntry", " Found existing state " << toString(state));
	  return it->second;
	}

	// Create new entry
	debugMsg("StateCache:ensureCacheEntry",
			 " Allocating cache entry for state " << toString(state));
	CacheEntryId entry = m_states[state] = (new CacheEntry(state))->getId();
	return entry;
  }

  void StateCache::handleQuiescenceStarted()
  {
	check_error(!m_inQuiescence);
	m_inQuiescence = true;
	if (m_timeEntry->lastQuiescence < m_quiescenceCount)
	  updateTimeState();
  }

  void StateCache::handleQuiescenceEnded()
  {
	check_error(m_inQuiescence);
	m_quiescenceCount++;
	checkError(m_quiescenceCount > 0, "Quiescence counter wrapped around!");
	m_inQuiescence = false;
  }

  double StateCache::currentTime()
  {
	if (!m_inQuiescence) 
	  updateTimeState();
	return m_timeEntry->value;
  }

  void StateCache::updateTimeState()
  {
	double time = m_interface->lookupNow(m_timeState);
	checkError(m_timeEntry->value <= time,
			   "Time has regressed from " << std::setprecision(15) << m_timeEntry->value << " to " << time);
	if (internalStateUpdate(m_timeEntry, time))
	  m_interface->setThresholds(m_timeState, m_timeEntry->highThreshold, m_timeEntry->lowThreshold);
  }

  const State& StateCache::getTimeState() const
  {
	return m_timeState;
  }

  std::string StateCache::toString(const State& state)
  {
	std::ostringstream str;
	str << LabelStr(state.first).toString() << "(";
	for (size_t i = 0; i < state.second.size(); i++) {
	  if (i != 0)
		str << ", ";
	  str << Expression::valueToString(state.second[i]);
	}
	str << ")";
	return str.str();
  }

}
