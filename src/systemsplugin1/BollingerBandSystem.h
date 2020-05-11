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

class BollingerBandSystem : public BarSystem<BollingerBandSystem> {
 public:
  BollingerBandSystem(const std::vector<std::string>* params = 0)
      : BarSystem<BollingerBandSystem>(
            Info("0DAF8660-9114-4c69-8E1F-A96B924D8343",
                 "Simple Bollinger Bands system", "")) {}

  ~BollingerBandSystem(void) {}

  enum PriceStatus { neutral, overbought, oversold };

  Series _bbUpper;
  Series _bbLower;
  Series _movingAverage;

  PriceStatus _status;

  virtual bool init() {
    // calculate the lower and upper Bollinger bands,
    // period 20 bars, 2 standard deviations
    _bbUpper = closeSeries().BBandUpper(20, 2);
    _bbLower = closeSeries().BBandLower(20, 2);

    // calculate the 20 day simple moving average of the close prices
    _movingAverage = closeSeries().SMA(20);

    _status = neutral;

    return true;
  }

  virtual void run() { FOR_EACH_BAR(20); }

  // called for each bar
  virtual void onBar(Index bar) {
    FOR_EACH_OPEN_POSITION(bar);

    if (!HAS_OPEN_POSITIONS && _status == oversold &&
        close(bar) > _movingAverage[bar]) {
      BUY_AT_MARKET(bar + 1, 1000, "Buy signal");
      _status = neutral;
    }

    if (close(bar) <= _bbLower[bar])
      _status = oversold;
    else if (close(bar) >= _bbUpper[bar])
      _status = overbought;
  }

  virtual bool onOpenPosition(tradery::Position pos, Index bar) {
    if (_status == overbought && close(bar) < _movingAverage[bar]) {
      SELL_AT_MARKET(bar + 1, pos, "Sell signal");
      _status = neutral;
    }
    return true;
  }
};

class BollingerBandClassicSystem
    : public BarSystem<BollingerBandClassicSystem> {
 public:
  BollingerBandClassicSystem(const std::vector<std::string>* params = 0)
      : BarSystem<BollingerBandClassicSystem>(
            Info("8C5443F0-2063-4d1f-A339-72B3B5E38ADC",
                 "Simple Bollinger Bands Classic system",
                 "Classic in the sense that it is not object oriented")) {}

  ~BollingerBandClassicSystem(void) {}

  // an enumeration of possible status values
  // neutral is 0, overbought is 1, and
  // oversold is 2
  enum PriceStatus { neutral, overbought, oversold };

  void run() {
    // the series variables for the 3 indicators:
    // upper/lower Bollinger Bands, and
    // a median moving average band

    Series _bbUpper;
    Series _bbLower;
    Series _movingAverage;

    // a variable for the current status
    PriceStatus _status;

    // calculate the lower and upper Bollinger bands,
    // period 20 bars, 2 standard deviations
    _bbUpper = closeSeries().BBandUpper(20, 2);
    _bbLower = closeSeries().BBandLower(20, 2);

    // calculate the 20 day simple moving average
    // of the closing prices
    _movingAverage = closeSeries().SMA(20);

    // the initial status is neutral:
    // neither overbought, nor oversold
    _status = neutral;

    Pane defPane = getDefaultPane();

    defPane.drawSeries("bbupper", _bbUpper);
    defPane.drawSeries("bblower", _bbLower);
    defPane.drawSeries("SMA 20", _movingAverage);

    Pane bbpane = createPane("bb");

    bbpane.drawSeries("bbupper", _bbUpper);
    bbpane.drawSeries("bblower", _bbLower);
    bbpane.drawSeries("SMA 20", _movingAverage);

    for (Index bar = 20; bar < barsCount(); bar++) {
      // first handle open positions at the current bar
      // see onOpenPosition
      for (Position pos = getFirstOpenPosition(); pos;
           pos = getNextOpenPosition()) {
        if (_status == overbought && close(bar) < _movingAverage[bar]) {
          sellAtMarket(bar + 1, pos, "Sell signal");
          // after exit, status becomes neutral again
          _status = neutral;
        }
      }

      // entry rules:
      // if there are no open positions for the current symbol
      // and the status is oversold and the closing price crossed
      // above the median bar, then entry at market on the following bar
      if (!hasOpenPositions() && _status == oversold &&
          close(bar) > _movingAverage[bar]) {
        buyAtMarket(bar + 1, 1000, "Buy signal");
        // after entry, the status becomes neutral again
        _status = neutral;
      }

      // adjust the price status relative to the Bollinger Bands
      if (close(bar) <= _bbLower[bar])
        _status = oversold;
      else if (close(bar) >= _bbUpper[bar])
        _status = overbought;
    }
  }
};
