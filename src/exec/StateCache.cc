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
#include "ExternalInterface.hh"
#include "Expression.hh"
#include "Expressions.hh"
#include "Debug.hh"
//#include "Interfaces.hh"
#include <vector>
#include <limits>
#include <cmath>

namespace PLEXIL
{

   namespace Cache
   {
      Lookup::Lookup(const ExpressionId& _source, const Expressions& _dest, 
                     const StateKey& key)
         : source(_source), dest(_dest), state(key), m_id(this) {}

      Lookup::~Lookup()
      {
         m_id.remove();
      }

      ChangeLookup::ChangeLookup(const ExpressionId& _source, 
                                 const Expressions& _dest,
                                 const StateKey& key, 
                                 const std::vector<double>& _tolerances)
         : Lookup(_source, _dest, key), tolerances(_tolerances)
      {
         previousValues.insert(previousValues.end(), _dest.size(), 
                               Expression::UNKNOWN());
      }


      FrequencyLookup::FrequencyLookup(const ExpressionId& _source, 
                                       const Expressions& _dest,
                                       const StateKey& key, const double _lowFreq,
                                       const double _highFreq)
         : Lookup(_source, _dest, key), lowFreq(_lowFreq), 
           highFreq(_highFreq), lastTime(-1) 
      {
      }
   }

   StateCache::StateCache() : m_id(this), m_inQuiescence(false), 
                              m_quiescenceCount(0), m_timeState(-1)
   {
   }

   StateCache::~StateCache()
   {
      for (std::map<ExpressionId, Cache::LookupId>::iterator it = 
              m_lookupsByExpression.begin(); 
           it != m_lookupsByExpression.end(); ++it)
      {
         delete (Cache::Lookup*) it->second;
      }
      m_id.remove();
   }
   
   void StateCache::lookupNow(const ExpressionId& source, Expressions& dest,
                              const State& state)
   {
      check_error(m_inQuiescence, "Lookup outside of quiescence!");
      debugMsg("StateCache:lookupNow", "Looking up value for state " <<
               toString(state) << " because of " << source->toString());

      std::vector<double> tolerances(dest.size(), 0);
      StateKey key;
      bool newState = keyForState(state, key);

      if (newState || m_states.find(key)->second.second < m_quiescenceCount)
      {
         condDebugMsg(newState, "StateCache:lookupNow",
                      "New state, so performing external lookup.");
         condDebugMsg(!newState, "StateCache:lookupNow",
                      "Out-of-date state, so performing external lookup.");

         Cache::LookupId lookup = (new Cache::ChangeLookup(source, dest, key,
                                                           tolerances))->getId();
         std::multimap<StateKey, Cache::LookupId>::iterator it =
            m_lookups.insert(std::make_pair(key, lookup));
         std::vector<double> values(lookup->dest.size(), Expression::UNKNOWN());

         // if this is a new state or we haven't looked the state up this
         // quiescence, perform the request and update the state

         if (newState)
            getExternalInterface()->lookupNow(state, key, values);
         else
            getExternalInterface()->lookupNow(key, values);

         internalStateUpdate(key, values);

         delete (Cache::Lookup*) lookup;
         m_lookups.erase(it);
      }
      //otherwise, return the values we've got
      else
      {
         std::map<StateKey, std::vector<double> >::iterator currentValues = 
            m_values.find(key);
         check_error(currentValues != m_values.end());
         debugMsg("StateCache:lookupNow", 
                  "Already have up-to-date values for state, so using those ("
                  << toString(currentValues->second) << ")");

         // if this isn't a new state, then make sure the destination is
         // the right size

         checkError(currentValues->second.size() == dest.size(),
                    "Destination size mismatch for state " << 
                    LabelStr(state.first).toString() <<
                    ".  Expected " << currentValues->second.size() << 
                    ", got " << dest.size());
         std::vector<double>::iterator valueIt = currentValues->second.begin();
         for (Expressions::iterator destIt = dest.begin();
              valueIt != currentValues->second.end() && destIt != dest.end();
              ++valueIt, ++destIt)
         {
            if (destIt->isNoId())
               continue;
            (*destIt)->setValue(*valueIt);
         }
      }
   }

   void StateCache::registerChangeLookup(const ExpressionId& source,
                                         Expressions& dest,
                                         const State& state,
                                         const std::vector<double>& tolerances)
   {
      check_error(m_inQuiescence, "Lookup outside of quiescence!");
      debugMsg("StateCache:registerChangeLookup", "Registering change lookup " <<
               source->toString() << " for state " << toString(state) <<
               " with tolerances (" << toString(tolerances) << ")");
      StateKey key;
      bool newState = keyForState(state, key);

      Cache::LookupId lookup = (new Cache::ChangeLookup(source, dest, key,
                                                        tolerances))->getId();

      std::multimap<StateKey, Cache::LookupId>::iterator it =
         m_lookups.insert(std::make_pair(key, lookup));
      m_lookupsByExpression.insert(std::make_pair(source, lookup));

      if (newState || m_states.find(key)->second.second < m_quiescenceCount)
      {
         condDebugMsg(newState, "StateCache:registerChangeLookup",
                      "New state, so performing external lookup.");
         condDebugMsg(m_states.find(key)->second.second < m_quiescenceCount,
                      "StateCache:registerChangeLookup",
                      "Out-of-date state, so performing external lookup.");

         std::vector<double> values(lookup->dest.size(), Expression::UNKNOWN());
         getExternalInterface()->registerChangeLookup(
            (LookupKey) source, state, key, tolerances, values);
         internalStateUpdate(key, values);
      }
      else
      {
         getExternalInterface()->registerChangeLookup((LookupKey) source,
                                                             key, tolerances);
         std::map<StateKey, std::vector<double> >::iterator currentValues =
            m_values.find(key);
         check_error(currentValues != m_values.end());
         debugMsg("StateCache:registerChangeLookup", "Already have up-to-date values for state, so using those (" <<
                  toString(currentValues->second) << ")");
         std::vector<double>::iterator valueIt = currentValues->second.begin();
         for (Expressions::iterator destIt = dest.begin(); valueIt != currentValues->second.end() && destIt != dest.end(); ++valueIt, ++destIt)
         {
            if (destIt->isNoId())
               continue;
            (*destIt)->setValue(*valueIt);
         }
         ((Cache::ChangeLookup*)lookup)->previousValues = currentValues->second;
      }
   }

   void StateCache::registerFrequencyLookup(const ExpressionId& source, Expressions& dest, const State& state, const double lowFreq, const double highFreq)
   {
      check_error(m_inQuiescence, "Lookup outside of quiescence!");
      debugMsg("StateCache:registerFrequencyLookup", "Registering frequency lookup " << source->toString() << " for state " << toString(state));
      StateKey key;
      bool newState = keyForState(state, key);

      Cache::LookupId lookup = (new Cache::FrequencyLookup(source, dest, key, lowFreq, highFreq))->getId();

      std::multimap<StateKey, Cache::LookupId>::iterator it = m_lookups.insert(std::make_pair(key, lookup));
      m_lookupsByExpression.insert(std::make_pair(source, lookup));

      if (newState || m_states.find(key)->second.second < m_quiescenceCount)
      {
         condDebugMsg(newState, "StateCache:registerFrequencyLookup", "New state, so performing external lookup.");
         condDebugMsg(m_states.find(key)->second.second < m_quiescenceCount, "StateCache:registerFrequencyLookup",
                      "Out-of-date state, so performing external lookup.");
         std::vector<double> values(lookup->dest.size(), Expression::UNKNOWN());

         getExternalInterface()->registerFrequencyLookup((LookupKey) source, state, key, lowFreq, highFreq, values);
         internalStateUpdate(key, values);
      }
      else
      {
         std::map<StateKey, std::vector<double> >::iterator currentValues = m_values.find(key);
         check_error(currentValues != m_values.end());
         debugMsg("StateCache:registerFrequency", "Already have up-to-date values for state, so using those (" <<
                  toString(currentValues->second) << ")");
         std::vector<double>::iterator valueIt = currentValues->second.begin();
         for (Expressions::iterator destIt = dest.begin(); valueIt != currentValues->second.end() && destIt != dest.end(); ++valueIt, ++destIt)
         {
            if (destIt->isNoId())
               continue;
            (*destIt)->setValue(*valueIt);
         }
         ((Cache::FrequencyLookup*)lookup)->lastTime = currentTime();
         getExternalInterface()->registerFrequencyLookup((LookupKey) source, key, lowFreq, highFreq);
      }
   }

   void StateCache::internalUnregisterLookup(const ExpressionId& source)
   {
      check_error(source.isValid());
      checkError(m_lookupsByExpression.find(source) != m_lookupsByExpression.end(), "No stored lookup for " << source->toString());
      Cache::LookupId lookup = m_lookupsByExpression.find(source)->second;
      std::multimap<StateKey, Cache::LookupId>::iterator it = m_lookups.find(lookup->state);
      checkError(it != m_lookups.end(), "No state -> lookup entry for " << source->toString());
      while (it != m_lookups.end() && it->first == lookup->state)
      {
         if (it->second == lookup)
            break;
         ++it;
      }
      m_lookups.erase(it);
      m_lookupsByExpression.erase(source);
      delete (Cache::Lookup*) lookup;
   }

   void StateCache::unregisterChangeLookup(const ExpressionId& source)
   {
      check_error(m_inQuiescence, "Lookup outside of quiescence!");
      internalUnregisterLookup(source);
      getExternalInterface()->unregisterChangeLookup((LookupKey) source);
   }

   void StateCache::unregisterFrequencyLookup(const ExpressionId& source)
   {
      check_error(m_inQuiescence, "Lookup outside of quiescence!");
      internalUnregisterLookup(source);
      getExternalInterface()->unregisterFrequencyLookup((LookupKey)source);
   }

   void StateCache::updateState(const State& state, const std::vector<double>& values)
   {
      check_error(!m_inQuiescence);
      if (m_keysByState.find(state) == m_keysByState.end())
      {
         debugMsg("StateCache:updateState",
                  "Got the following update, but nobody cares: " << toString(state) << " = " <<
                  toString(values));
         return;
      }
      StateKey key;
      bool newState = keyForState(state, key);
      //should newState be an error?
      if (newState)
      {
         debugMsg("StateCache:updateState", "No values for state " << toString(state) << " so storing a bunch of UNKNOWNs.");
         m_values.insert(std::make_pair(key, std::vector<double>(values.size(), Expression::UNKNOWN())));
      }
      updateState(key, values);
   }

   void StateCache::updateState(const StateKey& key, const std::vector<double>& values)
   {
      check_error(!m_inQuiescence);

      std::map<StateKey, std::pair<State, int> >::iterator it = m_states.find(key);
      checkError(it != m_states.end(), "No known state for key " << key);

      internalStateUpdate(key, values);
      //i'm not sure who should be responsible for stepping the exec
      //bool changedValues = internalStateUpdate(key, values);
      //     if(changedValues)
      //       Interfaces::execFunctional()->stepExec();

   }


   bool StateCache::internalStateUpdate(const StateKey& key,
                                        const std::vector<double>& values)
   {
      check_error(m_states.find(key) != m_states.end());
      debugMsg("StateCache:updateState",
               "Updating state " << toString(m_states[key].first) <<
               " with values " << toString(values));

      std::map<StateKey, std::pair<State, int> >::iterator stateIt =
         m_states.find(key);
      checkError(stateIt->second.second <= m_quiescenceCount,
                 "Synchronization error.  State " <<
                 toString(stateIt->second.first) <<
                 " was upated in a future quiescence.  Current: " <<
                 m_quiescenceCount << " update: " << stateIt->second.second);
      stateIt->second.second = m_quiescenceCount;

      std::map<StateKey, std::vector<double> >::iterator valueIt =
         m_values.find(key);
      if (valueIt == m_values.end())
         m_values.insert(std::make_pair(key, values));
      else
      {
         checkError(
            valueIt->second.size() == values.size(),
            "Received a different number of values for state "
            << toString(stateIt->second.first)
            << " than previously.  Expected "
            << valueIt->second.size()
            << ", but got "
            << values.size());
         valueIt->second = values;
      }


      std::multimap<StateKey, Cache::LookupId>::iterator it = m_lookups.find(key);

      //should it be an error if we receive a state update without any lookups?
      bool retval = false;
      double time = (key == m_timeState ? values[0] : currentTime());

      while (it != m_lookups.end() && it->first == key)
      {
         Cache::LookupId l = it->second;
         check_error(l.isValid());

         if (Id<Cache::ChangeLookup>::convertable(l))
         {
            Cache::ChangeLookup* lookup = (Cache::ChangeLookup*) l;
            retval = updateChangeLookup(lookup, values) || retval;
         }
         else if (Id<Cache::FrequencyLookup>::convertable(l))
         {
            Cache::FrequencyLookup* lookup = (Cache::FrequencyLookup*) l;
            retval = updateFrequencyLookup(lookup, values, time) || retval;
         }
         else
         {
            checkError(ALWAYS_FAIL, "Bizarre... lookup stored not of type ChangeLookup or FrequencyLookup.");
         }
         ++it;
      }

      if (key == m_timeState)
      {
         debugMsg("StateCache:updateState", "State is time state.  Checking to see if there are timed-out frequency lookups.");
         for (std::map<ExpressionId, Cache::LookupId>::iterator it = m_lookupsByExpression.begin(); it != m_lookupsByExpression.end(); ++it)
         {
            Cache::LookupId lookup = it->second;
            check_error(lookup.isValid());
            if (!Id<Cache::FrequencyLookup>::convertable(lookup))
               continue;
            Cache::FrequencyLookup* freq = (Cache::FrequencyLookup*) lookup;
            checkError(freq->lastTime <= time, "Synchronization error.  Lookup " << freq->source->toString() << " was updated in the future: " << freq->lastTime << ", " << time);
            if (time - freq->lastTime > freq->lowFreq)
            {
               debugMsg("StateCache:updateState", "Lookup " << freq->source->toString() << " has timed out.  Current time: " <<
                        time << " last update: " << freq->lastTime << " low frequency: " << freq->lowFreq);
               for (Expressions::size_type i = 0; i < freq->dest.size(); ++i)
                  if (freq->dest[i].isValid())
                     freq->dest[i]->setValue(Expression::UNKNOWN());
            }
            else
            {
               debugMsg("StateCache:updateState",
                        "Lookup " << freq->source->toString() << " has not timed out.  Current time: " <<
                        time << " last update: " << freq->lastTime << " low frequency: " << freq->lowFreq);
            }
         }
      }

      return retval;
   }

   bool StateCache::updateChangeLookup(Cache::ChangeLookup* lookup, 
                                       const std::vector<double>& values)
   {
      bool needsUpdate = false;
      check_error(lookup->tolerances.size() == values.size());
      check_error(lookup->previousValues.size() == values.size());
      check_error(lookup->dest.size() == values.size());

      debugMsg("StateCache:updateState", 
               "Seeing if change lookup " << lookup->source->toString() <<
               " needs updating from " << toString(lookup->previousValues) <<
               " to " << toString(values));
      for (std::vector<double>::size_type i = (std::vector<double>::size_type) 0;
           i < values.size(); ++i)
      {
         if (differenceMagnitude(lookup->previousValues[i], values[i]) >= 
             lookup->tolerances[i])
         {
            condDebugMsg(lookup->previousValues[i] == Expression::UNKNOWN() &&
                         values[i] != Expression::UNKNOWN(),
                         "StateCache:updateState", 
                         "Updating because the previous value is"
                         " UNKNOWN and the new value is not.");
            condDebugMsg(!(lookup->previousValues[i] == Expression::UNKNOWN() && 
                           values[i] != Expression::UNKNOWN()) &&
                         fabs(lookup->previousValues[i] - values[i]) >= 
                         lookup->tolerances[i], "StateCache:updateState",
                         "Updating because the difference between value " <<
                         i << " (old: " << lookup->previousValues[i] <<
                         ", new: " << values[i] <<
                         ") is greater than or equal to the tolerance (" << 
                         lookup->tolerances[i] << ")");
            needsUpdate = true;
            break;
         }
         debugMsg("StateCache:updateState", 
                  "Not updating.  All changes are within the tolerance.");
      }
      if (needsUpdate)
      {
         debugMsg("StateCache:updateState", "Updating change lookup " <<
                  lookup->source->toString() << " from " <<
                  toString(lookup->previousValues) << " to " << toString(values));
         for (std::vector<double>::size_type i = (std::vector<double>::size_type)0;
              i < values.size(); ++i)
         {
            if (lookup->dest[i].isValid())
            {
               lookup->dest[i]->setValue(values[i]);
            }
            lookup->previousValues[i] = values[i];
         }
      }
      return needsUpdate;
   }

   bool StateCache::updateFrequencyLookup(Cache::FrequencyLookup* lookup, const std::vector<double>& values, const double& time)
   {
      bool needsUpdate = false;
      check_error(lookup->dest.size() == values.size());

      check_error(lookup->lastTime <= time);

      debugMsg("StateCache:updateState", "Seeing if frequency lookup " << lookup->source->toString() <<
               " needs updating from time " << lookup->lastTime << " (current: " << time << ")");
      //need an update if we've never been updated or delta t >= the high frequency
      needsUpdate = (lookup->lastTime == -1) || (time - lookup->lastTime >= lookup->highFreq);
      //too late if we've been previously updated and delta t > the low frequency
      bool tooLate = (lookup->lastTime != -1) && (time - lookup->lastTime > lookup->lowFreq);
      condDebugMsg(tooLate, "StateCache:updateState", "Update is too late.  Setting unknown.");
      condDebugMsg(lookup->lastTime != -1 && (time - lookup->lastTime < lookup->highFreq), "StateCache:updateState",
                   "Not updating.  The time of the last update (" << lookup->lastTime << ") and current time (" << time <<
                   ") differ by less than the high frequency (" << lookup->highFreq << ")");

      lookup->lastTime = time;
      if (needsUpdate)
      {
         debugMsg("StateCache:updateState",
                  "Updating frequency lookup " << lookup->source->toString() << " to " << (tooLate ? std::string("UNKNOWN") : toString(values)));
         for (std::vector<double>::size_type i = (std::vector<double>::size_type) 0; i < values.size(); ++i)
            if (lookup->dest[i].isValid())
               lookup->dest[i]->setValue((tooLate ? Expression::UNKNOWN() : values[i]));
      }
      return needsUpdate;
   }

  /**
   * @brief Find the unique key for a state.
   * @param state The state.
   * @param key The key associated with this state.
   * @return True if the key was found.
   */
  bool StateCache::findStateKey(const State& state, StateKey& key)
  {
    std::map<State, StateKey>::const_iterator it = m_keysByState.find(state);
    if (it != m_keysByState.end())
      {
	debugMsg("StateCache:findStateKey", " found state \"" << toString(state) << "\", key " << it->second);
	key = it->second;
	return true;
      }

    debugMsg("StateCache:findStateKey", " state \"" << toString(state) << "\" not found");
    return false;
  }

   bool StateCache::keyForState(const State& state, StateKey& key)
   {
     std::map<State, StateKey>::const_iterator it = m_keysByState.find(state);
     if (it != m_keysByState.end())
       {
         debugMsg("StateCache:keyForState", " Already have a key for state \"" << toString(state) << "\": " << it->second);
         key = it->second;
         return false;
       }

     static double* sl_stateKey = NULL;
     if (sl_stateKey == NULL)
       sl_stateKey = new double(std::numeric_limits<double>::min());

     debugMsg("StateCache:keyForState", " Allocating key for state \"" << toString(state) << "\": " << sl_stateKey);
     key = *sl_stateKey;
     (*sl_stateKey) += 2 * EPSILON;
     m_keysByState.insert(std::make_pair(state, key));
     m_states.insert(std::make_pair(key, std::make_pair(state, -1)));
     return true;
   }

   bool StateCache::stateForKey(const StateKey& key, State& state) const
   {
      std::map<StateKey, std::pair<State, int> >::const_iterator it = m_states.find(key);
      if (it == m_states.end())
         return false;
      state = (*it).second.first;
      return true;
   }

  const char* StateCache::stateNameForKey(const StateKey& key) const
  {
    State state;
    if (stateForKey(key, state))
      return LabelStr(state.first).c_str();
    return NULL;
  };

   void StateCache::handleQuiescenceStarted()
   {
      check_error(!m_inQuiescence);
      m_inQuiescence = true;

      bool firstTimeQuery = false;
      if (firstTimeQuery = (m_timeState == -1))
      {
         State timeState;
         timeState.first = LabelStr("time");
         keyForState(timeState, m_timeState);
      }

      std::map<StateKey, std::pair<State, int> >::iterator timeIt = m_states.find(m_timeState);
      check_error(timeIt != m_states.end());

      checkError(timeIt->second.second <= m_quiescenceCount,
                 "Synchronization error.  'time' state has a quiescence count greater than the cache's (" << timeIt->second.second << " > " << m_quiescenceCount << ")");

      if (timeIt->second.second == m_quiescenceCount)
         return;

      std::vector<double> time(1, 0);
      if (firstTimeQuery)
         getExternalInterface()->lookupNow(timeIt->second.first, m_timeState, time);
      else
         getExternalInterface()->lookupNow(m_timeState, time);

      timeIt->second.second = m_quiescenceCount;

      std::map<StateKey, std::vector<double> >::iterator valueIt = m_values.find(m_timeState);
      if (valueIt == m_values.end())
         m_values.insert(std::make_pair(m_timeState, time));
      else
         valueIt->second = time;


      internalStateUpdate(m_timeState, time);
   }

   void StateCache::handleQuiescenceEnded()
   {
      check_error(m_inQuiescence);
      m_quiescenceCount++;
      m_inQuiescence = false;
   }

   double StateCache::currentTime()
   {
      check_error(m_timeState != -1);
      if (!m_inQuiescence)
      {
         std::vector<double> time(1, 0);
         getExternalInterface()->lookupNow(m_timeState, time);
         checkError(m_values.find(m_timeState)->second[0] <= time[0], "Time has regressed from " << m_values.find(m_timeState)->second[0] << " to " << time[0]);
         internalStateUpdate(m_timeState, time);
      }
      std::map<StateKey, std::vector<double> >::iterator it = m_values.find(m_timeState);
      check_error(it != m_values.end());
      return it->second[0];
   }

   const State& StateCache::getTimeState() const
   {
      std::map<StateKey, std::pair<State, int> >::const_iterator it = m_states.find(m_timeState);
      checkError(it != m_states.end(), "No state found for time!");
      return (*it).second.first;
   }

   const StateKey& StateCache::getTimeStateKey() const
   {
      std::map<StateKey, std::pair<State, int> >::const_iterator it = m_states.find(m_timeState);
      checkError(it != m_states.end(), "No state found for time!");
      return (*it).first;
   }

   double StateCache::differenceMagnitude(const double x, const double y) const
   {
      if (x == Expression::UNKNOWN())
      {
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

   std::string StateCache::toString(const State& state)
   {
      std::stringstream str;
      str << LabelStr(state.first).toString() << "(" << toString(state.second) << ")";
      return str.str();
   }

   std::string StateCache::toString(const std::vector<double>& values)
   {
      std::stringstream str;
      if (!values.empty())
      {
         for (std::vector<double>::size_type i = 0; i < values.size(); ++i)
         {
           if (i > 0)
             str << ", ";
           if (LabelStr::isString(values[i]))
             str << LabelStr(values[i]).toString();
           else if (StoredArray::isKey(values[i]))
             str << StoredArray(values[i]).toString();
           else
             str << values[i];
         }
      }
      return str.str();
   }
}
