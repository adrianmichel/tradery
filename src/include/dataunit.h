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
/** @endcond */

/**
 * \brief The base class for all specific elements of data, such as Bar, Tick
 * and others
 *
 * Represents a generic unit of data, which has a time stamp. Specific units of
 * data such as bars and ticks build on this and add various other fields that
 * define them
 */
class DataUnit {
 private:
  const DateTime _time;

 public:
  virtual ~DataUnit() {}

  /**
   * Constructor - takes a time stamp as argument
   *
   * @param time   The timestamp of the data unit
   */
  DataUnit(const DateTime& time) : _time(time) {}

  /**
   * Returns the time stamp of the data unit
   *
   * @return time stamp
   */
  const DateTime& time() const { return _time; }

  const Date date() const { return _time.date(); }
};
}  // namespace tradery