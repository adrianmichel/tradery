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
 * Shows a basic use for data and series synchronization code.
 * It also shows how to get data for a symbol different than the default symbol
 */
class System5 : public BarSystem<System5> {
 public:
  System5(const std::vector<std::string>* params = 0)
      : BarSystem<System5>(Info(
            "6A01AF7B-C49C-4334-8C89-DFB99637963D", "System 5 - multi-symbol",
            "Illustrates how to create a system that uses more than one "
            "symbol, and how to synchronize data from multiple symbols")) {}

  virtual bool init(const std::string& defSymbol) {
    // reset the symbols iterator, so it will point to the first symbol
    resetSymbols();
    // get the first symbol
    // STRING firstSymbol = GET_NEXT_SYMBOL;
    String firstSymbol("yhoo");

    // only process if the current symbol is not the first symbol
    if (defSymbol != firstSymbol) {
      // get data for the first symbol
      Bars firstSymbolData = bars(firstSymbol);

      if (firstSymbolData) {
        // if data available
        // create a data synchronizer for the first symbol. The refernce is the
        // current symbol
        firstSymbolData.synchronize(DEF_BARS);
        // create a series synchronizer for the sma 5 of the closing prices of
        // the first symbol
        Series ss = firstSymbolData.closeSeries().SMA(5);
        //        SeriesSynchronizer ss( ds, CLOSE_SERIES( firstSymbolData
        //        ).SMA( 5 ) );

        // add the current close series to the synchronized first symbol close
        // series
        Series sum = CLOSE_SERIES + ss;

        // print to the output window some of the values of the two close prices
        // and the sum
        for (size_t n = 0; n < 20; n++)
          PRINT_LINE(time(n).to_simple_string()
                     << ", " << close(n) << ", " << ss[n] << ", " << sum[n]);
      }
    }
    return true;
  }

  virtual void run() {}

  void onBar(Index index) {}
};
