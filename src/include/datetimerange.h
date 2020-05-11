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

#include "datetime.h"
namespace tradery {
/**
 * \brief Base class for all types of ranges
 *
 * Abstract class which provides an interface that needs to be implemented by
 * all types of ranges, be them time or index (or other type) based.
 *
 * Typically a range implements two boundaries, upper and lower, and they can
 * even implement unbounded ranges (upper, lower or both). The interface doesn't
 * specify anything about the implementation though, not even that there are two
 * boundaries, but it defines an ordering relationship between the range and a
 * data unit, which can be checked by calling one of the two abstract methods.
 * It is up to the concrete ranges to actually decide how this ordering is to be
 * implemented.
 *
 *
 * Current implementation include time and index based ranges
 */
class Range {
 public:
  virtual ~Range() {}

  /**
   * Indicates whether the range is lower than the data unit
   *
   * Lower in this context means that the data unit has a time stamp or a
   * position in the collection of data units that is higher than the upper
   * boundary of the range
   *
   * derived class will implement this comparison operator
   *
   * @param dataUnit the data unit to be tested against the range
   * @return true if range is lower than the data unit, false otherwise
   */
  virtual bool operator<(const DataUnit& dataUnit) const = 0;
  /**
   * Indicates whether the range is higher than the data unit
   *
   * Higher in this context means that the data unit has a time stamp or a
   * position in the collection of data units that is lower than the lower
   * boundary of the range
   *
   * derived class will implement this comparison operator
   *
   * @param dataUnit the data unit to be tested against the range
   * @return true if range is higher than the data unit, false otherwise
   */
  virtual bool operator>(const DataUnit& dataUnit) const = 0;
  /**
   * Generates a string that uniquely identifies the range
   *
   * Used internally by the cache functionality, to generate unique ids of
   * different cacheable elements
   *
   * It is up to the concrete derived classes to define the strings
   *
   * @return The id of the range
   */
  virtual std::string getId() const = 0;
  virtual std::string toString() const = 0;
};

/**
 * A smart pointer to a range
 */
using RangePtr = std::shared_ptr<Range>;


class DateTimeRangeException {};

/**
 * A time range
 *
 * A time range can have both upper and lower, or it can be unbounded at one or
 * both ends
 *
 * Either limit is inclusive, meaning that it belongs to the range
 *
 * @see Range
 * @see DateTime
 */
class DateTimeRange : public Range, private DateTimePair {
 public:
  /**
   * Constructor - takes references to two XTime objects as parameters
   *
   * This creates a range bounded at both ends, as references cannot be 0
   *
   * <!-- TODO: check the validity of the range (begin <= end ) -->
   *
   * @param begin  Reference to the lower end of the range
   * @param end    Reference to the upper end of the range
   * @see XTime
   */
  DateTimeRange(const DateTime& begin, const DateTime& end)
      : DateTimePair(begin, end) {
    if (end < begin) {
      throw DateTimeRangeException();
    }
  }

  /**
   * Constructor - takes 2 ISO strings representation of data/time parameters
   *
   * <!-- TODO: check the validity of the range (begin <= end ) -->
   *
   * @param begin  The lower end of the range
   * @param end    The upper end of the range
   * @see XTime
   */
  DateTimeRange(const std::string& begin, const std::string& end)
      : DateTimePair(DateTimeFromIsoString(begin), DateTimeFromIsoString(end)) {
    // TODO: enforce that the strings are valid (in the DateTime class)
    if (first > second) {
      throw DateTimeRangeException();
    }
  }

  DateTimeRange()
      : DateTimePair(NegInfinityDateTime(), PosInfinityDateTime()) {}

  /**
   * Implements the "lower" relationship between a time range and a DataUnit
   *
   *
   * Indicates if the upper bound of the range is lower than the data unit time
   * stamp
   *
   * @param dataUnit Data unit to be compared with the time range
   * @return true if the upper bound is lower than the data unit time stamp
   */
  virtual bool operator<(const DataUnit& dataUnit) const {
    /*    std::wostringstream o;
    o << _T( " --- " ) << xtime_to_simple_string( _p.last() ) << _T( "----" ) <<
    xtime_to_simple_string( bar.getTime() ) << std::endl; tsprint( o, std::cout
    );

    */
    return second < dataUnit.time();
  }

  /**
   * Creates a "higher" relationship between a time range and a DataUnit
   *
   * Indicates if the lower bound of the range is higher than the data unit time
   * stamp
   *
   * @param dataUnit Data unit to be compared with the time range
   * @return true if the lower bound is higher than the data unit time stamp
   */
  virtual bool operator>(const DataUnit& dataUnit) const {
    return first > dataUnit.time();
  }

  /**
   * Generates a unique id of the time range
   *
   * @return The unique id
   */
  virtual std::string getId() const {
    return tradery::format("Time range (begin - last): ", toString() );
  }

  /**
   * Generates a string representation of the time range
   *
   * <!-- TODO - example of the string representation -->
   *
   * @return The string representation
   */
  std::string toString() const {
    return first.to_simple_string() + " - " + second.to_simple_string();
  }

  DateTime from() const { return __super::first; }
  DateTime to() const { return __super::second; }
};

using DateTimeRangePtr = std::shared_ptr<DateTimeRange>;

}  // namespace tradery