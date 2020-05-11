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

#include <moremiscwin.h>

namespace tradery {
namespace chart {
class ChartManager;
}
/**\defgroup Error Error handling
 * Definitions of error handling related classes
 *
 * Because trading systems run in a multithreaded environment, and because C++
 * exceptions cannot be thrown across different threads an signalnate mechanism
 * for error handling must be used for signaling error conditions in trading
 * systems.
 *
 * Trading systems signal error conditions by sending error events to an error
 * sink.
 *
 *
 * @{
 */

/**
 * \brief Error event
 *
 * Error events are used by trading systems to signal error conditions within
 * the system.
 *
 * An error event has information about the type of error, the name of the
 * system that triggered it, a message describing the error, and the symbol on
 * which the error occured
 *
 * @see ErrorEventSink
 */
class CORE_API ErrorEvent {
 public:
  /**
   * Constants that define the different error types
   */
  enum Types {
    DATA_SOURCE_ERROR = 1,
    INVALID_DATA,
    DATA_ERROR,
    SYMBOLS_ERROR,
    SERIES_MANAGER_ERROR,
    BARS_ERROR,
    SERIES_ERROR,
    SYSTEM_ERROR,
    GENERAL_SYSTEM_ERROR,
    BAR_INDEX_OUT_OF_RANGE_ERROR,
    SERIES_INDEX_OUT_OF_RANGE_ERROR,
    SYNCHRONIZED_SERIES_INDEX_OUT_OF_RANGE_ERROR,
    TIME_SERIES_INDEX_OUT_OF_RANGE_ERROR,
    COVERING_LONG_POSITION_ERROR,
    SELLING_SHORT_POSITION_ERROR,
    CLOSING_ALREADY_CLOSED_POSITION_ERROR,
    INTEGER_DIVIDE_BY_0_ERROR,
    FLOATING_POINT_DIVIDE_BY_0_ERROR,
    ACCESS_VIOLATION_ERROR,
    OPERATION_ON_UNEQUAL_SIZE_SERIES_ERROR,
    DATA_INFO_ERROR,
    SIGNAL_HANDLER_ERROR,
    INVALID_INDEX_FOR_OPERATION_EXCEPTION,
    UNKNOWN_APPLICATION_ERROR,
    SERIES_SYNCHRONIZER_ERROR,
    CHART_ERROR,
    POSITION_ID_NOT_FOUND_ERROR,
    OPERATION_ON_SERIES_SYNCED_TO_DIFFERENT_SYNCHRONIZERS_ERROR,
    POSITION_CLOSE_OPERATION_ON_OPEN_POSITION_ERROR,
    POSITION_ZERO_PRICE_ERROR,
    OPERATION_NOT_ALLOWED_ON_SYNCHRONIZED_SERIES_ERROR,
    EXIT_STATMENT_CALL,
    INVALID_BARS_COLLECTION_ERROR,
    INVALID_POSITION_ERROR,
    ARRAY_INDEX_NOT_FOUND_ERROR,
    DICTIONARY_KEY_NOT_FOUND_ERROR,
    CLOSING_POSITION_ON_DIFFERENT_SYMBOL_ERROR,
    INVALID_LIMIT_PRICE_ERROR,
    INVALID_STOP_PRICE_ERROR,
    ERROR_COUNT  // this is the number of types of errors
  };

  enum Category { warning, error };

 private:
  const Types _type;
  const Category _category;
  const std::string _systemName;
  const std::string _message;
  const std::string _symbol;

 public:
  /**
   * Constructor - takes various error paramters as arguments:
   * - an error event type,
   * - the name of the system that triggered the error,
   * - the error message
   * - the symbol on which the error was triggered
   *
   * @param type       type of event
   * @param systemName system name
   * @param message    message string
   * @param symbol     The symbol on which the error event was triggered
   * @see ErrorEventSink
   */
  ErrorEvent(Types type, Category category, const std::string& systemName, const std::string& message, const std::string& symbol);
  ErrorEvent(const ErrorEvent& event)
      : _type(event.type()), _systemName(event.systemName()), _message(event.message()),
        _symbol(event.symbol()), _category(event.category()) {}

  /**
   * Returns the error type as defined in the enum Types
   *
   * @return Error type
   */
  Types type() const { return _type; }

  Category category() const { return _category; }

  std::string typeToString() const {
    switch (_type) {
      case DATA_SOURCE_ERROR:
        return "Data source";
      case INVALID_DATA:
        return "Invalid data";
      case DATA_ERROR:
        return "Data error";
      case SYMBOLS_ERROR:
        return "Symbols";
      case SERIES_MANAGER_ERROR:
        return "Series manager error";
      case BARS_ERROR:
        return "Bars error";
      case SERIES_ERROR:
        return "Series error";
      case SYSTEM_ERROR:
        return "System error";
      case GENERAL_SYSTEM_ERROR:
        return "General system error";
      case BAR_INDEX_OUT_OF_RANGE_ERROR:
        return "Bar index out of range";
      case INVALID_STOP_PRICE_ERROR:
        return "Invalid stop price error";
      case INVALID_LIMIT_PRICE_ERROR:
        return "Invalid limit price error";
      case SERIES_INDEX_OUT_OF_RANGE_ERROR:
        return "Series index out of range";
      case SYNCHRONIZED_SERIES_INDEX_OUT_OF_RANGE_ERROR:
        return "Synced series index out of range";
      case TIME_SERIES_INDEX_OUT_OF_RANGE_ERROR:
        return "Time series index out of range";
      case COVERING_LONG_POSITION_ERROR:
        return "Covering long position";
      case SELLING_SHORT_POSITION_ERROR:
        return "Selling short position";
      case CLOSING_ALREADY_CLOSED_POSITION_ERROR:
        return "Closing already closed position";
      case INTEGER_DIVIDE_BY_0_ERROR:
        return "Division by 0";
      case FLOATING_POINT_DIVIDE_BY_0_ERROR:
        return "Division by 0";
      case ACCESS_VIOLATION_ERROR:
        return "Acces violation error";
      case OPERATION_ON_UNEQUAL_SIZE_SERIES_ERROR:
        return "Operation on unequal size series";
      case DATA_INFO_ERROR:
        return "Data info error";
      case SIGNAL_HANDLER_ERROR:
        return "Signal handler error";
      case INVALID_INDEX_FOR_OPERATION_EXCEPTION:
        return "Invalid index for operation";
      case UNKNOWN_APPLICATION_ERROR:
        return "Unknow application error";
      case SERIES_SYNCHRONIZER_ERROR:
        return "Series synchronizer error";
      case CHART_ERROR:
        return "Chart error";
      case POSITION_ID_NOT_FOUND_ERROR:
        return "Position id not found";
      case OPERATION_ON_SERIES_SYNCED_TO_DIFFERENT_SYNCHRONIZERS_ERROR:
        return "Operation on series synced to different synchronizers";
      case POSITION_CLOSE_OPERATION_ON_OPEN_POSITION_ERROR:
        return "Operation requiring a closed position, but applied to an open position";
      case OPERATION_NOT_ALLOWED_ON_SYNCHRONIZED_SERIES_ERROR:
        return "Operation not allowed on synced series";
      case EXIT_STATMENT_CALL:
        return "Exit statement call";
      case INVALID_BARS_COLLECTION_ERROR:
        return "Invalid bars collection";
      case INVALID_POSITION_ERROR:
        return "Invalid position";
      case ARRAY_INDEX_NOT_FOUND_ERROR:
        return "Array index not found";
      case DICTIONARY_KEY_NOT_FOUND_ERROR:
        return "Dictionary key not found";
      case CLOSING_POSITION_ON_DIFFERENT_SYMBOL_ERROR:
        return "Closing position on different symbol";
      default:
        return "Unknown type";
    }
  }

  std::string categoryToString() const {
    switch (_category) {
      case error:
        return "Error";
      case warning:
        return "Warning";
      default:
        assert(false);
        return "";
    }
  }

  /**
   * Returns the name of the system that triggered the error
   *
   * @return The system name
   */
  const std::string& systemName() const { return _systemName; }

  /**
   * The symbol on which the system triggered the error
   *
   * @return The symbol
   */
  const std::string& symbol() const { return _symbol; }
  /**
   * Returns the error message
   *
   * @return The message
   */
  const std::string& message() const { return _message; }
  /**
   * Returns a string representation of the error event
   *
   * @return The string representation of the event
   */
  const std::string toString() const;
};

using ErrorEventPtr = std::shared_ptr< ErrorEvent >;

/**
 * Abstract class - base for error event sinks
 *
 * An error event sink is a class that is capable of receiving error events.
 * An error event sink must be thread safe, as multiple systems running in
 * different threads may be triggering errors
 *
 * The framework provides a predefined error event sink, which is implemented as
 * a queue - see ErrorEvent
 *
 * The application code can define its own implementation - see the samples for
 * different implementations of a different error sink
 *
 * It is implemented as a stack of events.
 *
 * @see ErrorEvent
 * @see createBasicErrorEventSink
 */
class CORE_API ErrorEventSink {
 public:
  virtual ~ErrorEventSink() {}
  /**
   * Pushes an event at the top of the stack
   *
   * @param event  The event to be pushed
   * @see ErrorEvent
   */
  virtual void push(const ErrorEventPtr event) = 0;
  /**
   * Remove the event at the top of the stack.
   */
  virtual void pop() = 0;
  /**
   * Returns a pointer to the event at the top of the stack, or 0 if the sink is
   * empty
   *
   * @return pointer to a ErrorEvent object at the front of the sink or 0 if the
   * sink is empty
   */
  virtual ErrorEventPtr front() const = 0;
  /**
   * Indicates whether the even sink stack is empty
   *
   * @return true if emtpy, false otherwise
   */
  virtual bool empty() const = 0;
  /**
   * indicates the number of error events in the stack
   *
   * @return number of events
   */
  virtual size_t size() const = 0;
};

/**
 * A "smart" pointer to an error event sink
 */
using ErrorEventSinkPtr = std::shared_ptr<ErrorEventSink>;
//@}

/**
 * does per thread init/uninit, before and after each system run
 * the implementation of init and uninit may need to be synchronized (made
 * thread safe) as the same initializer object is used for all threads in the
 * same SchedulerI object
 *
 * @see SchedulerI::setThreadInitializer
 */
class ThreadInitializer {
 public:
  virtual void init() = 0;
  virtual void uninit() = 0;
};

/**
 * The Runnable status information that the Scheduler will send to
 * RunnableRunInfoHandler objects
 *
 * At the end of running various runnables, the Scheduler will send out
 * information about the current status of the run, such as what symbol it was
 * run on, the duration of getting the data from the data source, the duration
 * of the actual run, if there were runtime errors or not.
 *
 * This information can then be used by a RunnableRunInfoHandler for example to
 * show it in a status window
 *
 * @see RunnableRunInfoHandler
 */
class RunnableRunInfo {
 private:
  const std::string _status;
  const std::string _symbol;
  const double _dataDuration;
  const double _runnableDuration;
  // number of data units for the run (bars, ticks etc)
  const unsigned __int64 _dataUnitCount;
  const bool _errors;
  const std::string& _threadName;
  const unsigned int _cpuNumber;

 public:
  /**
   * Constructor - sets the status information
   *
   * @param status
   * @param symbol
   * @param dataDuration
   * @param runnableDuration
   * @param errors
   */
  RunnableRunInfo(const std::string& status, const std::string& symbol, double dataDuration, double runnableDuration,
                  unsigned __int64 dataUnitCount, bool errors, const std::string& threadName)
      : _status(status), _symbol(symbol), _dataDuration(dataDuration),
        _runnableDuration(runnableDuration), _errors(errors),
        _dataUnitCount(dataUnitCount), _threadName(threadName), _cpuNumber(getCurrentCPUNumber()) {}

  /**
   *
   * @return
   */
  const std::string& status() const { return _status; }
  const std::string& symbol() const { return _symbol; }
  double dataDuration() const { return _dataDuration; }
  double runnableDuration() const { return _runnableDuration; }
  const std::string& threadName() const { return _threadName; }
  unsigned long cpuNumber() const { return _cpuNumber; }
  unsigned __int64 dataUnitCount() const { return _dataUnitCount; }
  bool errors() const { return _errors; }
};

class RunnableRunInfoHandler {
 public:
  virtual ~RunnableRunInfoHandler() {}
  virtual void status(const RunnableRunInfo& status) = 0;
};

/**
 * Controls the execution of the different Runnable instances on one or more
 * symbols lists, in multiple threads
 *
 * A Runnable is anything that can be run by the scheduler such as trading
 * systems, optimizers, or anything else that implements all the methods of the
 * abstract class Runnable. This mechanism provides a high level of flexibility
 * to the trading framework.
 *
 * The first step is to instantiate one or more Scheduler objects, by calling
 * the static method Scheduler::create.
 *
 * The user code will then instantiate a number of Runnable derived objects, and
 * pass them along with other parameters to the scheduler by calling the method
 * Scheduler::add.
 *
 * The class Simulation can be used instead of Scheduler, as it provides common
 * default values for some of the parameters.
 *
 * The Runnables can be run synchronously (the run method returns after all
 * Runnables have completed) or asynchronously (the run method returns
 * immediately).
 *
 *
 * @see Runnable
 * @see Simulator
 */
class CORE_API Scheduler {
 public:
  virtual ~Scheduler() {}
  /**
   * Sets the thread initializer associated with the scheduler
   *
   * By default no thread initializer is used (a 0 value is set internally) so
   * calling this method is optional, but if the user code needs to do special
   * per thread init/uninit, than a ThreadInitializerI derived class can be
   * defined and an instance passed to this method.
   *
   * Please note that all threads will use the same thread initializer object.
   *
   * @param threadInitializer
   *               A pointer to the ThreadInitializer derived object. If 0, no
   * thread initializer will be used
   * @see ThreadInitializer
   */
  virtual void setThreadInitializer(ThreadInitializer* threadInitializer) = 0;
  /**
   * Adds a Runnable to the list of Runnables to be run by the Scheduler and its
   * runtime parameters:
   * - the list of symbols on which to run it,
   * - a positions container (could be empty or not) which will receive the
   * positions created during the run
   * - an error event sink which will receive the error events triggered during
   * the runs
   * - a Slippage class
   * - a Commission class
   * - an signal handler
   *
   * Some or all of these parameters can be common for more than one Runnable.
   *
   * @param runnable   Pointer to a runnable object to be added
   * @param s          A pointer to a symbols list iterator
   * @param pos        A pointer to a positions container
   * @param es         A pointer to an error event sink
   * @param slippage   A pointer to a slippage handler, or 0 if no slippage
   * handler is to be used
   * @param commission A pointer to a commission handler, or 0 if no commission
   * handler is to be used
   * @param signalHandler
   *                   A pointer to an signal handler, or 0 if no signal handler
   * is to be used
   * @see Runnable
   * @see System
   * @see PositionsContainer
   * @see ErrorEventSink
   * @see Slippage
   * @see Commission
   * @see signalHandler
   */
  virtual void addRunnable( Runnable* runnable, DataInfoIteratorPtr s, PositionsVector& pos, ErrorEventSink* es, SignalHandler* signalHandler = 0,
      RunnableRunInfoHandler* runnableRunInfoHandler = 0, Slippage* slippage = 0, Commission* commission = 0, chart::ChartManager* chartManager = 0,
      const tradery::ExplicitTrades* explicitTrades = 0) = 0;
  virtual void addRunnable(Runnable* runnable, DataInfoIteratorPtr s, PositionsVector& pos, ErrorEventSink* es, const std::vector<SignalHandler*>& signalHandlers,
      RunnableRunInfoHandler* runnableRunInfoHandler = 0, Slippage* slippage = 0, Commission* commission = 0,
      chart::ChartManager* chartManager = 0, const tradery::ExplicitTrades* explicitTrades = 0) = 0;

  /**
   * Starts the execution of all Runnable objects added to the Scheduler
   *
   * Depending on the first parameter, the run can be asynchronous (this call
   * returns immediately, while the Runnable instances continue to run), or
   * synchronous (the call only returns after all runnables have completed their
   * runs).
   *
   * The method is not reentrant - the current run must finish before a new call
   * to run can be made, even for asynchronous calls.
   *
   * Multiple successive calls can be made assuming that before a new call the
   * status is not running (see Schedule::isRunning)
   *
   * Depending on the range parameter, the runnables can be run on all the
   * available bars, if range is 0, or on a limited range. The range can be time
   * based or bar index based.
   *
   * @param asynch runs in asynchrnous mode if true, or synchronous mode if
   * false
   * @param range  The range on which the runnables will be run on. The range
   * can be a time range or bar range. If 0, it will run on all available data.
   * @exception SchedulerReentrantRunCallException
   *                   Thrown if the mthod is called while there are already
   * running threads started by a previous call to runAsync
   * @see isRunning
   * @see cancelSync
   * @see cancelAsync
   * @see Range
   * @see TimeRange
   * @see BarRange
   * @see Runnable
   */
  virtual void run(bool asynch, unsigned long threads, bool cpuAffinity, DateTimeRangePtr range = 0, DateTime startTradesDateTime = DateTime())= 0;
  /**
   * creates an instance of a SchedulerI object
   *
   * The parameter specify the number of threads to run the Runnable objects in
   *
   * @param nbThreads specifies how many different threads to create during the
   * run.
   * @param printStatus
   *                  Controls the printing of runtime status info - for debug
   * or demo purposes for now. Print if true, no print if false
   * @return pointer to a Scheduler object
   */
  static Scheduler* create(RunEventHandler* runEventHandler = 0);
  /**
   * Indicates the running status of the SchedulerI object
   *
   * Note: the scheduler can be running and canceling at the same time, as
   * before it finishes, it will still try to end the current run
   *
   * @return true if at least one system thread is running, false if no system
   * threads are running
   */
  virtual bool isRunning() const = 0;
  /**
   * Indicates the canceling status of the SchedulerI object
   *
   * If true, the Scheduler is in the process of canceling the current run
   *
   * Note: the scheduler can be running and canceling at the same time, as
   * before it finishes, it will still try to end the current run
   *
   * @return true if the scheduler has a received a cancel order but has not
   * completed the cancel
   */
  virtual bool isCanceling() const = 0;

  /**
   * Cancels the current run synchronously, i.e. the call will return only after
   * there are no more threads running
   *
   * @see runAsync
   */
  virtual void cancelSync() = 0;
  /**
   * Cancels the current run asynchrnously, i.e. it returns immediately,
   * without waiting for the system threads to stop.
   *
   * When using this call, the user will usually have to check the running
   * status using isRunning before performing some other action on the
   * SchedulerI object
   *
   * @see isRunning
   */
  virtual void cancelAsync() = 0;
  virtual void resetRunnables() = 0;
};

/**
 * A "smart" pointer to a Scheduler object
 *
 * @see Scheduler
 */
using SchedulerPtr = std::shared_ptr<Scheduler>;

/**
 * Encapsulation of a Scheduler with default values for some parameters
 *
 * Provides default values to some of the parameters that are needed to run a
 * simulation.
 *
 * <!-- TODO: show example usage -->
 *
 * @see System
 * @see Scheduler
 */
class CORE_API Session {
 private:
  SchedulerPtr _defScheduler;
  //  SymbolsSource* _defSymbolsSource;

 public:
  /**
   * Constructor - takes the number of threads, user defined symbols info
   * source. print status and event sink as parameters
   *
   * The error sink is owned by the caller, so the caller will have to delete
   * this pointer
   *
   * This constructor will use these as default parameters for all the systems.
   * The error sink is optional, so if an error sink is not provided, a default
   * error sink will be created and used internally.
   *
   * Other runtime parameters are set on a per system basis (see addRunable)
   *
   * It is possible to override this default setting and use a different symbol
   * info iterator for each system by setting it in the addRunnable method.
   *
   * @param threads   Number of threads to start when running the runnables
   * @param symbolsInfoSource	The source of the symbols info used to load the
   * symbols data
   * @param printStatus
   *                  Controls the printing of runtime status info - for debug
   * or demo purposes for now. Print if true, no print if false. Default = false
   * @param errorSink
   * @exception SymbolsException
   *                   Thrown if there is an error with the symbols
   * @exception SymbolsListException
   * @see addRunnable
   * @see run
   * @see DefaultErrorSink
   * @see ErrorEventSink
   */
  Session(RunEventHandler* runEventHandler = 0);
  virtual ~Session();

 public:
  /**
   * Adds a runnable object
   *
   * Each runnable is associated with a DataInfoIterator, which holds info about
   * the symbols and datasouces to use for that runnable
   *
   * If the parameter symbolsListIterator is 0, the symbols list iterator set in
   * the constructor will be used. Otherwise, the one specified here will be
   * used.
   *
   * Multiple systems can use the same iterator, in which case all symbols will
   * be run exactly once by the group of runnables sharing the iterator. This is
   * useful in case multiple instances of the same systems are to be run in
   * different threads, and they will all run off of the same list of symbols.
   * The assumption is that the iterator is thread safe.
   *
   * @param runnable   The runnable object to be added . Cannot be 0. Normally a
   * System object, but can be other things (see optimization)
   * @param pos        pointer to a PositionsContainer object, to which the
   * system will add the position it creates during simulation run. Cannot be 0
   * @param slippage   Pointer to a Slippage object. Default value is not
   * slippage (=0)
   * @param commission pointer to a Commission object. Default value is no
   * commission (=0)
   * @param signalHandler
   *                   Sets the signalHandler object to be used by default, with
   * the default positions associated with this system
   * @param symbolsListIterator
   *                   Pointer to a SymbolsListIterator* object, default 0 -
   * using the classwide iterator set in constructor.
   *
   *                   Note: this pointer is owned externally, so it does not
   * need to and it should not be deleted in this class.
   */
  void addRunnable(Runnable* runnable, PositionsVector& pos, ErrorEventSink* errorSink, DataInfoIteratorPtr dataInfoIterator,
                   SignalHandler* signalHandler = 0, RunnableRunInfoHandler* runnableRunInfoHandler = 0, Slippage* slippage = 0, Commission* commission = 0,
                   chart::ChartManager* cm = 0, const tradery::ExplicitTrades* explicitTrades = 0);

  void addRunnable(Runnable* runnable, PositionsVector& pos, ErrorEventSink* errorSink, DataInfoIteratorPtr dataInfoIterator,
                   const std::vector<SignalHandler*>& signalHandlers, RunnableRunInfoHandler* runnableRunInfoHandler = 0,
                   Slippage* slippage = 0, Commission* commission = 0, chart::ChartManager* cm = 0,
                   const tradery::ExplicitTrades* explicitTrades = 0);

  /**
   * Starts the simulation process for all added runnables.
   *
   * See SchedulerI for more details of synchronous and asynchronous modes, and
   * on ranges.
   *
   * @param asynch Runs the systems in asynchronous mode if true, or synchronous
   * mode if false
   * @param range
   * @see SchedulerI
   * @see Range
   * @see TimeRange
   * @see BarRange
   */
  void run(bool asynch, unsigned int threads, bool cpuAffinity, DateTimeRangePtr range, const DateTime startTradesDateTime);
  /**
   * Indicates the running scheduler running status.
   *
   * @return true if any system thread is running, false if no system threads
   * are running
   */
  bool isRunning() const;

  /**
   * Indicates the running scheduler canceling status.
   *
   * @return true if any system thread is running, false if no system threads
   * are running
   */
  bool isCanceling() const;

  /**
   * Cancels the current run synchronously, i.e. the call will return only after
   * there are no more runnables running
   *
   * The cancel works by telling all threads to return after the current
   * runnable has completed its run, so depending on the system, the data etc,
   * the cancel process may not be instantaneous
   *
   * @see runAsync
   */
  void cancelSync() const;
  /**
   * Cancels the current run asynchrnously, i.e. it returns immediately,
   * without waiting for the runnables stop.
   *
   * The cancel works by telling all threads to return after the current
   * runnable has completed its run, so depending on the system, the data etc,
   * the cancel process may not be instantaneous
   *
   * When using this call, the user will usually have to check the running
   * status using isRunning before performing some other action on the
   * SchedulerI object
   *
   * @see isRunning
   */
  void cancelAsync() const;

  void resetRunnables();
};

/**
 * Registers a data source. This should be called before a data source is to be
 * used with simlib once a data source has been registered, the framework owns
 * it, i.e. it is the framework that is responsible for deleting the data source
 * object, unless it is unregistered, in which case the data source needs to be
 * deleted explicitely by the caller
 *
 * @param dataSource
 * @return
 * @exception DataSourceAlreadyRegisteredException
 */
CORE_API void registerDataSource(DataSource* dataSource);
// returns 0 if not registered
CORE_API bool unregisterDataSource(DataSource* dataSource);
// returns 0 if not registered
CORE_API bool unregisterDataSource(const UniqueId& dataSourceId);
//<!-- TODO: temporary -->

CORE_API void init(unsigned int cacheSize);
CORE_API void uninit();
CORE_API void setDataCacheSize(unsigned int cacheSize);
}  // namespace tradery
