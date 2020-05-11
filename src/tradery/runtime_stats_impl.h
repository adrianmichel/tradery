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

#pragma once

//#include <Tradery.h>
#include <traderytypes.h>

constexpr auto DURATION = "duration";
constexpr auto TOTAL_BAR_COUNT = "totalBarCount";
constexpr auto STATUS = "status";
constexpr auto TOTAL_SYMBOL_COUNT = "symbolCount";
constexpr auto PROCESSED_SYMBOL_COUNT = "processedSymbolCount";
constexpr auto SYMBOL_PROCESSED_WITH_ERRORS_COUNT = "symbolsProcessedWithErrorsCount";
constexpr auto RAW_TRADE_COUNT = "rawTradeCount";
constexpr auto PROCESSED_TRADE_COUNT = "processedTradeCount";  // after position sizing
constexpr auto SIGNAL_COUNT = "signalCount";
constexpr auto PROCESSED_SIGNAL_COUNT = "processedSignalCount";  // after signal sizing
constexpr auto ERROR_COUNT = "errorCount";
constexpr auto CURRENT_SYMBOL = "currentSymbol";
constexpr auto PERCENTAGE_DONE = "percentageDone";
constexpr auto SYSTEM_COUNT = "systemCount";
constexpr auto MESSAGE = "message";

inline void to_json(nlohmann::json& j, const ::RuntimeStats& rs) {
  rs.to_json(j);
}

class RuntimeStatsImpl : public RuntimeStats,
                         public tradery_x::RuntimeStats {
 private:
  // this is the number of increments that will make up 100%
  // used to show percentages between different processing elements
  unsigned __int64 _totalClicks;
  mutable Timer _timer;

  mutable std::mutex _mutex;

  double _extraPct;

 public:
  RuntimeStatsImpl() : _extraPct(0) { setStatus(RuntimeStatus::READY); }

  RuntimeStatsImpl(const nlohmann::json& j) {
    __super::duration = j[DURATION].get<double>();
    __super::processedSymbolCount =
        j[PROCESSED_SYMBOL_COUNT].get<unsigned int>();
    __super::symbolProcessedWithErrorsCount =
        j[SYMBOL_PROCESSED_WITH_ERRORS_COUNT].get<unsigned int>();
    __super::totalSymbolCount = j[TOTAL_SYMBOL_COUNT].get<unsigned int>();
    __super::systemCount = j[SYSTEM_COUNT].get<unsigned int>();
    __super::rawTradeCount = j[RAW_TRADE_COUNT].get<unsigned int>();
    __super::processedTradeCount = j[PROCESSED_TRADE_COUNT].get<unsigned int>();
    __super::signalCount = j[SIGNAL_COUNT].get<unsigned int>();
    __super::processedSignalCount =
        j[PROCESSED_SIGNAL_COUNT].get<unsigned int>();
    __super::totalBarCount = j[TOTAL_BAR_COUNT].get<unsigned int>();
    __super::errorCount = j[ERROR_COUNT].get<unsigned int>();
    __super::percentageDone = j[PERCENTAGE_DONE].get<double>();
    __super::currentSymbol = j[CURRENT_SYMBOL].get<std::string>();
    __super::status =
        (tradery_x::RuntimeStatus::type)j[STATUS].get<unsigned int>();
    __super::message = j[MESSAGE].get<std::string>();
  }

  void setTotalSymbols(unsigned int totalSymbols) {
    std::scoped_lock lock(_mutex);
    __super::totalSymbolCount = totalSymbols;
  }

  virtual void addPct(double pct) {
    std::scoped_lock  lock(_mutex);
    _extraPct += pct;
    assert(_extraPct < 100);
  }

  double getPercentage() const {
    std::scoped_lock  lock(_mutex);
    return __super::percentageDone;
  }

  virtual void step(double pct) {
    std::scoped_lock  lock(_mutex);
    percentageDone += pct;
  }

  void incSignals() {
    std::scoped_lock  lock(_mutex);
    __super::signalCount++;
  }
  void setRawTrades(unsigned int trades) {
    std::scoped_lock  lock(_mutex);
    __super::rawTradeCount = trades;
  }

  void setProcessedTrades(unsigned int trades) {
    std::scoped_lock  lock(_mutex);
    __super::processedTradeCount = trades;
  }

  void setProcessedSignals(unsigned int signals) {
    std::scoped_lock  lock(_mutex);
    __super::processedSignalCount = signals;
  }

  void incErrors() {
    std::scoped_lock  lock(_mutex);
    __super::errorCount++;
  }
  void incTotalRuns() {
    std::scoped_lock  lock(_mutex);
    __super::totalRuns++;
    __super::percentageDone +=
        (100.0 - _extraPct) / ((double)__super::totalSymbolCount);
    //    std::cout << "incTotalRuns - totalruns: " << _totalRuns << ", total
    //    symbols:" << _totalSymbols << ", crtpct: " << _crtPct << ", extraPct:
    //    " << _extraPct << std::endl;
  }
  void incErrorRuns() {
    std::scoped_lock  lock(_mutex);
    __super::errorCount++;
  }

  void incTotalBarCount(unsigned int barsCount) {
    std::scoped_lock  lock(_mutex);
    __super::totalBarCount += barsCount;
  }

  unsigned int getTotalBarCount() const {
    std::scoped_lock  lock(_mutex);
    return __super::totalBarCount;
  }

  virtual void setStatus(RuntimeStatus status) {
    switch (status) {
      case READY:
        __super::status = tradery_x::RuntimeStatus::READY;
        break;
      case RUNNING:
        __super::status = tradery_x::RuntimeStatus::RUNNING;
        break;
      case CANCELING:
        __super::status = tradery_x::RuntimeStatus::CANCELING;
        break;
      case ENDED:
        __super::status = tradery_x::RuntimeStatus::ENDED;
        break;
      case CANCELED:
        __super::status = tradery_x::RuntimeStatus::CANCELED;
        break;
      default:
        break;
    }
  }

  virtual void setMessage(const std::string& message) {
    __super::message = message;
  }

  void to_json(nlohmann::json& j) const {
    j = nlohmann::json{{DURATION, __super::duration},
                       {PROCESSED_SYMBOL_COUNT, __super::processedSymbolCount},
                       {SYMBOL_PROCESSED_WITH_ERRORS_COUNT,
                        __super::symbolProcessedWithErrorsCount},
                       {TOTAL_SYMBOL_COUNT, __super::totalSymbolCount},
                       {SYSTEM_COUNT, __super::systemCount},
                       {RAW_TRADE_COUNT, __super::rawTradeCount},
                       {PROCESSED_TRADE_COUNT, __super::processedTradeCount},
                       {SIGNAL_COUNT, __super::signalCount},
                       {PROCESSED_SIGNAL_COUNT, __super::processedSignalCount},
                       {TOTAL_BAR_COUNT, __super::totalBarCount},
                       {ERROR_COUNT, __super::errorCount},
                       {PERCENTAGE_DONE, __super::percentageDone},
                       {CURRENT_SYMBOL, __super::currentSymbol},
                       {STATUS, __super::status},
                       {MESSAGE, __super::message}};
  }

  std::string to_json() const {
    nlohmann::json j;
    to_json(j);
    return j.dump(4);
  }

 protected:
  void outputStats(std::ostream& os) const {
    std::scoped_lock  lock(_mutex);

    nlohmann::json j = *this;
    os << j.dump(4);
  }
};

class FileRuntimeStats : public RuntimeStatsImpl {
 private:
  const std::string _fileName;

 public:
  FileRuntimeStats(const std::string& fileName) : _fileName(fileName) {}

  void outputStats() const {
    if (_fileName.length() > 0) {
      std::ofstream outputStats(_fileName.c_str());

      if (outputStats) {
        //        std::cout << "wrote output stats" << std::endl;
        //        __super::outputStats( std::cout );
        __super::outputStats(outputStats);
      } else

      {
        LOG(log_error, "Could not open runtime stats file for writing");
        // todo could not open file for writing
      }

      /*
      TCHAR inbuf[ 8000 ];
      DWORD bytesRead;
      if( !CallNamedPipe( lpszPipeName.c_str(), const_cast< TCHAR* >(cmdLine
      .str().c_str() ), cmdLine.str().length() + 1, inbuf, sizeof(inbuf),
      &bytesRead, NMPWAIT_WAIT_FOREVER) )
      {
      // todo: handle errors
      }
      */
      // todo handle response from server
    }
  }
};
