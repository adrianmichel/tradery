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

/*
IMPORTANT DISCLAIMER

These sample trading systems are included for illustrative purpose only.

Using them with real money can lead to substantial financial losses.

DO NOT USE FOR REAL MONEY TRADING OR INVESTMENT DECISIONS.
*/

#pragma once
#pragma warning(disable : 4482)

#include <macros.h>

/**
 * This sample system does "something" within the open positions handler
 */
class System2 : public BarSystem<System2> {
 private:
  // do something on each open position - close all open positions at limit $25
  virtual bool onOpenPosition(tradery::Position pos, Index bar) {
    sellAtLimit(bar + 1, pos, 25, "sell at limit");

    return true;
  }

 public:
  System2(const std::vector<std::string>* params = 0)
      : BarSystem<System2>(Info("1AB6A6A9-A13F-480e-9DAD-2F4D71D140FE",
                                "System 2 - example of open positions handler",
                                "This is an example of a system class that "
                                "does \"something\" in the open positions "
                                "handler")) {}

  virtual bool init(const std::string& symbol) {
    PrintLine(green << name() << tradery::Control::reset << italic << " in init"
                    << tradery::endl);

    return true;
  }

  virtual void cleanup() {
    PrintLine(blue << name() << tradery::Control::reset << bold << " in cleanup"
                   << endl);
  }

  virtual void onBar(Index bar) {
    // this will call onOpenPosition on each open position
    forEachOpenPosition(bar);
    // entry at limit
    buyAtLimit(bar + 1, low(bar) * 1.05, 1000, "buy at limit");
  }

  virtual void run() {
    // apply a loop for each bar
    forEachBar(0);
  }
};
