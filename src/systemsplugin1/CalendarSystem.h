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

class CalendarSystem : public BarSystem<CalendarSystem> {
 public:
  //  constructor of the System1 class
  // does one time initialization of the class, which whill hold for the life of
  // the class, indicated by the "const" qualifier associated with the
  // variables. Per run intialization should be done in the method "init".
  CalendarSystem(const std::vector<std::string>* params = 0)
      // set the system name and the holding period to 2 days, profit target to
      // 5%
      : BarSystem<CalendarSystem>(Info("510D5B99-31C8-4117-8E19-6C08D4B78844",
                                       "May-October - by odelys in WL", ""))

  {}

 public:
  ~CalendarSystem(void) {}

  virtual void run() {
    installProfitTarget(5);
    installStopLoss(5);

    // state 0 - we are somewhere whithin the buying period
    // state 1 - we are somewhere in the shorting period
    int state = 0;

    for (Index bar = 0; bar < size(); bar++) {
      applyAutoStops(bar);

      Date d = date(bar);

      if (d.month() == 5 && state == 1) {
        closeAllShortAtMarket(bar + 1, "closing all short at market");
        buyAtMarket(bar + 1, 1000, "buy at market");

        state = 0;
      }

      if (d.month() == 10 && state == 0) {
        closeAllLongAtMarket(bar + 1, "closing all long");
        shortAtMarket(bar + 1, 1000, "short at market");

        state = 1;
      }

      Series s;

      //		s[5;
    }
  }
};
