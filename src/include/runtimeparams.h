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

constexpr auto DEFAULT_THREADS = 2;

class RuntimeParams {
 private:
  unsigned long _threads;
  ThreadAlgorithm _threadAlgorithm;
  DateTime _startTradesDateTime;
  DateTimeRangePtr _range;
  PositionSizingParams _posSizing;

  bool _chartsEnabled;
  bool _statsEnabled;
  bool _equityEnabled;
  bool _tradesEnabled;
  bool _signalsEnabled;
  bool _outputEnabled;

 public:
   RuntimeParams()
      : _threads(DEFAULT_THREADS), _range(std::make_shared< DateTimeRange >(LocalTimeSec() - Days(30), LocalTimeSec())) {}

  void setRange(DateTimeRangePtr range) {
    _range = range;
  }

  void setStartTradesDateTime(DateTime startTradesDateTime) {
    _startTradesDateTime = startTradesDateTime;
  }

  void setPositionSizingParams(const PositionSizingParams& psp) {
    _posSizing.set(psp);
  }

  void setChartsEnabled(bool b) { _chartsEnabled = b; }
  void setEquityEnabled(bool b) { _equityEnabled = b; }
  void setStatsEnabled(bool b) { _statsEnabled = b; }
  void setTradesEnabled(bool b) { _tradesEnabled = b; }

  void setThreads(unsigned long threads) {
    _threads = threads;
  }

  void setThreadAlgorithm(ThreadAlgorithm ta) { _threadAlgorithm = ta; }

  bool chartsEnabled() const { return _chartsEnabled; }
  bool equityCurveEnabled() const { return _equityEnabled; }
  bool statsEnabled() const { return _statsEnabled; }
  bool tradesEnabled() const { return _tradesEnabled; }
  bool signalsEnabled() const { return _signalsEnabled; }
  bool outputEnabled() const { return _outputEnabled; }

  DateTime startTradesDateTime() const { return _startTradesDateTime; }

  unsigned long getThreads() const { return _threads; }
  ThreadAlgorithm getThreadAlgorithm() const { return _threadAlgorithm; }
  DateTimeRangePtr getRange() const {
    return _range;
  }

  const PositionSizingParams* positionSizing() const { return &_posSizing; }
};

