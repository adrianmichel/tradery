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

class System8 : public BarSystem<System8> {
 public:
  //  constructor of the System1 class
  // does one time initialization of the class, which whill hold for the life of
  // the class, indicated by the "const" qualifier associated with the
  // variables. Per run intialization should be done in the method "init".
  System8(const std::vector<std::string>* params = 0)
      // set the system name and the holding period to 2 days, profit target to
      // 5%
      : BarSystem<System8>(
            Info("8B609E49-A9BB-4fb9-A0FC-45C0B58C514D", "System 8 - ", ""))

  {}

  ~System8() {}

  Series QQV;
  Series QQVBBUpper;
  Series QQVBBLower;
  Series QQVMedianCloseLong;
  Series QQVMedianCloseShort;
  int period1;
  int stdDev1;
  int period2;
  int stdDev2;
  int period3;
  int period4;
  double trigger;
  double level;
  int holdingBars;
  double stopLoss;

  virtual bool init(const std::string& symbol) {
    // The QQV seriesc contains the closing values for the QQV
    //    BARS QQVData = getBars( "QQV" );
    Bars QQVData = bars("INTC");
    if (QQVData) {
      QQVData.synchronize(DEF_BARS);
      // create a series synchronizer for the sma 5 of the closing prices of the
      // first symbol
      QQV = QQVData.closeSeries().SMA(5);

      QQVBBUpper = QQV.BBandUpper(period1, stdDev1);
      QQVBBLower = QQV.BBandLower(period2, stdDev2);
      QQVMedianCloseLong = QQV.EMA(period3);
      QQVMedianCloseShort = QQV.EMA(period4);

      INSTALL_STOP_LOSS(stopLoss);
      INSTALL_TRAILING_STOP(trigger, level);
      INSTALL_TIME_BASED_EXIT(holdingBars);
      return true;
    } else {
      PRINT_LINE("QQV data not available!");
      return false;
    }
  }

  // the run method - this is called by the framework after a succesful
  // initialization it just does forEachBar which in turn calls dataHandler for
  // all the available bars
  virtual void run() { FOR_EACH_BAR(25); }

  // called for each bar
  virtual void onBar(Index bar) {
    // apply auto stops - so check for profit target and holding period in our
    // case
    APPLY_AUTO_STOPS(bar);

    if (!HAS_OPEN_POSITIONS) {
      if (QQV.crossOver(bar, QQVBBUpper))
        BUY_AT_LIMIT(bar + 1, LOW_SERIES[bar], 1000, "Open long");
      else if (QQV.crossOver(bar, QQVBBLower))
        SHORT_AT_LIMIT(bar + 1, HIGH_SERIES[bar], 1000, "Open short");
    } else
      FOR_EACH_OPEN_POSITION(bar);
  }

  virtual bool onOpenPosition(Position pos, Index bar) {
    if (QQV.crossUnder(bar, QQVMedianCloseLong))
      SELL_AT_LIMIT(bar + 1, pos, high(bar), "Sell at limit");
    else if (QQV.crossUnder(bar, QQVMedianCloseShort))
      COVER_AT_LIMIT(bar + 1, pos, low(bar), "Cover at limit");

    return true;
  }
};

class QQVSystem : public BarSystem<QQVSystem> {
 public:
  //  constructor of the System1 class
  // does one time initialization of the class, which whill hold for the life of
  // the class, indicated by the "const" qualifier associated with the
  // variables. Per run intialization should be done in the method "init".
  QQVSystem(const std::vector<std::string>* params = 0)
      // set the system name and the holding period to 2 days, profit target to
      // 5%
      : BarSystem<QQVSystem>(
            Info("34B96937-4EA2-4fb1-B1B3-DAB09F381359", "QQVSystem ", ""))

  {}

  ~QQVSystem() {}

  void run() {
    if (getSymbol() != getFirstSymbol()) exit("done processing");

    // periods and standard deviations for the Bollinger Bands
    double stdDev1 = 1.5;
    double stdDev2 = 1.5;
    int period1 = 5;
    int period2 = 10;

    // periods for the 2 medians
    int period3 = 15;
    int period4 = 20;

    // auto exit strategies values
    // the new level is set to grow in 2% increments
    double trigger = 2;
    // the stop is 1% below the level
    double level = 1;
    int holdingBars = 10;
    double stopLoss = 2;

    // initialize different series
    // The QQV seriesc contains the closing values for the QQV
    Bars qqv = getBarsForSymbol("^QQV");

    if (!qqv) exit("qqv not available");

    synchronize(qqv);

    Series qqvClose = qqv.closeSeries();

    // calculate the various bollinger bands and the
    Series QQVBBUpper = qqvClose.BBandUpper(period1, stdDev1);
    Series QQVBBLower = qqvClose.BBandLower(period2, stdDev2);

    // calculate the medians used for long/short positions
    // in practice they can be different
    Series QQVMedianCloseLong = qqvClose.EMA(period3);
    Series QQVMedianCloseShort = qqvClose.EMA(period4);

    // install the various auto exit strategies
    installStopLoss(stopLoss);
    installTrailingStop(trigger, level);
    installTimeBasedExit(holdingBars);

    for (Index bar = 25; bar < barsCount(); bar++) {
      applyAutoStops(bar);

      if (!hasOpenPositions()) {
        if (qqvClose.crossOver(bar, QQVBBUpper))
          buyAtLimit(bar + 1, low(bar), 1000, "Open long");
        else if (qqvClose.crossUnder(bar, QQVBBLower))
          shortAtLimit(bar + 1, high(bar), 1000, "Open short");
      } else {
        for (Position pos = getFirstOpenPosition(); pos;
             pos = getNextOpenPosition()) {
          if (qqvClose.crossUnder(bar, QQVMedianCloseLong)) {
            if (pos.isLong())
              sellAtLimit(bar + 1, pos, high(bar), "Sell at limit");
          } else if (qqvClose.crossOver(bar, QQVMedianCloseShort)) {
            if (pos.isShort())
              coverAtLimit(bar + 1, pos, low(bar), "Cover at limit");
          }
        }
      }
    }
  }
};
