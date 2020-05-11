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

class LongShortKeltnerBands : public BarSystem<LongShortKeltnerBands> {
 public:
  //  constructor of the System1 class
  // does one time initialization of the class, which whill hold for the life of
  // the class, indicated by the "const" qualifier associated with the
  // variables. Per run intialization should be done in the method "init".
  LongShortKeltnerBands(const std::vector<std::string>* params = 0)
      // set the system name and the holding period to 2 days, profit target to
      // 5%
      : BarSystem<LongShortKeltnerBands>(Info(
            "00434C1B-A8B3-4565-B096-1E38C713294D", "LongShortKeltnerBands - ",
            "WL name: A lindq type long/short limit system using Keltner "
            "Bands"))

  {}

 public:
  ~LongShortKeltnerBands(void) {}

  void run() {
    int days = 3;
    int var1 = 30;
    int var2 = 9;
    double profitTarget = 14;
    int start = var1 + var2;

    Series factor1 = highSeries() - lowSeries();
    Series factor2 = (closeSeries() + highSeries() + lowSeries()) / 3;
    Series factor3 = factor1.EMA(var1);
    Series factor4 = factor2.EMA(var2);

    Series kUp = factor4 + factor3;
    Series kDown = factor4 - factor3;

    Pane defPane = getDefaultPane();
    defPane.drawSeries("kDown", kDown);
    defPane.drawSeries("kUp", kUp);

    Series buyLimitPrice = kDown * 0.85;
    Series shortLimitPrice = kUp * 1.12;

    installProfitTarget(profitTarget);
    installTimeBasedExit(days);

    for (Index bar = start + 1; bar < size(); bar++) {
      applyAutoStops(bar);

      buyAtLimit(bar + 1, buyLimitPrice[bar], 1000, "buy at limit");
      shortAtLimit(bar + 1, shortLimitPrice[bar], 1000, "short at limit");
    }
  }
};
