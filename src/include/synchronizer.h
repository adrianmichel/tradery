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

#include "datasource.h"

namespace tradery {
class Bars;
class TimeSeries;
/* @cond */
class CORE_API Synchronizer {
public:
  using SynchronizerPtr = std::shared_ptr<Synchronizer>;

  static SynchronizerPtr create(Bars ref, Bars syncd);
  virtual ~Synchronizer() {}
  virtual int index(size_t ix) const = 0;
  virtual size_t size() const = 0;
  virtual bool modified() const = 0;
  virtual bool operator==(const Synchronizer& synchronizer) const = 0;
  virtual bool operator==(const Synchronizer* synchronizer) const = 0;
  bool operator!=(const Synchronizer& synchronizer) const {
    return !(*this == synchronizer);
  }
  bool operator!=(const Synchronizer* synchronizer) const {
    return !(*this == synchronizer);
  }
  virtual const std::string& refSymbol() const = 0;
  virtual TimeSeries timeSeries() const = 0;
};
/* @endcond */

}  // namespace tradery