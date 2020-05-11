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

#include <macros.h>

/**
 * This sample system does "something" within the open positions handler
 */
class SimpleTradingSystem : public BarSystem<System2> {
 private:
  // do something on each open position - close all open positions at limit $25
  virtual bool onOpenPosition(tradery::Position pos, Index bar) {
    sellAtLimit(bar + 1, pos, 25, "sell at limit");

    return true;
  }

 public:
  SimpleTradingSystem(const std::vector<std::string>* params = 0)
      : BarSystem<System2>(Info("E6D64E9E-3A59-4a99-9C1F-ABD300E9AFE3",
                                "Sample for a simple trading system",
                                "This is an example of a system class that "
                                "does \"something\" in the open positions "
                                "handler")) {}

  virtual void run() {
    // apply a loop for each bar
    for (Index bar = 0; bar < barsCount(); bar++) {
      // positions exit logic
      for (Position pos = getFirstOpenPosition(); pos;
           pos = getNextOpenPosition()) {
        PrintLine("pos entry price" << pos.getEntryPrice() << ", target price: "
                                    << pos.getEntryPrice() * 1.05);
        bool b = sellAtLimit(bar + 1, pos, pos.getEntryPrice() * 1.05,
                             "sell at limit");

        if (b) PrintLine("Position closed at limit: " << pos.getClosePrice());
      }

      // position entry logic
      buyAtLimit(bar + 1, close(bar) * 0.95, 1000, "buy at limit");
    }
  }
};
