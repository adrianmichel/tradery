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

#include "stdafx.h"
#include "wchart.h"

WebBarsChart* WebChartManager::createWebChart(const std::string& name, const std::string& symbol) {
  WebBarsChart* chart = new WebBarsChart(name, symbol, _chartRootPath, _reduced);
  // the default pane name is the symbol for the time being
  chart->createDefaultPane("Prices");

  return chart;
}

std::string getUniqueFileName() {
  static std::mutex _mx;
  std::scoped_lock  lock(_mx);

  static unsigned int ix = 1;

  return std::to_string( ix++ );
}

WebChartManager::WebChartManager(const std::string& name, const std::string& symbolsToChartFile,
            const std::string& chartRootPath, const std::string& chartDescriptionFile, bool reduced)
    : _chartRootPath(chartRootPath), _chartsDescriptionFile(chartDescriptionFile), _reduced(reduced) {
  try {
    if (!symbolsToChartFile.empty() && !chartDescriptionFile.empty()) {
      // get the list of symbols
      StrListPtr symbols = getSymbols(symbolsToChartFile, true);
      assert(symbols.get());

      LOG(log_info, "---------symbols to chart: ", symbolsToChartFile);
      LOG(log_info, "symbols count: ", symbols->size());

      std::string list;
      for (auto symbol : *symbols) {
        list += symbol + " ";
        __super::addChart(symbol, createWebChart(name, symbol));
      }
      LOG(log_info, "symbols: ", list);

      LOG(log_info, "---- chart Description File: ", chartDescriptionFile);
      LOG(log_info, "---- chart root path: ", chartRootPath);
    }

  }
  catch (const FileSymbolsParserException& e) {
    // could not open the symbols file
    throw ChartManagerException( "ChartManagerException: "s + e.message() );
  }
}

#define SEP " "

void WebComponent::serializeSeries(ostream& os, const std::string& name, const Series& series, const std::string& fnPostfix){
  std::string fileName = addFSlash(_path) + name + fnPostfix + ".txt";

  std::ofstream of(fileName.c_str());

  if (!of) {
    // handle error
  }
  else {
    for (size_t n = 0; n < series.size(); n++) {
      of << fixed << series[n] << SEP;
    }

    XMLCDATASerializer x(os, name, fileName);
  }
 }


PositionsSerializer::PositionsSerializer(ostream& os) : _os(os) {
  // writing data format line:
  _os << "long/short,shares,entry date,entry bar,entry price,open/closed,exit date,exit bar,exit price,gain,pct gain" << std::endl;
}

void PositionsSerializer::onPosition(tradery::Position pos) {
  _os << pos.isLong() << SEP << pos.getShares() << SEP
    << pos.getEntryTime().date().to_iso_extended_string() << SEP
    << pos.getEntryBar() << SEP << pos.getEntryPrice() << SEP
    << pos.isOpen();

  if (pos.isClosed()) {
    _os << SEP << pos.getCloseTime().date().to_iso_extended_string() << SEP
      << pos.getCloseBar() << SEP << pos.getClosePrice() << SEP
      << pos.getGain() << SEP << pos.getPctGain();
  }
  _os << std::endl;
}
