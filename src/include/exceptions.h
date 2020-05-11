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

#include "misc.h"

/** @file
 *  \brief Contains all the SimLib exception classes

    From the user perspective, there are two categories of exceptions in this
 framwork
  #- exceptions thrown internally by simlib. These are already defined here
  #- exceptions thrown from an external module that is used by simlib, such as
 datasources, symbol lists etc. User defined external modules could either use
 these exceptions directly or they can create derived exception classes and
 throw these in case of errors, but they need to be of the types defined in this
 module, or the framework would not be able to catch and handle them.

  As for handling exceptions, it is ok to catch and handle these in the system
 code, if the user finds it useful or necessary. But it is recommended that the
 error event sink model be used instead. This is done by allowing the framwork
 to catch and handle these exceptions, and set and ErrorEventSink to receive
 error notifications asyncronously (outside of all trading system threads) from
 the framework. One advantage is that this works better in a multithreaded
 environment, as exceptions cannot be thrown outside of a thread, and also this
 offers a complete separation between the user code and error handling

  The samples provided illustrate how to use the error event sink model.

*/

///
/// @cond
///
namespace tradery
///
/// @endcond
///
{
/**
 * Defines various error codes
 *
 * Each exception has an error code, indicating the nature of the error to the
 * calling application.
 */
enum ErrorCode {
  DATA_INDEX_OUT_OF_RANGE_ERROR,
  SERIES_INDEX_OUT_OF_RANGE_ERROR,
  SYNCHRONIZED_SERIES_INDEX_OUT_OF_RANGE_ERROR,
  TIME_SERIES_INDEX_OUT_OF_RANGE_ERROR,
  OPERATION_ON_UNEQUAL_SIZE_SERIES_ERROR,
  CLOSING_ALREADY_CLOSED_POSITION_ERROR,
  COVERING_LONG_POSITION_ERROR,
  SELLING_SHORT_POSITION_ERROR,
  OPENING_BARS_FILE_ERROR,
  OPENING_TICKS_FILE_ERROR,
  POSITION_INDEX_OUT_OF_RANGE_ERROR,
  DATA_SOURCE_ERROR,
  DATA_SOURCE_NOT_REGISTERED_ERROR,
  DATA_SOURCE_ALREADY_REGISTERED_ERROR,
  DATA_SOURCE_FORMAT_ERROR,
  SYMBOLS_LIST_ERROR,
  SYMBOLS_INFO_ERROR,
  DATA_ERROR,
  UNKNOWN_DATA_TYPE,
  BAR_ERROR,
  SCHEDULER_REENTRANT_CALL_ERROR,
  POSITION_CLOSE_OPERATION_ON_OPEN_POSITION_ERROR,
  POSITION_ZERO_PRICE_ERROR,
  UNKNOWN_STRUCTURED_ERROR,
  ACCESS_VIOLATION_ERROR,
  DIVIDE_BY_ZERO_ERROR,
  BAR_SYSTEM_RECEIVED_NON_BAR_DATA_ERROR,
  TICK_SYSTEM_RECEIVED_NON_TICK_DATA_ERROR,
  DATE_STRING_ERROR,
  TIME_STRING_ERROR,
  SIGNAL_HANDLER_ERROR,
  INVALID_INDEX_FOR_OPERATION_EXCEPTION,
  PLUGIN_ERROR,
  SERIES_SYNCHRONIZER_ERROR,
  GENERAL_SYSTEM_ERROR,
  EXPLICIT_TRADES_NOT_AVAILABLE,
  OPERATION_ON_SERIES_SYNCED_TO_DIFFERENT_SYNCHRONIZERS,
  OPERATION_NOT_ALLOWED_ON_SYNCHRONIZED_SERIES,
  POSITION_ID_NOT_FOUND_ERROR,
  DICTIONARY_KEY_NOT_FOUND_EXCEPTION,
  ARRAY_INDEX_NOT_FOUND_EXCEPTION,
  EXIT_STATEMENT_CALL_EXCEPTION,
  INVALID_BARS_COLLECTION_EXCEPTION,
  INVALID_POSITION_EXCEPTION,
  CLOSING_POSITION_ON_DIFFERENT_SYMBOL_EXCEPTION,
  INVALID_LIMIT_PRICE_EXCEPTION,
  INVALID_STOP_PRICE_EXCEPTION,
  COULD_NOT_LOAD_SYSTEM_FILE
};

/**
 * Base class for all SimLib exceptions.
 *
 * Provides basic functionality that must be supported by all exceptions
 * - error code
 * - message
 */
class CoreException {
 private:
  const ErrorCode _code;
  std::string _message;

 protected:
  /**
   * Sets the exception message
   *
   * The message is received as a string
   *
   * @param str    THe message string
   */
  void setMessage(const std::string& str) { _message = str; }

  /**
   * Sets the exception message
   *
   * The message is received as a std::wostringstream.
   *
   * @param str    The string stream containing the message
   * @see std::ostringstream
   * @see std::wostringstream
   */
  void setMessage(const std::ostringstream& str) { _message = str.str(); }

 public:
  /**
   * Constructor that takes an error code as only parameter
   *
   * @param code   Error code
   * @return
   * @see ErrorCode
   */
  explicit CoreException(ErrorCode code) : _code(code) {}

  /**
   * Constructor that takes an error code and a pointer to a string as
   * parameters
   *
   * @param code    error code
   * @param message message
   * @return
   * @see ErrorCode
   */
  explicit CoreException(ErrorCode code, const char* message)
      : _code(code), _message(message) {}

  /**
   * Consructor that takes an error code and a reference to a std::wstring as
   * parameters
   *
   * @param code    error code
   * @param message message
   * @return
   * @see ErrorCode
   * @see std::wstring
   */
  explicit CoreException(ErrorCode code, const std::string& message)
      : _code(code), _message(message) {}

  virtual ~CoreException() {}

  /**
   * Returns the exception message
   *
   * @return message
   */
  const std::string& message() const { return _message; }

  /**
   * Returns the error code of the exception
   *
   * @return Error code
   * @see ErrorCode
   */
  ErrorCode code() const { return _code; }
};

/**
 * Structured exception thrown in case of an access violation exception
 *
 * This exception is never thrown in simlib presently and it is here for future
 * use only
 *
 * @see CoreException
 * @see ErrorCode
 */
class AccessViolationException {
 public:
  virtual ~AccessViolationException() {}
  virtual std::string message() const = 0;
};

/**
 * Thrown in case of a divizion by zero error
 *
 * @see CoreException
 * @see ErrorCode
 */
class DivideByZeroException {
 public:
  virtual ~DivideByZeroException() {}
  virtual std::string message() const = 0;
};

/**
 * Base class for more specialized trading systms exceptions
 *
 * More common specialized trading system are bar based systems and tick based
 * systems, but other more exotic types could be implemented
 *
 * @see TickSystemException
 * @see BarSystemException
 * @see CoreException
 */
class SystemException : public CoreException {
 private:
 public:
  SystemException(ErrorCode code, const std::string& message)
      : CoreException(code, message) {}
};

class GeneralSystemException : public SystemException {
 public:
  GeneralSystemException(const std::string& message)
      : SystemException(GENERAL_SYSTEM_ERROR, message) {}
};

/**
 * Exception thrown when an error specific to a bar based system happens
 *
 * An example of such an error would be feeding other type of data (such as
 * tick) to a system that expects bars
 *
 * @see CoreException
 * @see ErrorCode
 */
class BarSystemException : public SystemException {
 public:
  BarSystemException(ErrorCode code, const std::string& message)
      : SystemException(code, message) {}
};

/**
 * Exception thrown when an error specific to a tick based system happens
 *
 * An example of such an error would be feeding other type of data (such as
 * bars) to a system that expects ticks
 *
 * @see CoreException
 * @see ErrorCode
 */
class TickSystemException : public SystemException {
 public:
  TickSystemException(ErrorCode code, const std::string& message)
      : SystemException(code, message) {}
};

/**
 * Thrown when trying to access a bar with a highr index than the number of bars
 * in a BarsI bar collection
 *
 * @see BarsI
 * @see CoreException
 * @see ErrorCode
 */
class DataIndexOutOfRangeException : public CoreException {
 private:
  size_t _size;
  size_t _index;
  const std::string _symbol;

  // TODO: set the exception code
 public:
  DataIndexOutOfRangeException(size_t size, size_t index, const std::string& symbol)
      : _size(size), _index(index), CoreException(DATA_INDEX_OUT_OF_RANGE_ERROR), _symbol(symbol) {
    std::ostringstream _o;
    _o << "Index out of range exception - size: " << _size << ", index: " << _index << " on: " << _symbol;
    setMessage(_o);
  }

  /**
   * Returns the size of the BarsI bars collection object
   *
   * @return the size of BarsI object
   */
  size_t getSize() const { return _size; }

  /**
   * Returns the actual index that triggered the exception
   *
   * @return The index that is out of range
   */
  size_t getIndex() const { return _index; }
};

class TickIndexOutOfRangeException : public DataIndexOutOfRangeException {
 public:
  TickIndexOutOfRangeException(size_t size, size_t index, const std::string& symbol)
      : DataIndexOutOfRangeException(size, index, symbol) {}
};

class BarIndexOutOfRangeException : public DataIndexOutOfRangeException {
 public:
  BarIndexOutOfRangeException(size_t size, size_t index, const std::string& symbol)
      : DataIndexOutOfRangeException(size, index, symbol) {}
};

class InvalidPriceException : public CoreException {
 private:
  size_t _barIndex;
  double _price;

 public:
  InvalidPriceException(size_t barIndex, double price, ErrorCode code)
      : CoreException(code, makeMessage(barIndex, price)) {}

 private:
  std::string makeMessage(size_t barIndex, double price) {
    return tradery::format( "bar ", barIndex, ", price ", price );
  }
};

class InvalidStopPriceException : public InvalidPriceException {
 public:
  InvalidStopPriceException(size_t barIndex, double price)
      : InvalidPriceException(barIndex, price, INVALID_STOP_PRICE_EXCEPTION) {}
};

class InvalidLimitPriceException : public InvalidPriceException {
 public:
  InvalidLimitPriceException(size_t barIndex, double price)
      : InvalidPriceException(barIndex, price, INVALID_LIMIT_PRICE_EXCEPTION) {}
};

/**
 * Thrown when trying to access an elment with higher index than the number of
 * elements in a SeiesI collection of values
 *
 * @see SeriesI
 * @see CoreException
 * @see ErrorCode
 */
class SeriesIndexOutOfRangeException : public CoreException {
 private:
  size_t _size;
  size_t _index;

 public:
  // TODO: set the right code
  SeriesIndexOutOfRangeException(size_t size, size_t index)
      : _size(size), _index(index), CoreException(SERIES_INDEX_OUT_OF_RANGE_ERROR) {
    std::ostringstream _o;
    _o << "Series index out of range - size: " << _size << ", index: " << _index;
    setMessage(_o);
  }

  /**
   * Returns the size of the SeriesI object
   *
   * @return The size of the series
   */
  size_t getSize() const { return _size; }

  /**
   * The index that triggered the exception
   *
   * @return the index that is out of range
   */
  size_t getIndex() const { return _index; }
};

class SynchronizedSeriesIndexOutOfRangeException : public CoreException {
 private:
  size_t _size;
  size_t _index;

 public:
  SynchronizedSeriesIndexOutOfRangeException(size_t size, size_t index)
      : _size(size),  _index(index), CoreException(SYNCHRONIZED_SERIES_INDEX_OUT_OF_RANGE_ERROR) {
    std::ostringstream _o;
    _o << "Synchronized series index out of range - size: " << _size << ", index: " << _index;
    setMessage(_o);
  }

  /**
   * Returns the size of the SeriesI object
   *
   * @return The size of the series
   */
  size_t getSize() const { return _size; }

  /**
   * The index that triggered the exception
   *
   * @return the index that is out of range
   */
  size_t getIndex() const { return _index; }
};

class TimeSeriesIndexOutOfRangeException : public CoreException {
 private:
  size_t _size;
  size_t _index;

 public:
  TimeSeriesIndexOutOfRangeException(size_t size, size_t index)
      : _size(size), _index(index), CoreException(TIME_SERIES_INDEX_OUT_OF_RANGE_ERROR) {
    std::ostringstream _o;
    _o << "Time Series index out of range - size: " << _size << ", index: " << _index;
    setMessage(_o);
  }

  /**
   * Returns the size of the SeriesI object
   *
   * @return The size of the series
   */
  size_t getSize() const { return _size; }

  /**
   * The index that triggered the exception
   *
   * @return the index that is out of range
   */
  size_t getIndex() const { return _index; }
};

class OperationNotAllowedOnSynchronizedseriesException : public CoreException {
 public:
  OperationNotAllowedOnSynchronizedseriesException()
      : CoreException(OPERATION_NOT_ALLOWED_ON_SYNCHRONIZED_SERIES) {}
};

class OperationOnSeriesSyncedToDifferentSynchronizers : public CoreException {
 public:
  OperationOnSeriesSyncedToDifferentSynchronizers()
      : CoreException(OPERATION_ON_SERIES_SYNCED_TO_DIFFERENT_SYNCHRONIZERS, "Error trying to perform an operation between 2 synchronized series") {}
};

/**
 * Trigerred when the series operands of an operation performed on multiple
 * series have different sizes
 *
 * Normally, operation such as adding, multiplying, dividing etc of two series
 * should be performed on series of equal length. If this is not true, then this
 * exception is thrown.
 *
 * @see SeriesI
 * @see CoreException
 * @see ErrorCode
 */
class OperationOnUnequalSizeSeriesException : public CoreException {
 private:
  size_t _size1;
  size_t _size2;

  // TODO: set the right code
 public:
  OperationOnUnequalSizeSeriesException(size_t size1, size_t size2)
      : _size1(size1), _size2(size2), CoreException(OPERATION_ON_UNEQUAL_SIZE_SERIES_ERROR) {
    std::ostringstream _o;
    _o << "Operation on series of unequal sizes - size1: " << _size1 << ", size2: " << _size2;
    setMessage(_o);
  }
  /**
   * Returns the size in elements of the first series of the operation
   *
   * @return The size of the first operand
   */
  size_t getSize1() const { return _size1; }

  /**
   * Returns the size in elements of the second series of the operation
   *
   * @return The size of the second operand
   */
  size_t getSize2() const { return _size2; }
};

class InvalidIndexForOperationException : public CoreException {
 private:
  size_t _index;
  const std::string _operationName;

 public:
  InvalidIndexForOperationException(size_t index, const std::string& operationName)
      : _index(index), _operationName(operationName), CoreException(INVALID_INDEX_FOR_OPERATION_EXCEPTION) {
    std::ostringstream o;

    o << "Invalid index: " << _index << " in " << _operationName;
    setMessage(o);
  }

  size_t getIndex() const { return _index; }
  const std::string& getOperationName() const { return _operationName; }
};

/**
 * Thrown if the user code attempts to close (sell or cover) a position that has
 * already been closed
 *
 * @see PositionsI
 * @see CoreException
 * @see ErrorCode
 */
class ClosingAlreadyClosedPositionException : public CoreException {
 public:
  /**
   */
  ClosingAlreadyClosedPositionException()
      : CoreException(CLOSING_ALREADY_CLOSED_POSITION_ERROR, "Closing already closed position") {}
};

/**
 * Thrown when attempting to cover a long position
 *
 * A long position should be closed by selling it
 *
 * @see PositionsI
 * @see ErrorCode
 * @see CoreException
 */
class CoveringLongPositionException : public CoreException {
 public:
  /**
   */
  CoveringLongPositionException()
      : CoreException(COVERING_LONG_POSITION_ERROR, "Covering long position") {}

  CoveringLongPositionException(const std::string& str)
      : CoreException(COVERING_LONG_POSITION_ERROR, str.c_str()) {}
};

/**
 * Thrown when attempting to sell a short position
 *
 * A short position should be closed by covering it
 *
 * @see PositionsI
 * @see ErrorCode
 * @see CoreException
 */
class SellingShortPositionException : public CoreException {
 public:
  // TODO: set the right code
  SellingShortPositionException()
      : CoreException(SELLING_SHORT_POSITION_ERROR, "Selling short position") {}

  SellingShortPositionException(const std::string& str)
      : CoreException(SELLING_SHORT_POSITION_ERROR, str.c_str()) {}
};

/**
 * General data source exception.
 *
 * Data sources can either throw it directly or derive more specialized
 * exceptions from it and throw those, but the rule is that each data source
 * should throw a DataSourceException type excption.
 *
 * @see CoreException
 */
class DataSourceException : public CoreException {
 private:
  const std::string _dataSourceName;

 public:
  /**
   * Constructor taking an error code, a message and the name of the data source
   *
   * <!-- TODO: the error code scheme should be different
   * - one global error code identifying all data source errors
   * - one error code specific to the data source that threw the exception -->
   *
   * @param errorCode the error code
   * @param message   the error message
   * @param dataSourceName
   *                  the name of the data source that threw the exception
   * @see CoreException
   * @see ErrorCode
   */
  DataSourceException(ErrorCode errorCode, const std::string& message, const std::string& dataSourceName)
      : _dataSourceName(dataSourceName), CoreException(errorCode, message) {}

  /**
   * Returns the name of the data source that threw the exception
   *
   * @return The name of the data source
   */
  const std::string& getDataSourceName() const { return _dataSourceName; }
};

/**
 * Thrown when trying to register the same data source twice
 *
 * @see registerDataSource
 * @see CoreException
 */
class DataSourceAlreadyRegisteredException : public CoreException {
 private:
  const std::string _dataSourceName;

 public:
  DataSourceAlreadyRegisteredException(const std::string& dataSourceName)
      : _dataSourceName(dataSourceName), CoreException(DATA_SOURCE_ALREADY_REGISTERED_ERROR, "Data source already registered: "s + dataSourceName + "\n") {}

  /**
   * Returns the name of the data source that triggered the exception
   *
   * @return The name of the data source
   */
  const std::string& getDataSourceName() const { return _dataSourceName; }
};

/**
 * Thrown when trying to register the same data source twice
 *
 * @see registerDataSource
 * @see CoreException
 */
class DataSourceNotRegisteredException : public CoreException {
 private:
  const std::string _dataSourceName;

 public:
  DataSourceNotRegisteredException(const std::string& dataSourceName)
      : _dataSourceName(dataSourceName), CoreException(DATA_SOURCE_NOT_REGISTERED_ERROR, "Data source not registered: "s + dataSourceName + "\n") {}

  /**
   * Returns the name of the data source that triggered the exception
   *
   * @return The name of the data source
   */
  const std::string& getDataSourceName() const { return _dataSourceName; }
};

/**
 * Thrown in case of a symbols list exception
 *
 * There can be any different types of symbols list (such as file based, db
 * based etc) and they will either throw this very exception or will throw
 * exceptions derived from it, which they have to define
 *
 * @see SymbolsListSourceI
 * @see SymbolsListIteratorI
 * @see CoreException
 */
class SymbolsInfoException : public CoreException {
 public:
  SymbolsInfoException(const std::string& message)
      : CoreException(SYMBOLS_INFO_ERROR, message) {}
};

class SymbolsSourceException : public CoreException {
 public:
  SymbolsSourceException(const std::string& message)
      : CoreException(SYMBOLS_LIST_ERROR, message) {}
};

/**
 * Thrown if a bar is not well formed, i.e. the low is higher than the high or
 * similar
 *
 * @see Bar
 * @see CoreException
 */
class BarException : public CoreException {
 public:
  /**
   * Constructor taking all the bar info as parameters
   *
   * @param time   The date and time of the bar
   * @param open   Open price
   * @param low    Low price
   * @param high   High price
   * @param close  Close price
   */
  BarException(const std::string& message)
      : CoreException(BAR_ERROR, message) {}
};

/**
 * Thrown if the user code attempts to run the scheduler while it is already
 * running in asynchronous mode
 *
 * @see SchedulerI
 * @see CoreException
 */
class SchedulerReentrantRunCallException : public CoreException {
 public:
  /**
   * Default constructor
   */
  SchedulerReentrantRunCallException()
      : CoreException(SCHEDULER_REENTRANT_CALL_ERROR, "Scheduler reentrant run call not allowed") {}
};

/**
 * Thrown if an operation that requires a closed position is performed on a
 * still open position
 *
 * Such operations include requesting the time a position was closed, or the
 * closing price for example
 *
 * @see CoreException
 * @see Position
 */
class PositionCloseOperationOnOpenPositionException : public CoreException {
 public:
  PositionCloseOperationOnOpenPositionException(const std::string& message)
      : CoreException(POSITION_CLOSE_OPERATION_ON_OPEN_POSITION_ERROR, "Cannot perform an operation that require a closed "
                      "position on a still open position - test if the position is closed first") {}
};

class PositionZeroPriceException : public CoreException {
 public:
  PositionZeroPriceException()
      : CoreException(POSITION_ZERO_PRICE_ERROR, "A position entry or exit price is 0") {}
};

class SignalHandlerException : public CoreException {
 private:
  const std::string _name;

 public:
  SignalHandlerException(const std::string& name, const std::string& message)
      : CoreException(SIGNAL_HANDLER_ERROR, message), _name(name) {}

  const std::string& name() const { return _name; }
};

class PluginException : public CoreException {
  const std::string _name;

 public:
  PluginException(const std::string& name, const std::string& message)
      : CoreException(PLUGIN_ERROR, message), _name(name) {}
  const std::string& name() const { return _name; }
};

class SeriesSynchronizerException : public CoreException {
 public:
  SeriesSynchronizerException(const std::string& message)
      : CoreException(SERIES_SYNCHRONIZER_ERROR, message) {}
};

class ExitRunnableException : public CoreException {
 public:
  ExitRunnableException(const std::string& exitMessage = "")
      : CoreException(EXIT_STATEMENT_CALL_EXCEPTION, "System stopped due to exit() call: "s + exitMessage) {}
};

class InvalidBarsCollectionException : public CoreException {
 public:
  InvalidBarsCollectionException(const std::string& symbol)
      : CoreException(INVALID_BARS_COLLECTION_EXCEPTION, "Trying to use an invalid bars collection. Check if data is available for \""s + symbol + "\"") {}
};

class InvalidPositionException : public CoreException {
 public:
  InvalidPositionException()
      : CoreException(INVALID_POSITION_EXCEPTION, "Trying to use an invalid Position object") {}
};

class ClosingPostionOnDifferentSymbolException : public CoreException {
 public:
  ClosingPostionOnDifferentSymbolException(const std::string& originalSymbol, const std::string& newSymbol)
      : CoreException(CLOSING_POSITION_ON_DIFFERENT_SYMBOL_EXCEPTION, "Closing position on different symbol. Original symbol: "s + originalSymbol + ", new symbol: " + newSymbol) {}
};

}  // namespace tradery
