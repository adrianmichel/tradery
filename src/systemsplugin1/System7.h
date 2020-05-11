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

using namespace tradery;

class System7 : public BarSystem<System7> {
 public:
  //  constructor of the System1 class
  // does one time initialization of the class, which whill hold for the life of
  // the class, indicated by the "const" qualifier associated with the
  // variables. Per run intialization should be done in the method "init".
  System7(const std::vector<std::string>* params = 0)
      // set the system name and the holding period to 2 days, profit target to
      // 5%
      : BarSystem<System7>(
            Info("CF965196-F0BB-49ed-8A5A-DBB3A31620D6", "System 7 - ", ""))

  {}

  ~System7() {}

  //**************************************
  // System varaibles
  //**************************************
  Series slowSMAHigh;
  Series fastSMAHigh;
  Series slowSMALow;
  Series fastSMALow;

  //**************************************
  // System initialization (variables or other resources)
  //**************************************
  bool init(const String& symbol) {
    slowSMAHigh = HIGH_SERIES.SMA(10);
    fastSMAHigh = HIGH_SERIES.SMA(5);
    slowSMALow = LOW_SERIES.SMA(10);
    fastSMALow = LOW_SERIES.SMA(5);

    INSTALL_PROFIT_TARGET(5);
    INSTALL_TIME_BASED_EXIT(5);

    return true;
  }

  void run() {
    //**************************************
    // Do processing in onBar for each bar starting at bar 10
    //**************************************
    FOR_EACH_BAR(10);
  }

  // called for each bar
  void onBar(Index bar) {
    //**************************************
    // Apply various exit strategies
    //**************************************
    // apply built in auto exit strategies
    APPLY_AUTO_STOPS(bar);
    // apply user defined exit strategies
    // by calling onOpenPosition for each open position
    FOR_EACH_OPEN_POSITION(bar);

    //**************************************
    // Apply entry strategies
    //**************************************
    if (openPositionsCount() < 3 AND fastSMALow.crossUnder(bar, slowSMALow))
      BUY_AT_MARKET(bar + 1, 1000, "Buy at market on crossover");
  }

  bool onOpenPosition(Position pos, Index bar) {
    //**************************************
    // Apply user defined exit strategies here
    // for each open position
    //**************************************
    if (fastSMAHigh.crossOver(bar, slowSMAHigh))
      SELL_AT_MARKET(bar + 1, pos, "Sell at market on crossunder");

    return true;
  }
};
