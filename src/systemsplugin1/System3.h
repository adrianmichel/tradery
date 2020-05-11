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

/**
 * Illustrates how to set an automatic break even stop exit strategy
 */
class System3 : public BarSystem<System3> {
 public:
  System3(const std::vector<std::string>* params = 0)
      : BarSystem<System3>(Info("5BCB483E-5D13-4934-AA37-00D8B28D0EA6",
                                "System 3 - automatic exit strategy",
                                "Illustrates how to install one of the "
                                "automatic exit strategies, in this case break "
                                "even stop")) {}

  virtual void onBar(Index bar) {
    // processes all auto exit strategies, in this case we only have break even
    // stop 1%
    APPLY_AUTO_STOPS(bar);
    // entry a position at market
    SHORT_AT_MARKET(bar + 1, 1000, "short entry");

    if (CLOSE_SERIES.SMA(12).crossUnder(bar, CLOSE_SERIES)) {
    }
  }

  virtual void run() {
    // install the break even stop 1%, will be executed in the dataHandler
    INSTALL_BREAK_EVEN_STOP(1);
    // run the data handler for each bar
    FOR_EACH_BAR(0);
  }
};
