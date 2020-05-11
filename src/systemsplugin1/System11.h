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
class System11 : public BarSystem<System11> {
 public:
  System11(const std::vector<std::string>* params = 0)
      : BarSystem<System11>(Info("B9357770-295C-4f30-876A-6DACE66F0630",
                                 "System 11 - for Joe", "")) {}

  //***********************
  // start of the system code as it is on the website
  //***********************
  struct SymbolROC {
    String symbol;
    double roc;

    SymbolROC(String _symbol, double _roc) : symbol(_symbol), roc(_roc) {}
  };

  class SymbolROCArray {
   private:
    unsigned int maxCount;

    Array(SymbolROC) array;

   public:
    SymbolROCArray() {
      // RocDictionary Size
      // Symbolist Must be >= Size
      maxCount = 5;
    }

    int findMinROC() {
      // DebugBreak();
      int minROCIndex = -1;
      double minROCValue;

      for (unsigned int n = 0; n < array.size(); n++) {
        SymbolROC x = array[n];
        if (n == 0 || x.roc < minROCValue) {
          minROCValue = x.roc;
          minROCIndex = n;
        }
      }
      return minROCIndex;
    }

    void add(String symbol, double roc) {
      if (array.size() < maxCount) {
        array.add(SymbolROC(symbol, roc));
      } else {
        // find the min roc and replace it with the new roc if lower
        int minIndex = findMinROC();
        // minIndex should not be -1, which means the array is empty
        if (roc > array[minIndex].roc) array[minIndex] = SymbolROC(symbol, roc);
      }
    }

    bool hasSymbol(String symbol) {
      for (unsigned int n = 0; n < array.size(); n++) {
        SymbolROC& sroc = array[n];
        if (sroc.symbol == symbol) return true;
      }
      return false;
    }

    String toString() {
      String str;

      for (unsigned int n = 0; n < array.size(); n++) {
        SymbolROC& s = array[n];
        str += s.symbol + ": " + std::to_string( s.roc ) + ", ";
      }
      return str;
    }
  };

  class ROCsDictionary {
   private:
    Dictionary(unsigned int, SymbolROCArray) dict;

   public:
    void add(unsigned int bar, String symbol, double roc) {
      if (!dict.has(bar)) {
        dict.add(bar, SymbolROCArray());
      }
      SymbolROCArray& s = dict[bar];
      s.add(symbol, roc);
    }

    bool hasSymbol(unsigned int bar, String symbol) {
      SymbolROCArray& s = dict[bar];
      return s.hasSymbol(symbol);
    }

    void print() {
      Array(unsigned int)& barIndexes = dict.getKeys();

      String str;

      for (unsigned int n = 0; n < barIndexes.size(); n++) {
        unsigned int index = barIndexes[n];

        SymbolROCArray& s = dict[index];

        PrintLine(str << "bar: " << n << " - " << s.toString());
      }
    }
  };

  ROCsDictionary rocDict;

  void populateROCDictionary(Bars refBars, unsigned int period) {
    PrintLine("Populating the dictionary");
    for (String crtSymbol = getFirstSymbol(); !crtSymbol.empty();
         crtSymbol = getNextSymbol()) {
      Bars crtBars = bars(crtSymbol);

      if (crtBars) {
        crtBars.synchronize(refBars);
        Series roc = msRocSeries(crtBars.closeSeries(), period);
        for (unsigned int n = period; n < crtBars.size(); n++) {
          // PrintLine( "Populating dict " << crtSymbol << ", bar: " << n );

          rocDict.add(n, crtSymbol, roc[n]);
        }
      }
    }
    PrintLine("End populating the dictionary");
  }

  Series msRocSeries(Series series1, int period) {
    // test for valid period - default to 2 if not valid
    if (period < 2) period = 2;

    // create the result series of the same size as input series
    Series result(series1.size());

    // go through all elements in the input series and calculate the
    // msRoc and assign the result to the result series
    for (unsigned int n = period; n < series1.size(); n++)
      result[n] =
          ((series1[n] - series1[n - (n - 1)]) / n) / series1[n - (n - 1)];
    return result;
  }

  void run() {
    if (getSymbol() == getFirstSymbol()) {
      PrintLine("Running on : " << getSymbol());
      // only do processing on the first symbol - we'll iterate
      // through all the symbols in the list
      STRING refSymbol = "IWN";

      Bars refBars = bars(refSymbol);

      if (refBars) {
        PrintLine("got ref bars");
        DateTime t = time(0);
        PrintLine("Bar[0] = " << t.to_simple_string());

        // Set msROCperiod = 5
        populateROCDictionary(refBars, 5);

        rocDict.print();

        for (String crtSymbol = getFirstSymbol(); !crtSymbol.empty();
             crtSymbol = getNextSymbol()) {
          PrintLine("Processing: " << crtSymbol);
          Bars crtBars = bars(crtSymbol);

          if (crtBars) {
            // if data available
            // create a data synchronizer for the current symbol
            // with the reference being the current symbol
            crtBars.synchronize(refBars);
            // Set Switch Frequency (Rebalance Period)= 10 periods
            for (unsigned int bar = 5; bar < crtBars.size(); bar += 10) {
              tradery::Position pos = getLastPosition();

              if (rocDict.hasSymbol(bar, crtSymbol)) {
                if (!pos || !pos.isOpen() || pos.getSymbol() != crtSymbol)
                  buyAtMarket(crtBars, bar + 1, 1000, "buying at market");
              } else {
                if (pos && pos.isOpen() && pos.getSymbol() == crtSymbol)
                  sellAtMarket(crtBars, bar + 1, pos, "selling at market");
              }
            }
          }
          PrintLine("done processing crtSymbol: " << crtSymbol);
        }
      } else {
        PrintLine("Could not load symbol " << refSymbol);
      }
      exit("done processing first symbol, exiting...");
    }

    PrintLine("Done!");
  }
  //***********************
  // end of the system code as it is on the website
  //***********************
};
