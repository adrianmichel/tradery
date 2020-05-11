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
#pragma warning(disable : 4290)

#ifdef DATASOURCE_EXPORTS
#define DATASOURCE_API __declspec(dllexport)
#else
#define DATASOURCE_API __declspec(dllimport)
#endif

/**
 * @file
 *  \brief Contains all data source and symbols info source adapters related
 * definitions
 *
 * A data source adapter is a module that sits between the simlib framework and
 * a specific data source, such as the file system, an Internet based real time
 * quote provider, a historical quotes data base etc.
 *
 * Data sources can connect to arbitrary types of data. However, currently the
 * Framework has support for bar and tick data. Other types, although handled by
 * the framework due to polymorphic implementtion of data types, will have to be
 * understood by user code.
 *
 * This file defines classes that constitute the common protocols that all data
 * source adapters need to support in order to communicate with the SimLib
 * framework.
 *
 * A symbols info source adapter is a module that can iterate through
 * collections of symbols. More specifically, a symbols info source adapter
 * needs to be able to read lists of symbols stored on some medium or even
 * generate them on the fly, and be prepared to iterate through the collection
 * of symbols and generate all the information needed by the framework to
 * retrieve the data associated with each symbol and run various systems using
 * this data.
 *
 */

#include "exceptions.h"
#include "series.h"
#include "filesymbols.h"
#include "synchronizer.h"
#include "macros.h"
#include "dataunit.h"

/** @cond */
namespace tradery {
/** @endcond */

/**
 * \brief Base class for user defined bar extra info.
 *
 * Bar data can have
 *
 * BarExtraInfo derived classes can contain arbitrary per bar data, such as
 * fundamental data or anything else
 */

  class BarExtraInfo;
  using BarExtraInfoPtr = std::shared_ptr< BarExtraInfo >;

  class BarExtraInfo {
 public:
  virtual ~BarExtraInfo() {}

  virtual BarExtraInfoPtr clone() const = 0;

 public:
  virtual std::wostream& dump(std::wostream& os) const = 0;
};

/**
 * \brief A series of XTime values, implemented as a vector
 *
 * @see XTime
 */
using TimeSeriesImpl = std::vector<DateTime>;
using TimeSeriesPtr = std::shared_ptr<TimeSeriesImpl>;

class TimeSeries {
 private:
  TimeSeriesPtr _ts;
  Synchronizer::SynchronizerPtr _s;

 public:
  TimeSeries(TimeSeriesPtr ts) : _ts(ts) {}

  TimeSeries() : _ts(std::make_shared< TimeSeriesImpl >()) {}

  void synchronize(Synchronizer::SynchronizerPtr s) { _s = s; }

  size_t size() const { return _ts->size(); }

  void push_back(const DateTime& dt) { _ts->push_back(dt); }

  const DateTime& at(size_t index) const {
    assert(_ts);
    if (index < _ts->size()) {
      return _ts->at(index);
    }
    else {
      throw TimeSeriesIndexOutOfRangeException(_ts->size(), index);
    }
  }

  const DateTime& operator[](size_t index) const {
    if (_s) {
      return _s->timeSeries()[index];
    }
    else {
      return at(index);
    }
  }

  using DateTimePair = std::pair<DateTime, DateTime>;
  DateTimePair unsyncStartEnd() const {
    return _ts->size() > 0 ? DateTimePair((*_ts)[0], (*_ts)[_ts->size() - 1]) : DateTimePair{};
  }
};

/**
 * \brief A series of BarExtraInfo objects, implemented as a vector of constant
 * pointers (CosntPtrVector) BarExtraInfo objects
 *
 * @see ConstPtrVector
 */
using ExtraInfoSeries = ConstPtrVector<BarExtraInfo>;

/**
 * \brief A bar - specialized and concrete type of data unit
 * Bar objects are only used while Bars collections are populated. They are not
 * used internally by the framework, as Bars collections have their own internal
 * representation of data.
 *
 * A bar has:
 * - open price,
 * - low price,
 * - high price,
 * - close price
 * - volume,
 * - a time stamp (in the base class)
 * - open interest (otional, for futures)
 * - extra info such as fundamental data or other (optional)
 *
 * @see DataUnit
 * @see BarExtraInfo
 * @see Bars
 */
class Bar : public DataUnit {
  enum BarStatus {
    valid,
    empty,
    highLTopenError,
    highLTlowError,
    highLTcloseError,
    lowHTopenError,
    lowHTcloseError,
    volume0Error
  };

 private:
  const double _open;
  const double _low;
  const double _high;
  const double _close;
  const unsigned long _volume;
  const unsigned long _openInterest;
  const BarExtraInfoPtr _barExtraInfo;

  BarStatus _status;

 private:
  BarStatus status(const DateTime& time, double open, double high, double low, double close, unsigned long volume) {
    if (open == 0 && high == 0 && low == 0 && close == 0 && volume == 0) {
      return empty;
    }
    else if (high < open) {
      return highLTopenError;
    }
    else if (high < low) {
      return highLTlowError;
    }
    else if (high < close) {
      return highLTcloseError;
    }
    else if (low > open) {
      return lowHTopenError;
    }
    else if (low > close) {
      return lowHTcloseError;
    }
    else if (0 == volume) {
      return volume0Error;
    }
    else {
      return valid;
    }
  }

 public:
  /**
   * Constructor - takes all the values that define a bar as parameters
   *
   * Open interest and bar extra info are optional.
   *
   * The bar extra info object, if needed, must to be created with new in the
   * datasource code, and the resulting pointer will be passed to the Bars
   * object being populated, so the Bars collection object will own the
   * BarsExtraInfo object and be responsible of its deletion
   *
   * @param time   Bar time stamp
   * @param open   open price
   * @param high   high price
   * @param low    low price
   * @param close  close price
   * @param volume volume
   * @param openInterest
   *               optional open interest, set 0 if absent
   * @param barExtraInfo
   *               optional pointer to bar extra info, set to 0 if absent
   * @exception BarException
   *                   Thrown in case the bar data is not valid (such as high <
   * low)
   * @see BarExtraInfo
   * @see Bars
   * @see XTime
   */
  Bar(const DateTime& time, double open, double high, double low, double close, unsigned long volume, unsigned long openInterest = 0,
      const BarExtraInfoPtr barExtraInfo = BarExtraInfoPtr{})
      : _open(open),
        _low(low),
        _high(high),
        _close(close),
        _volume(volume),
        DataUnit(time),
        _openInterest(openInterest),
        _barExtraInfo(barExtraInfo),
        _status(status(time, open, high, low, close, volume)) {}

  Bar(DateTime& time)
      : _open(0),
        _low(0),
        _high(0),
        _close(0),
        _volume(0),
        DataUnit(time),
        _openInterest(0),
        _status(empty) {}

  virtual ~Bar() {}

 public:
  /**
   * Returns the open price
   *
   * @return open price
   */
  double getOpen() const { return _open; }
  /**
   * returns the low price
   *
   * @return low price
   */
  double getLow() const { return _low; }
  /**
   * return the high price
   *
   * @return high price
   */
  double getHigh() const { return _high; }
  /**
   * return the close price
   *
   * @return close price
   */
  double getClose() const { return _close; }
  /**
   * returns the volume
   *
   * @return volume
   */
  unsigned long getVolume() const { return _volume; }
  /**
   * returns the open interest or 0 if no open interest
   *
   * @return open interest
   */
  unsigned long getOpenInterest() const { return _openInterest; }
  /**
   * pointer to the bar extra info object
   *
   * @return bar extra info
   */
  const BarExtraInfoPtr getBarExtraInfo() const { return _barExtraInfo; }
  /**
   * sends a string representation of the bar object to an output stream
   *
   * @param os     output stream
   * @return reference to the output stream
   * @see t_ostream
   */
  /*  t_ostream& dump( t_ostream& os) const
  {
  std::wostringstream o;
  o << time().to_simple_string().c_str() << _T( ", " ) << _open << _T( ", " ) <<
  _high << _T( ", " ) << _low << _T( ", " ) << _close << _T( ", " ) <<
  _openInterest << std::endl; if( _barExtraInfo != 0 ) o << _barExtraInfo ->
  dump(os ); return os;
  }
  */
  bool isValid() const { return _status == valid; }
  BarStatus getStatus() const { return _status; }
  std::string getStatusAsString() const {
    std::string str = date().toString() + ": ";
    switch (_status) {
      case valid:
        return str + "valid";
      case empty:
        return str + "empty";
      case highLTopenError:
        return str + "high < open";
      case highLTlowError:
        return str + "high < low";
      case highLTcloseError:
        return str + "high < close";
      case lowHTopenError:
        return str + "low > open";
      case lowHTcloseError:
        return str + "low > close";
      case volume0Error:
        return str + "volume 0";
      default:
        assert(false);
        return "";
    }
  }
};

/**
 * An automatic (smart) pointer to a bar
 *
 * @see Bar
 */
using BarPtr = std::shared_ptr<const Bar>;

/**
 * Different tick types.
 *
 * For now:
 * - bid price
 * - ask price
 * - best bid price
 * - best ask price
 * - trade
 */
enum TickType { BID, ASK, BEST_BID, BEST_ASK, TRADE };

/**
 * A series of tick types implemented as a vector of TickType values
 */
using TickTypeSeries = std::vector<TickType>;
/**
 * A series of string representing the exchange info in the tick
 */
using ExchangeSeries = std::vector<std::wstring>;

/**
 * \brief A tick
 *
 * Contains basic data (price, size, exchange, type), but can be expanded to
 * contain anything that is relevant to a particular data source
 *
 * A tick is not used as is by the framwork. Like any other data unit, it is
 * mainly used when the collection of ticks is being populated
 */
class Tick : public DataUnit {
 private:
  const double _price;
  const unsigned long _size;
  const std::wstring _exchange;
  const TickType _type;

 public:
  /**
   * Constructor - takes all values that define a tick as parameters:
   * - time stamp
   * - price
   * - size
   * - type
   * - exchange
   *
   * @param time
   * @param price
   * @param size
   * @param type
   * @param exchange
   * @see DataUnit
   */
  Tick(const DateTime& time, double price, unsigned long size, TickType type, const std::wstring& exchange)
      : DataUnit(time),
        _price(price),
        _size(size),
        _type(type),
        _exchange(exchange) {}

  /**
   * Returns the price of the tick
   *
   * @return
   */
  double price() const { return _price; }

  /**
   * Returns the size of the tick
   *
   * @return
   */
  unsigned long size() const { return _size; }

  /**
   * Returns the type of the tick
   *
   * @return
   */
  const TickType type() const { return _type; }

  /**
   * Returns the exchange of the tick
   *
   * @return
   */
  const std::wstring& exchange() const { return _exchange; }
};

/**
 * An automatic (smart) pointer to a tick
 *
 * @see Tick
 */
using TickPtr = std::shared_ptr<const Tick>;

class Ticks;
class BarsAbstr;

class DataLocationInfo {
 public:
  virtual ~DataLocationInfo() {}
  DataLocationInfo() {}

  virtual const std::string toXML() const = 0;
};

class DataFileLocationInfo : public DataLocationInfo {
 private:
  const std::string m_fileName;
  const unsigned __int64 m_startPos;
  const unsigned __int64 m_count;

 public:
  ~DataFileLocationInfo() {}
  DataFileLocationInfo(const std::string& fileName, __int64 startPos,
                       __int64 count)
      : m_fileName(fileName), m_startPos(startPos), m_count(count) {
    //		COUT << _T( "file: " ) << m_fileName << _T( ", start: " ) <<
    // m_startPos << _T( ", count: " ) << m_count << std::endl;
  }

  virtual const std::string toXML() const {
    return tradery::format( "<dataLocation type=\"file\" path=\"", m_fileName, "\" start=\"", m_startPos, "\" count=\"", m_count, "\"/>");
  }
};

using DataLocationInfoPtr = std::shared_ptr<DataLocationInfo>;

CORE_API DataLocationInfoPtr makeDataFileLocationInfo(
    const std::string& fileName, __int64 startPos, __int64 count);

/**
 * \brief Abstract base class for an arbitrary collection of data elements, such
 * as bars or ticks.
 *
 * The common elements of all data collections so far are:
 * - symbol
 * - size
 *
 * @see Bars
 * @see TicksI
 */
class DataCollection {
 private:
  const std::string _symbol;
  DataLocationInfoPtr _locationInfo;

 public:
  /**
   * Constructor - takes a symbol as argument
   *
   * @param symbol
   */
  DataCollection(const std::string& symbol) : _symbol(symbol) {}

  virtual ~DataCollection() {}

  virtual bool hasInvalidData() const = 0;
  virtual std::string getInvalidDataAsString() const = 0;

  /**
   * Returns the number of individual items of data (be they bars, ticks etc) in
   * the collection
   *
   * @return Number of bars
   */
  virtual size_t size() const = 0;

  /**
   * Returns the symbol associated with this collection of data
   *
   * @return
   */
  const std::string& getSymbol() const { return _symbol; }

  void setDataLocationInfo(DataLocationInfoPtr locationInfo) {
    _locationInfo = locationInfo;
  }

  const std::string locationInfoToXML() const {
    return _locationInfo ? _locationInfo->toXML() : std::string();
  }
};

using DataManagedPtr = std::shared_ptr<const DataCollection>;

/**
 * SymbolInfo class - contains information associated with a symbol:
 *
 * - the symbol
 *
 * @see DataSource
 */
class Symbol {
 public:
 private:
  const std::string _symbol;

 public:
  /**
   * Constructor - takes the symbol
   *
   * @param symbol Symbol string
   * @see DataSource
   */
  Symbol(const std::string& symbol) : _symbol(symbol) {}

  virtual ~Symbol() {}

  /**
   * Returns the symbol
   *
   * @return symbol
   */
  const std::string& symbol() const { return _symbol; }

  operator std::string() const { return _symbol; }
};

using SymbolConstPtr = std::shared_ptr<const Symbol>;

class DataInfoException {};

class DataSource;

class DataInfo {
 private:
  const DataSource* _dataSource;
  const SymbolConstPtr _symbol;

 public:
  DataInfo(DataSource* dataSource, SymbolConstPtr symbol)
      : _dataSource(dataSource), _symbol(symbol) {
    if (_symbol.get() == 0) {
      throw DataInfoException();
    }
  }

  virtual ~DataInfo() {}

  const DataSource* dataSource() const { return _dataSource; }

  const Symbol& symbol() const {
    assert(_symbol.get() != 0);
    return *_symbol;
  }

  /*
  const std::string id() const
  {
  std::ostringstream o;
  o << "Data info id: " << _dataSourceId.toString() << _symbol->symbol();
  return o.str();
  }
  */
};

using DataInfoConstPtr = std::shared_ptr<const DataInfo>;

// used to request a set of data elements from a data source
// it handles cashing etc
class DataRequester {
 public:
  virtual ~DataRequester() {}

  /**
   * Returns a pointer to a collection of data elements, given a SymbolInfo
   * descriptor object and a range.
   *
   * The returned pointer is still owned by and its lifecycle controlled by the
   * DataManagerI object and should not be deleted by the application.
   *
   * @param symbol pointer to a SymbolInfo object - describes the data to be
   * loaded
   * @param range  The range, either time or bar index to be used
   * @return pointer to a barsI object - should not be deleted by the
   * application
   */
  virtual DataManagedPtr getData(const DataInfo* dataInfo, DateTimeRangePtr range) = 0;
};

/**
 * A template abstract class derived from DataCollection, which defines data
 * specific method such as add and forEach
 *
 * Concrete data collection classes will be defined by instantiating the
 * template with specific data types, such as bars and ticks (T) as well as the
 * type of the bar handler (U) which is to receive notifications on each bar
 * when the forEach method is called.
 *
 * The bar handler type U is required to have a method
 *
 * @see DataCollection
 * @see DataUnit
 * @see DataHandler
 */
template <class DataType, class DataHandlerType>
class DataCollectionBase : public DataCollection, public Addable<DataType> {
 public:
  /**
   * Constructor - takes the sybmol of this data collection
   *
   * @param symbol The symbol
   */
  DataCollectionBase(const std::string& symbol) : DataCollection(symbol) {}

  virtual ~DataCollectionBase() {}
  /**
   * Pure virtual method for iterating over all the elements of a data
   * collection.
   *
   * It is used to simplify the creation of bar loops in trading systems.
   *
   * Example:
   *
   * class MyBarHandler : public BarHandler
   * {
   *   virtual void onBar( const Bars& bars, size_t bar )
   *   {
   *     // do something here for bar with index "bar", get the open price for
   * example double open = bars.getOpen( bar );
   *   }
   * };
   *
   * //...
   *
   * // create an instance of MyBarHandler
   * MyBarHandler myBarHandler;
   * // for each bar starting at index 10, call the MyBarHandler::onBar
   * forEachBar( myBarHandler, 10 );
   *
   * @param T
   * @param U
   * @param u
   * @param u
   * @param startIndex
   * @param startIndex
   * @exception BarIndexOutOfRangeException
   *                   thrown if the starBar parameter is outside the bounds of
   * the bars collection
   * @exception DataIndexOutOfRangeException
   * @see BarHandler
   */
  virtual void forEach(DataHandlerType& u, size_t startIndex = 0) const = 0;

  /**
   * Returns a data element at index
   *
   * Returns an object on the stack rather than a reference as internally the
   * datacollection may not store data elements as DataType instances. Instead
   * it may have to assemble them on the fly, as the class Bars does.
   *
   * @param index  The index of the data element to be returned
   *
   * @return A copy of the data element
   * @exception DataOutOfRangeException
   *                   Thrown if the index is out of the range of indexes for
   * this collection
   */
  virtual const DataType get(size_t index) const = 0;

  /*  virtual void forEach( U& u, size_t startIndex = 0 ) const throw(
  DataIndexOutOfRangeException )
  {
  if ( startIndex >= size() )
  throw DataIndexOutOfRangeException( size(), startIndex, getSymbol() );

  for ( size_t index = startIndex; index < size(); index++ )
  u.dataHandler( *this, index );
  }
  */
};

/**
 * Abstract base class for data handlers
 *
 * Data handlers implement a callback mechanism for data loops. A data loop is
 * an iteration through all or some of the available data.
 *
 * DataHandler instances' dataHandler methods are called by forEach on each data
 * unit in the collection
 *
 * The purpose of this mechanism is to simplify the creation of loops on bars in
 * a trading system - the user code will not have to worry about writing the
 * loop code, only deciding how to handle each of the data units.
 *
 * @see DataContainer
 */
template <class T>
class DataHandler {
 public:
  virtual ~DataHandler() {}
  /**
   * Pure virtual method - The data handler callback method
   *
   * This method is called on each data unit
   *
   * <!--//TODO; add more parameters that could be useful in the user code -->
   *
   * @param T
   * @param container A reference to the container that called this method
   * @param container
   * @param index     The index of the data unit in the container for which this
   * method is called
   * @param index
   * @see Bars::forEachBar
   */
  virtual void dataHandler(const T& container, size_t index) = 0;
};

class BarHandler : public DataHandler<BarsAbstr> {
 public:
  virtual void onBar(const BarsAbstr& bars, size_t index) = 0;
  virtual void dataHandler(const BarsAbstr& bars, size_t index) {
    onBar(bars, index);
  }
};
using TickHandler = DataHandler<Ticks>;
using BarsBase = DataCollectionBase<Bar, BarHandler>;
using TicksBase = DataCollectionBase<Tick, TickHandler>;

/**
 * Abstract class - base class for a collection of ticks.
 *
 * @see Data
 */
class Ticks : public TicksBase {
 public:
  Ticks(const std::string& symbol) : TicksBase(symbol) {}

  virtual ~Ticks() {}
};

class BarIndicators {
 public:
  virtual ~BarIndicators() {}

  virtual const Series TrueRange() const = 0;
  virtual const Series AccumDist() const = 0;
  virtual const Series ADX(unsigned int period) const = 0;
  virtual const Series ADXR(unsigned int period) const = 0;
  virtual const Series MinusDI(unsigned int period) const = 0;
  virtual const Series MinusDM(unsigned int period) const = 0;
  virtual const Series AvgPrice() const = 0;
  virtual const Series MedPrice() const = 0;
  virtual const Series TypPrice() const = 0;
  virtual const Series WclPrice() const = 0;
  virtual const Series CCI(unsigned int period) const = 0;
  virtual const Series NATR(unsigned int period) const = 0;
  virtual const Series DX(unsigned int period) const = 0;
  virtual const Series ChaikinAD() const = 0;
  virtual const Series ChaikinADOscillator(unsigned int fastPeriod, unsigned int slowPeriod) const = 0;
  virtual const Series OBV(const Series& series) const = 0;
  virtual const Series MidPrice(unsigned int period) const = 0;
  virtual const Series SAR(double acceleration, double maximum) const = 0;
  virtual const Series MFI(unsigned int period) const = 0;
  virtual const Series ATR(unsigned int period) const = 0;
  virtual const Series TR() const = 0;
  virtual const Series PlusDI(unsigned int period) const = 0;
  virtual const Series PlusDM(unsigned int period) const = 0;
  virtual const Series WillR(unsigned int period) const = 0;
  virtual const Series StochSlowK(int fastKPeriod, int slowKPeriod, MAType slowKMAType, int slowDPeriod, MAType slowDMAType) const = 0;
  virtual const Series StochSlowD(int fastKPeriod, int slowKPeriod, MAType slowKMAType, int slowDPeriod, MAType slowDMAType) const = 0;
  virtual const Series StochFastK(int fastKPeriod, int fastDPeriod, MAType fastDMAType) const = 0;
  virtual const Series StochFastD(int fastKPeriod, int fastDPeriod, MAType fastDMAType) const = 0;
  virtual const Series BOP() const = 0;

  /**\name Candle patterns
   * @{
   */
  virtual const Series Cdl3BlackCrows() const = 0;
  virtual const Series CdlAbandonedBaby(double penetration) const = 0;
  virtual const Series CdlDojiStar() const = 0;
  virtual const Series CdlEngulfing() const = 0;
  virtual const Series CdlEveningDojiStar(double penetration) const = 0;
  virtual const Series CdlEveningStar(double penetration) const = 0;
  virtual const Series CdlHammer() const = 0;
  virtual const Series CdlHangingMan() const = 0;
  virtual const Series CdlHarami() const = 0;
  virtual const Series CdlHaramiCross() const = 0;
  virtual const Series CdlHighWave() const = 0;
  virtual const Series CdlIdentical3Crows() const = 0;
  virtual const Series CdlInvertedHammer() const = 0;
  virtual const Series CdlLongLine() const = 0;
  virtual const Series CdlMorningDojiStar(double penetration) const = 0;
  virtual const Series CdlMorningStar(double penetration) const = 0;
  virtual const Series CdlShootingStar() const = 0;
  virtual const Series CdlShortLine() const = 0;
  virtual const Series CdlSpinningTop() const = 0;
  virtual const Series CdlTristar() const = 0;
  virtual const Series CdlUpsideGap2Crows() const = 0;
  virtual const Series CdlHikkake() const = 0;

  // ta-lib 0.1.3
  virtual const Series Cdl2Crows() const = 0;
  virtual const Series Cdl3Inside() const = 0;
  virtual const Series Cdl3LineStrike() const = 0;
  virtual const Series Cdl3WhiteSoldiers() const = 0;
  virtual const Series Cdl3Outside() const = 0;
  virtual const Series CdlInNeck() const = 0;
  virtual const Series CdlOnNeck() const = 0;
  virtual const Series CdlPiercing() const = 0;
  virtual const Series CdlStalled() const = 0;
  virtual const Series CdlThrusting() const = 0;
  virtual const Series CdlAdvanceBlock(double penetration) const = 0;

  // ta-lib 0.1.4
  virtual const Series Cdl3StarsInSouth() const = 0;
  virtual const Series CdlBeltHold() const = 0;
  virtual const Series CdlBreakaway() const = 0;
  virtual const Series CdlClosingMarubuzu() const = 0;
  virtual const Series CdlConcealingBabySwallow() const = 0;
  virtual const Series CdlCounterattack() const = 0;
  virtual const Series CdlDoji() const = 0;
  virtual const Series CdlDragonFlyDoji() const = 0;
  virtual const Series CdlGapSideBySideWhite() const = 0;
  virtual const Series CdlGravestoneDoji() const = 0;
  virtual const Series CdlHomingPigeon() const = 0;
  virtual const Series CdlKicking() const = 0;
  virtual const Series CdlKickingByLength() const = 0;
  virtual const Series CdlLadderBottom() const = 0;
  virtual const Series CdlLongLeggedDoji() const = 0;
  virtual const Series CdlMarubozu() const = 0;
  virtual const Series CdlMatchingLow() const = 0;
  virtual const Series CdlRickshawMan() const = 0;
  virtual const Series CdlRiseFall3Methods() const = 0;
  virtual const Series CdlSeparatingLines() const = 0;
  virtual const Series CdlStickSandwich() const = 0;
  virtual const Series CdlTakuri() const = 0;
  virtual const Series CdlTasukiGap() const = 0;
  virtual const Series CdlUnique3River() const = 0;
  virtual const Series CdlXSideGap3Methods() const = 0;
  virtual const Series CdlMatHold(double penetration) const = 0;
};

enum ErrorHandlingMode { fatal, warning, ignore };

constexpr auto ERROR_HANDLING_MODE_FATAL = "fatal";
constexpr auto ERROR_HANDLING_MODE_WARNING = "warning";
constexpr auto ERROR_HANDLING_MODE_IGNORE = "ignore";

inline std::string errorHandlingModeAsString(
    ErrorHandlingMode errorHandlingMode) {
  switch (errorHandlingMode) {
    case fatal:
      return ERROR_HANDLING_MODE_FATAL;
    case warning:
      return ERROR_HANDLING_MODE_WARNING;
    case ignore:
      return ERROR_HANDLING_MODE_IGNORE;
    default:
      assert(false);
      return ERROR_HANDLING_MODE_FATAL;
  }
}

/**
 * Abstract class - base class to a collection of bars associated with a symbol
 *
 * Objects of this class are always created by data sources objects
 *
 * <!-- \htmlonly <FONT COLOR="RED">TODO: add a dump method </FONT> \endhtmlonly
 * -->
 * <!--\htmlonly <FONT COLOR="RED">TODO: add a method that returns times as a
 * series?</FONT> \endhtmlonly -->
 *
 * Once created, objects of this class are added to an internal cache which
 * optimizes their use.
 *
 * This class has access methods, utility methods (forEachBar) as well as a
 * variety of technical indicators that operate on a collection of bars
 *
 * @see DataSource
 * @see BarsAddableI
 * @see DataCollection
 * @see BarHandler
 */
class BarsAbstr : public BarIndicators {
 public:
  virtual ~BarsAbstr() {}

  virtual size_t size() const = 0;
  virtual void forEach(tradery::BarHandler& barHandler, size_t startBar = 0) const = 0;
  virtual size_t unsyncSize() const = 0;
  virtual std::pair<DateTime, DateTime> unsyncStartEnd() const = 0;
  virtual bool isSynchronized() const = 0;
  virtual bool syncModified() const = 0;

  virtual ErrorHandlingMode getErrorHandlingMode() const = 0;

  /**
   * Returns the open value at a specified bar
   *
   * @param barIndex The bar index for which the open value is to be returned
   * @return open value at barIndex
   * @exception BarIndexOutOfRangeException
   *                   thrown if barIndex is outside the bounds of the
   * collection
   */
  virtual double open(size_t barIndex) const = 0;
  /**
   * Returns the low value at a specified bar
   *
   * @param barIndex The bar index for which the low value is to be returned
   * @return open value at barIndex
   * @exception BarIndexOutOfRangeException
   *                   thrown if barIndex is outside the bounds of the
   * collection
   */
  virtual double low(size_t barIndex) const = 0;
  /**
   * Returns the high value at a specified bar
   *
   * @param barIndex The bar index for which the high value is to be returned
   * @return low value at barIndex
   * @exception BarIndexOutOfRangeException
   *                   thrown if barIndex is outside the bounds of the
   * collection
   */
  virtual double high(size_t barIndex) const = 0;
  /**
   * Returns the close value at a specified bar
   *
   * @param barIndex The bar index for which the close value is to be returned
   * @return high value at barIndex
   * @exception BarIndexOutOfRangeException
   *                   thrown if barIndex is outside the bounds of the
   * collection
   */
  virtual double close(size_t barIndex) const = 0;
  /**
   * Returns the volume value at a specified bar
   *
   * @param barIndex The bar index for which the volume value is to be returned
   * @return open volume at barIndex
   * @exception BarIndexOutOfRangeException
   *                   thrown if barIndex is outside the bounds of the
   * collection
   */
  virtual unsigned long volume(size_t barIndex) const = 0;
  /**
   * Returns the open interest value at a specified bar
   *
   * @param barIndex The bar index for which the open interest value is to be
   * returned
   * @return open volume at barIndex
   * @exception BarIndexOutOfRangeException
   *                   thrown if barIndex is outside the bounds of the
   * collection
   */
  virtual unsigned long openInterest(size_t barIndex) const = 0;
  /**
   * Returns the time stamp at a specified bar
   *
   * @param barIndex The bar index for which the open value is to be returned
   * @return time stamp at barIndex
   * @exception BarIndexOutOfRangeException
   *                   thrown if barIndex is outside the bounds of the
   * collection
   */
  virtual DateTime time(size_t barIndex) const = 0;
  virtual Date date(size_t barIndex) const = 0;
  /**
   * Returns a pointer to the BarExtraInfo object associated with the bar at
   * barIndex.
   *
   * @param barIndex Index of the bar for which the BarExtraInfo is requested
   * @return pointer to BarExtraInfo object
   * @exception BarIndexOutOfRangeException
   *                   thrown if the bar index is outside the bounds
   * @see BarExtraInfo
   */
  virtual const BarExtraInfoPtr getBarExtraInfo(size_t barIndex) const = 0;

  virtual const Bar getBar(size_t index) const = 0;
  /**
   * The possible types of bars collections: so far stocks and futures
   */
  enum Type { stock, future };

  virtual const std::string& getSymbol() const = 0;

  /**
   * Returns the type of the bars collection, stocks or futures
   *
   * @return one of the Type elements: stocks or futures
   */
  virtual Type type() const = 0;
  /**
   * The resolution of the bar collection, i.e. the duration of a bar
   *
   * @return duration of a bar in seconds
   */
  virtual unsigned long resolution() const = 0;
  /**
   * Returns all the open values as a Series object
   *
   * @return A pointer to a Series object containing all the open values
   * @see Series
   */
  virtual const Series openSeries() const = 0;
  /**
   * Returns all the low values as a Series object
   *
   * @return A pointer to a Series object containing all the low values
   * @see Series
   */
  virtual const Series lowSeries() const = 0;
  /**
   * Returns all the high values as a Series object
   *
   * @return A pointer to a Series object containing all the high values
   * @see Series
   */
  virtual const Series highSeries() const = 0;
  /**
   * Returns all the close values as a Series object
   *
   * @return A pointer to a Series object containing all the close values
   * @see Series
   */
  virtual const Series closeSeries() const = 0;
  /**
   * Returns all the volume values as a Series object
   *
   * @return A pointer to a Series object containing all the volume values
   * @see Series
   */
  virtual const Series volumeSeries() const = 0;

  /**
   * Returns all the open interest values as a pointer to a Series object
   *
   * @return A pointer to a Series object containing all the open interest
   * values
   * @see Series
   */
  virtual const Series openInterestSeries() const = 0;

  /**
   * Returns all the time values as a pointer to a TimeSeries object
   *
   * @return const reference to the TimeSeries object associated with the Bars
   * @see XTime
   */
  virtual TimeSeries timeSeries() const = 0;
  /**
   * Returns the series of BarExtraInfo
   *
   * @return pointer to a ExtraInfoSeries
   * @see ExtraInfoSeries
   */
  virtual const ExtraInfoSeries& getExtraInfoSeries() const = 0;

  /** \defgroup Indicators Technical Indicators
   */
  //@{
  /** @defgroup BarsIndicators Technical indicators on Collections of Bars
   * Technical indicators that apply to collections of bars
   */
  //@{

  virtual void synchronize(Bars bars) = 0;
  virtual const std::string locationInfoToXML() const = 0;

  //@}
  //@}
  //@}
};

class Bars : private BarsAbstr {
 private:
  const BarsAbstr* _bars;
  std::string _symbol;

  bool validate() const {
    if (!_bars) {
      throw InvalidBarsCollectionException(_symbol);
    }
    else {
      return true;
    }
  }

 public:
  double open(size_t barIndex) const override {
    validate();
    return _bars->open(barIndex);
  }
  double low(size_t barIndex) const override {
    validate();
    return _bars->low(barIndex);
  }
  double high(size_t barIndex) const override {
    validate();
    return _bars->high(barIndex);
  }
  double close(size_t barIndex) const override {
    validate();
    return _bars->close(barIndex);
  }
  unsigned long volume(size_t barIndex) const override {
    validate();
    return _bars->volume(barIndex);
  }
  unsigned long openInterest(size_t barIndex) const override {
    validate();
    return _bars->openInterest(barIndex);
  }
  Date date(size_t barIndex) const override {
    validate();
    return _bars->date(barIndex);
  }
  DateTime time(size_t barIndex) const override {
    validate();
    return _bars->time(barIndex);
  }
  const BarExtraInfoPtr getBarExtraInfo(size_t barIndex) const override {
    validate();
    return _bars->getBarExtraInfo(barIndex);
  }
  bool isSynchronized() const override {
    validate();
    return _bars->isSynchronized();
  }
  bool syncModified() const override {
    validate();
    return _bars->syncModified();
  }

  operator bool() const { return _bars != 0; }

  Bars() : _bars(0) {}

  Bars(const std::string& symbol) : _symbol(symbol), _bars(0) {}

  Bars(const BarsAbstr* bars) : _bars(bars) {}

  ErrorHandlingMode getErrorHandlingMode() const override {
    validate();
    return _bars->getErrorHandlingMode();
  }

  Index size() const override {
    validate();
    return _bars->size();
  }
  Index unsyncSize() const override {
    validate();
    return _bars->unsyncSize();
  }
  std::pair<DateTime, DateTime> unsyncStartEnd() const override {
    validate();
    return _bars->unsyncStartEnd();
  }

  const std::string& getSymbol() const override {
    validate();
    return _bars->getSymbol();
  }
  const Bar getBar(Index index) const override {
    validate();
    return _bars->getBar(index);
  }
  void forEach(tradery::BarHandler& barHandler, size_t startBar = 0) const override {
    validate();
    _bars->forEach(barHandler, startBar);
  }

  Type type() const override {
    validate();
    return _bars->type();
  }
  unsigned long resolution() const override {
    validate();
    return _bars->resolution();
  }
  const Series openSeries() const override {
    validate();
    return _bars->openSeries();
  }
  const Series lowSeries() const override {
    validate();
    return _bars->lowSeries();
  }
  const Series highSeries() const override {
    validate();
    return _bars->highSeries();
  }
  const Series closeSeries() const override {
    validate();
    return _bars->closeSeries();
  }
  const Series volumeSeries() const override {
    validate();
    return _bars->volumeSeries();
  }
  const Series openInterestSeries() const override {
    validate();
    return _bars->openInterestSeries();
  }
  TimeSeries timeSeries() const override {
    validate();
    return _bars->timeSeries();
  }
  const ExtraInfoSeries& getExtraInfoSeries() const override {
    validate();
    return _bars->getExtraInfoSeries();
  }

  const Series TrueRange() const override {
    validate();
    return _bars->TrueRange();
  }
  const Series AccumDist() const override {
    validate();
    return _bars->AccumDist();
  }
  const Series ADX(unsigned int period) const override {
    validate();
    return _bars->ADX(period);
  }
  const Series ADXR(unsigned int period) const override {
    validate();
    return _bars->ADXR(period);
  }
  const Series MinusDI(unsigned int period) const override {
    validate();
    return _bars->MinusDI(period);
  }
  const Series MinusDM(unsigned int period) const override {
    validate();
    return _bars->MinusDM(period);
  }
  const Series AvgPrice() const override {
    validate();
    return _bars->AvgPrice();
  }
  const Series MedPrice() const override {
    validate();
    return _bars->MedPrice();
  }
  const Series TypPrice() const override {
    validate();
    return _bars->TypPrice();
  }
  const Series WclPrice() const override {
    validate();
    return _bars->WclPrice();
  }
  const Series CCI(unsigned int period) const override {
    validate();
    return _bars->CCI(period);
  }
  const Series DX(unsigned int period) const override {
    validate();
    return _bars->DX(period);
  }
  const Series NATR(unsigned int period) const override {
    validate();
    return _bars->NATR(period);
  }
  const Series ChaikinAD() const override {
    validate();
    return _bars->ChaikinAD();
  }
  const Series ChaikinADOscillator(unsigned int fastPeriod, unsigned int slowPeriod) const override {
    validate();
    return _bars->ChaikinADOscillator(fastPeriod, slowPeriod);
  }
  const Series OBV(const Series& series) const override {
    validate();
    return _bars->OBV(series);
  }
  const Series MidPrice(unsigned int period) const override {
    validate();
    return _bars->MidPrice(period);
  }
  const Series SAR(double acceleration, double maximum) const override {
    validate();
    return _bars->SAR(acceleration, maximum);
  }
  const Series MFI(unsigned int period) const override {
    validate();
    return _bars->MFI(period);
  }
  const Series ATR(unsigned int period) const override {
    validate();
    return _bars->ATR(period);
  }
  const Series TR() const override {
    validate();
    return _bars->TR();
  }
  const Series PlusDI(unsigned int period) const override {
    validate();
    return _bars->PlusDI(period);
  }
  const Series PlusDM(unsigned int period) const override {
    validate();
    return _bars->PlusDM(period);
  }
  const Series WillR(unsigned int period) const override {
    validate();
    return _bars->WillR(period);
  }
  const Series StochSlowK(int fastKPeriod, int slowKPeriod, MAType slowKMAType, int slowDPeriod, MAType slowDMAType) const override {
    validate();
    return _bars->StochSlowK(fastKPeriod, slowKPeriod, slowKMAType, slowDPeriod,slowDMAType);
  }
  const Series StochSlowD(int fastKPeriod, int slowKPeriod, MAType slowKMAType, int slowDPeriod, MAType slowDMAType) const override {
    validate();
    return _bars->StochSlowD(fastKPeriod, slowKPeriod, slowKMAType, slowDPeriod, slowDMAType);
  }
  const Series StochFastK(int fastKPeriod, int fastDPeriod, MAType fastDMAType) const override {
    validate();
    return _bars->StochFastK(fastKPeriod, fastDPeriod, fastDMAType);
  }
  const Series StochFastD(int fastKPeriod, int fastDPeriod, MAType fastDMAType) const override {
    validate();
    return _bars->StochFastD(fastKPeriod, fastDPeriod, fastDMAType);
  }
  const Series BOP() const override {
    validate();
    return _bars->BOP();
  }

  /**\name Candle patterns
   * @{
   */
  const Series Cdl3BlackCrows() const override {
    validate();
    return _bars->Cdl3BlackCrows();
  }
  const Series CdlAbandonedBaby(double penetration) const override {
    validate();
    return _bars->CdlAbandonedBaby(penetration);
  }
  const Series CdlDojiStar() const override {
    validate();
    return _bars->CdlDojiStar();
  }
  const Series CdlEngulfing() const override {
    validate();
    return _bars->CdlEngulfing();
  }
  const Series CdlEveningDojiStar(double penetration) const override {
    validate();
    return _bars->CdlEveningDojiStar(penetration);
  }
  const Series CdlEveningStar(double penetration) const override {
    validate();
    return _bars->CdlEveningStar(penetration);
  }
  const Series CdlHammer() const override {
    validate();
    return _bars->CdlHammer();
  }
  const Series CdlHangingMan() const override {
    validate();
    return _bars->CdlHangingMan();
  }
  const Series CdlHarami() const override {
    validate();
    return _bars->CdlHarami();
  }
  const Series CdlHaramiCross() const override {
    validate();
    return _bars->CdlHaramiCross();
  }
  const Series CdlHighWave() const override {
    validate();
    return _bars->CdlHighWave();
  }
  const Series CdlIdentical3Crows() const override {
    validate();
    return _bars->CdlIdentical3Crows();
  }
  const Series CdlInvertedHammer() const override {
    validate();
    return _bars->CdlInvertedHammer();
  }
  const Series CdlLongLine() const override {
    validate();
    return _bars->CdlLongLine();
  }
  const Series CdlMorningDojiStar(double penetration) const override {
    validate();
    return _bars->CdlMorningDojiStar(penetration);
  }
  const Series CdlMorningStar(double penetration) const override {
    validate();
    return _bars->CdlMorningStar(penetration);
  }
  const Series CdlShootingStar() const override {
    validate();
    return _bars->CdlShootingStar();
  }
  const Series CdlShortLine() const override {
    validate();
    return _bars->CdlShortLine();
  }
  const Series CdlSpinningTop() const override {
    validate();
    return _bars->CdlSpinningTop();
  }
  const Series CdlTristar() const override {
    validate();
    return _bars->CdlTristar();
  }
  const Series CdlUpsideGap2Crows() const override {
    validate();
    return _bars->CdlUpsideGap2Crows();
  }
  const Series CdlHikkake() const override {
    validate();
    return _bars->CdlHikkake();
  }

  // ta-lib 0.1.3
  const Series Cdl2Crows() const override {
    validate();
    return _bars->Cdl2Crows();
  }
  const Series Cdl3Inside() const override {
    validate();
    return _bars->Cdl3Inside();
  }
  const Series Cdl3LineStrike() const override {
    validate();
    return _bars->Cdl3LineStrike();
  }
  const Series Cdl3WhiteSoldiers() const override {
    validate();
    return _bars->Cdl3WhiteSoldiers();
  }
  const Series Cdl3Outside() const override {
    validate();
    return _bars->Cdl3Outside();
  }
  const Series CdlInNeck() const override {
    validate();
    return _bars->CdlInNeck();
  }
  const Series CdlOnNeck() const override {
    validate();
    return _bars->CdlOnNeck();
  }
  const Series CdlPiercing() const override {
    validate();
    return _bars->CdlPiercing();
  }
  const Series CdlStalled() const override {
    validate();
    return _bars->CdlStalled();
  }
  const Series CdlThrusting() const override {
    validate();
    return _bars->CdlThrusting();
  }
  const Series CdlAdvanceBlock(double penetration) const override {
    validate();
    return _bars->CdlAdvanceBlock(penetration);
  }

  // ta-lib 0.1.4
  const Series Cdl3StarsInSouth() const override {
    validate();
    return _bars->Cdl3StarsInSouth();
  }
  const Series CdlBeltHold() const override {
    validate();
    return _bars->CdlBeltHold();
  }
  const Series CdlBreakaway() const override {
    validate();
    return _bars->CdlBreakaway();
  }
  const Series CdlClosingMarubuzu() const override {
    validate();
    return _bars->CdlClosingMarubuzu();
  }
  const Series CdlConcealingBabySwallow() const override {
    validate();
    return _bars->CdlConcealingBabySwallow();
  }
  const Series CdlCounterattack() const override {
    validate();
    return _bars->CdlCounterattack();
  }
  const Series CdlDoji() const override {
    validate();
    return _bars->CdlDoji();
  }
  const Series CdlDragonFlyDoji() const override {
    validate();
    return _bars->CdlDragonFlyDoji();
  }
  const Series CdlGapSideBySideWhite() const override {
    validate();
    return _bars->CdlGapSideBySideWhite();
  }
  const Series CdlGravestoneDoji() const override {
    validate();
    return _bars->CdlGravestoneDoji();
  }
  const Series CdlHomingPigeon() const override {
    validate();
    return _bars->CdlHomingPigeon();
  }
  const Series CdlKicking() const override {
    validate();
    return _bars->CdlKicking();
  }
  const Series CdlKickingByLength() const override {
    validate();
    return _bars->CdlKickingByLength();
  }
  const Series CdlLadderBottom() const override {
    validate();
    return _bars->CdlLadderBottom();
  }
  const Series CdlLongLeggedDoji() const override {
    validate();
    return _bars->CdlLongLeggedDoji();
  }
  const Series CdlMarubozu() const override {
    validate();
    return _bars->CdlMarubozu();
  }
  const Series CdlMatchingLow() const override {
    validate();
    return _bars->CdlMatchingLow();
  }
  const Series CdlRickshawMan() const override {
    validate();
    return _bars->CdlRickshawMan();
  }
  const Series CdlRiseFall3Methods() const override {
    validate();
    return _bars->CdlRiseFall3Methods();
  }
  const Series CdlSeparatingLines() const override {
    validate();
    return _bars->CdlSeparatingLines();
  }
  const Series CdlStickSandwich() const override {
    validate();
    return _bars->CdlStickSandwich();
  }
  const Series CdlTakuri() const override {
    validate();
    return _bars->CdlTakuri();
  }
  const Series CdlTasukiGap() const override {
    validate();
    return _bars->CdlTasukiGap();
  }
  const Series CdlUnique3River() const override {
    validate();
    return _bars->CdlUnique3River();
  }
  const Series CdlXSideGap3Methods() const override {
    validate();
    return _bars->CdlXSideGap3Methods();
  }

  const Series CdlMatHold(double penetration) const override {
    validate();
    return _bars->CdlMatHold(penetration);
  }

  void synchronize(Bars bars) override {
    validate();
    (const_cast<BarsAbstr*>(_bars))->synchronize(bars);
  }
  const std::string locationInfoToXML() const override {
    validate();
    return _bars->locationInfoToXML();
  }
};

using BarsPtr = std::shared_ptr<BarsBase>;
using TicksPtr = std::shared_ptr<Ticks>;

/**
 * Data source event base class
 *
 * A real time data source will trigger events when new data is available for
 * example.
 *
 * @see DataSource
 * @see DataSourceListenerI
 */
class DataSourceEvent {
 public:
  enum Type { newDataAvailable, noMoreDataAvailable };

 private:
  const Type _type;
  const tradery::DateTime _timeStamp;

 private:
  // don't allow =
  void operator=(const DataSourceEvent& event) {}

 public:
  DataSourceEvent(Type type) : _type(type), _timeStamp(LocalTimeSec()) {}

  DataSourceEvent(const DataSourceEvent& event)
      : _type(event._type), _timeStamp(event.timeStamp()) {}

  Type type() const { return _type; }
  DateTime timeStamp() const { return _timeStamp; }
};

class DataInfo;

}  // namespace tradery
