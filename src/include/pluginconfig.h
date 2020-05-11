/*
   Copyright (C) 2018-2020 Adrian Michel

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#include <nlohmann\json.hpp>
#include "enum.h"
#include "misc.h"
#include "positionsizingparams.h"
#include "datacollection.h"
#include "runtimeparams.h"
#include "symbolsiterator.h"

namespace tradery {
class PositionsContainer;
}  // namespace tradery

DEFINE_ENUM_WITH_STRING_CONVERSIONS(
    RuntimeStatus, (READY)(RUNNING)(CANCELING)(ENDED)(CANCELED))

class RuntimeStats {
 public:
  virtual ~RuntimeStats() {}

  // will increase the number of steps, and return the value that will have to
  virtual void addPct(double pct) = 0;
  virtual void step(double pct) = 0;
  //  virtual void getCrtPct() const = 0;

  virtual void setTotalSymbols(unsigned int totalSymbols) = 0;
  virtual void incSignals() = 0;
  virtual void setRawTrades(unsigned int trades) = 0;
  virtual void setProcessedTrades(unsigned int trades) = 0;
  virtual void setProcessedSignals(unsigned int signals) = 0;
  virtual void incErrors() = 0;
  virtual void incTotalRuns() = 0;
  virtual void incErrorRuns() = 0;
  virtual void incTotalBarCount(unsigned int barsCount) = 0;
  virtual unsigned int getTotalBarCount() const = 0;
  virtual void setMessage(const std::string& message) = 0;
  virtual void setStatus(RuntimeStatus status) = 0;
  // virtual void setCurrentSymbol(const std::string& symbol) = 0;
  virtual void to_json(nlohmann::json& j) const = 0;
  virtual std::string to_json() const = 0;
};

/**
 * Information about a session, such as output sink, session name, symbols
 * iterator, runtime params, data associated with the session
 *
 * It is sent along with a sessionStarted notifications to SessionEventHandler
 * objects
 *
 * @see SessionEventHandler
 */
class SessionInfo {
 public:
  virtual ~SessionInfo() {}
  /**
   * Returns the OutputSink associated with the session
   *
   * This can be used to send information to the output window associated with
   * the session window
   *
   * @return The OutputSink
   */
  virtual OutputSink& outputSink() const = 0;

  /**
   * Returns the session name
   *
   * @return The session name
   */
  virtual const std::string& sessionName() const = 0;
  /**
   * Returns an iterator to the symbols source associated with the session
   *
   * Can be used to traverse the set of symbols for the current session
   *
   * @return A pointer to a SymbolsIterator for the current session
   */
  virtual tradery::SymbolsIteratorPtr symbolsIterator() const = 0;

  /**
   * Returns data for a symbol from the session data source
   *
   * The data returned will be the same as for other elements of the session,
   * such as runnables, so it it will span the same range etc.
   *
   * In case data is not available for that symbol, 0 will be returned
   *
   * @param symbol The symbol for which the data is to be returned for
   *
   * @return A smart pointer to the data collection. The pointer will be 0 if
   * data is not available
   * @see Datastd::shared_ptr
   */
  virtual BarsPtr getData(const std::string& symbol) const = 0;
  /**
   * Get runtime parameters associated with the current session
   *
   * Runtime parameters include range, threads, position sizing etc
   *
   * @return pointer to a
   */
  virtual const RuntimeParams* runtimeParams() const = 0;
  virtual RuntimeStats* runtimeStats() = 0;
};

/**
 * Abstract base for classes that are to receive session events.
 *
 * Besides the run events defined in the base class RunEventHandler, session
 * events signal the beginning and end of a session, the request for session
 * cancel,
 *
 * A SessionEventHandler implementation must assume that these events may be
 * sent from a thread different from the one that was used to create the handler
 * object, therefore the user code must make sure that multi-threading issues
 * such as concurrent access to shared data, GUI multithreaded issues etc. are
 * dealt with appropriately.
 */
class SessionEventHandler : public RunEventHandler {
 public:
  virtual ~SessionEventHandler() {}
  /**
   * Called when the session is about to start.
   *
   * This method also receives a reference to a SessionInfo object
   * Each SessionEventHandler receives its own instance of a SessionInfo, whose
   * lifetime spans the session lifetime
   *
   * The SessionEventHandler will typically store the reference to the
   * SessionInfo object and it will use it throughout its lifetime to interact
   * with the session.
   *
   * For example, a statistics plug-in will calculate stats based on the symbols
   * and their data used during the session, such as equity curve, un-realized
   * profit of open trades etc
   *
   * @param si     Reference to a SessionInfo object
   */
  virtual void sessionStarted(SessionInfo& si) = 0;
  /**
   * Called after the session has ended.
   *
   * Also passed a reference to a PositionsContainer object, which contains all
   * the trades generated during the session, open or closed. One use for this
   * parameter is to calculate statistics for the session for example.
   *
   * @param positions A PositionsContainer containing all the trades generated
   * during the session
   */
  virtual void sessionEnded(PositionsContainer& positions) = 0;
  /**
   * Caled when a cancel request was initiated for the session
   */
  virtual void sessionCanceled() = 0;
};

/**
 * A SessionEventHandler whose rolse is to delegate session notifications to one
 * or more other SessionEventHandler instances.
 *
 * Besides all the SessionEventHandler methods, it allows adding and removing
 * SessionEventHandlers that events are to be delegated to
 *
 * @see SessionEventHandler
 */
class SessionEventHandlerDelegator : public SessionEventHandler {
 private:
  std::set<SessionEventHandler*> _handlers;

 public:
  virtual ~SessionEventHandlerDelegator() { assert(_handlers.size() == 0); }

  /**
   * Adds a new SessionEventHandler
   *
   * @param handler The SessionEventHandler to be added
   */
  void add(SessionEventHandler* handler) { _handlers.insert(handler); }

  /**
   * Remove a SessionEventHandler
   *
   * @param handler The SessionEventHandler to be removed
   */
  void remove(SessionEventHandler* handler) { _handlers.erase(handler); }

  /**
   * Delegates the event to all SessionEventHandler instances that have been
   * added using the method add
   *
   * Note: the SessionInfo reference will be common to all the
   * SessionEventHandler instances that the notification is sent to by the
   * delegator, so care will have to be taken when using mutable objects such as
   * the symbols iterator
   *
   * TODO: create local copies of the SessionInfo to be passed to the
   * delegatees. How will these copies be managed?
   *
   * @param so     The SessionInfo reference
   */
  virtual void sessionStarted(SessionInfo& so) {
    for (auto handler : _handlers) {
      handler->sessionStarted(so);
    }
  }

  /**
   * Delegates the event to all SessionEventHandler instances that have been
   * added using the method add
   *
   * Note: the PositionsContainer reference will be common to all the
   * SessionEventHandler instances that the notification is sent to by the
   * delegator, so care will have to be taken when using non-const methods, as
   * they will affect all the other SessionEventHandler instances receiving this
   * message
   *
   * @param positions The reference to a PositionsContainer object containing
   * all the trades generated during the session
   */
  virtual void sessionEnded(PositionsContainer& positions) {
    for (auto handler : _handlers) {
      handler->sessionEnded(positions);
    }
  }

  /**
   * Delegates the event to all SessionEventHandler instances that have been
   * added using the method add
   */
  virtual void sessionCanceled() {
    for (auto handler : _handlers) {
      handler->sessionCanceled();
    }
  }

  /**
   * Delegates the event to all SessionEventHandler instances that have been
   * added using the method add
   */
  virtual void runStarted() {
    for (auto handler : _handlers) {
      handler->runStarted();
    }
  }

  /**
   * Delegates the event to all SessionEventHandler instances that have been
   * added using the method add
   */
  virtual void runCanceled() {
    for (auto handler : _handlers) {
      handler->runCanceled();
    }
  }

  /**
   * Delegates the event to all SessionEventHandler instances that have been
   * added using the method add
   */
  virtual void runEnded() {
    for (auto handler : _handlers) {
      handler->runEnded();
    }
  }
};

/**
 * Base class for all classes that may be used as plugin configurations, such as
 * DataSource, Runnable, SymbolsSource etc.
 *
 * It is itself derived from Info and SessionEventHandler. Each plugin
 * configuration needs to be associated with basic Info: UniqueId, name and
 * description.
 *
 * Also, each plug-in configuration must be able to receive session
 * notifications. In fact, all session events methods have default
 * implementation that simply ignore the event (except for sessionStarted), thus
 * making it easier to implement actual configuration classes. If any of the
 * events is of interest, the corresponding method must be implemented.
 *
 * The sessionStarted event is partly processed, in that the session info data
 * (name and OutputSink) are saved locally for use by the derived classes. Every
 * plugin configuration, including datasources, symbolssources etc can send
 * formatted strings to the session output window
 *
 * A PluginConfiguration provides an easy to use mechanism for GUI enabled
 * plug-in configurations. In order to show a window in the TradingApp GUI tool,
 * a plug-in configuration must indicate that it has a window, and be able to
 * respond to a request for the window info with elements such as the window
 * handle, the tab name (which will be displayed in the tab portion of the
 * session window in the TradingApp GUI application).
 *
 * Note: in order for a window to be displayed correctly there are two
 * constraints on the window:
 * - it must have the WM_CHILD style
 * - it should not have borders
 *
 * @see SessionEventHandler
 * @see WindowInfo
 * @see SessionInfo
 * @see PositionsContainer
 */
class PluginConfiguration : public Info, public SessionEventHandler {
 private:
  SessionInfo* _sessionInfo;

 public:
  /**
   * Constructor taking a reference to an Info as parameter
   *
   * @param info   The reference to a Info
   */
  PluginConfiguration(const Info& info) : Info(info) {}
  /**
   * Default constructor.
   *
   * Sets the id to a new UniqueId, and the name and description to empty
   * strings
   */
  PluginConfiguration() {}
  virtual ~PluginConfiguration() {}

  /**
   * Returns the SessionInfo associated with the session
   *
   * A reference to the SessionInfo object received by the virtual method
   * sessionStarted is stored and can be accessed and used throughout the
   * session within the PluginConfiguration, using this method
   *
   * @return The SessionInfo for the session
   */
  const SessionInfo& sessionInfo() const {
    assert(_sessionInfo != 0);
    return *_sessionInfo;
  }

  SessionInfo& sessionInfo() {
    assert(_sessionInfo != 0);
    return *_sessionInfo;
  }

  /**
   * Returns the OutputSink associated with the session.
   *
   * This is a convenience method to access the outputSink stored in the
   * SessionInfo object for the session, received by this PluginConfiguration
   * object
   *
   * @return The OuptutSink for the session
   */
  OutputSink& outputSink() const {
    assert(_sessionInfo != 0);
    return _sessionInfo->outputSink();
  }

  /**
   * Returns the session name
   *
   * This is a convenience method to access the session name stored in the
   * SessionInfo object for the session, received by this PluginConfiguration
   * object
   *
   * @return The session name for the session
   */
  const std::string& sessionName() const {
    assert(_sessionInfo != 0);
    return _sessionInfo->sessionName();
  }

  /**
   * Returns a data collection for a symbol.
   *
   * This is a convenience method that calls the method with the same name in
   * the SessionInfo object.
   *
   * The data collection is retrieved using the session datasource and time
   * range
   *
   * @param symbol The symbol for which the data is to be retrieved
   *
   * @return A pointer to a data collection
   * @see Datastd::shared_ptr
   */
  BarsPtr getData(const std::string& symbol) const {
    assert(_sessionInfo != 0);
    return _sessionInfo->getData(symbol);
  }

  /**
   * Returns the symbols iterator for the current session
   *
   * This is a convenience method that calls the method with the same name in
   * the SessionInfo object.
   *
   * The returned symbols iterator corresponds to the symbols source for the
   * current session
   *
   * Each PluginConfiguration receives its own copy of a symbols iterator, so it
   * can iterate over the symbols for the current session without any
   * interference from the other PluginConfiguration
   *
   * @return A pointer to a symbols iterator
   * @see SymbolsIterator
   */
  tradery::SymbolsIteratorPtr symbolsIterator() const {
    assert(_sessionInfo != 0);
    return _sessionInfo->symbolsIterator();
  }

  /* @cond */
  virtual void sessionStarted(SessionInfo& si) {
    _sessionInfo = &si;
    sessionStarted();
  }
  virtual void sessionEnded(PositionsContainer& positions) {}
  virtual void sessionCanceled() {}
  virtual void sessionStartedWaitingForNextRun() {}
  virtual void runStarted() {}
  virtual void runEnded() {}
  virtual void runCanceled() {}

 protected:
  /**
   * A new session event in addition to the ones provided by the
   * SessionEventHandler base class.
   *
   * This event is provided for convenience and it has the same meaning as the
   * sessionStarted of the SessionEventHandler, but without the SessionInfo
   * parameter, which can be retrieved using the corresponding accessor method
   * sessionInfo.
   *
   *
   * Can be overriden in the derived classes to process this event.
   *
   * @see sessionStarted( SessionInfo& si )
   */
  virtual void sessionStarted() {}

  /* @endcond */
};
