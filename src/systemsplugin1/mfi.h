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

class MFISystem : public BarSystem<MFISystem> {
 public:
  //  constructor of the System1 class
  // does one time initialization of the class, which whill hold for the life of
  // the class, indicated by the "const" qualifier associated with the
  // variables. Per run intialization should be done in the method "init".
  MFISystem(const std::vector<std::string>* params = 0)
      // set the system name and the holding period to 2 days, profit target to
      // 5%
      : BarSystem<MFISystem>(Info("A39D47BC-8AFA-422b-B04E-6E0D31C2F86E",
                                  "Money Flow Index - WL", ""))

  {}

 public:
  ~MFISystem(void) {}

  virtual void run() {
    unsigned int period = 10;
    int lowThreshold = 5;
    int highThreshold = 100 - lowThreshold;

    Series mfi = MFI(period);

    installProfitTarget(5);
    installStopLoss(5);

    for (Index bar = period; bar < size(); bar++) {
      applyAutoStops(bar);

      bool highCond = mfi[bar - 1] <= lowThreshold AND mfi[bar] > lowThreshold;

      bool lowCond = mfi[bar - 1] >= highThreshold AND mfi[bar] < highThreshold;

      if (hasOpenPositions()) {
        Position pos = getLastOpenPosition();
        if (highCond AND pos.isLong())
          sellAtMarket(bar + 1, pos, "sell at market");

        if (lowCond AND pos.isShort())
          coverAtMarket(bar + 1, pos, "cover at market");
      } else {
        if (highCond) shortAtMarket(bar + 1, 1000, "buy at market");
        if (lowCond) buyAtMarket(bar + 1, 1000, "short at market");
      }
    }
  }
};
