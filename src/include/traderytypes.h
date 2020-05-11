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
#include <string>
#include <vector>
#include <map>

namespace tradery_x {
  struct PositionSizeType {
    enum type {
      SYSTEM = 0,
      SHARES = 1,
      VALUE = 2,
      PCT_EQUITY = 3,
      PCT_CASH = 4
    };
  };

  extern const std::map<int, const char*> _PositionSizeType_VALUES_TO_NAMES;

  struct PositionSizeLimitType {
    enum type {
      NONE = 0,
      PCT_VOL = 1,
      VALUE = 2
    };
  };

  extern const std::map<int, const char*> _PositionSizeLimitType_VALUES_TO_NAMES;

  class PositionSizing{
  public:

    PositionSizing(const PositionSizing&);
    PositionSizing& operator=(const PositionSizing&);
    PositionSizing() : initialCapital(0), maxOpenPositions(0), positionSizeType((PositionSizeType::type)0), positionSize(0), positionSizeLimitType((PositionSizeLimitType::type)0), positionSizeLimit(0) {
    }

    virtual ~PositionSizing() throw();
    double initialCapital;
    int32_t maxOpenPositions;
    PositionSizeType::type positionSizeType;
    double positionSize;
    PositionSizeLimitType::type positionSizeLimitType;
    double positionSizeLimit;

    void __set_initialCapital(const double val);

    void __set_maxOpenPositions(const int32_t val);

    void __set_positionSizeType(const PositionSizeType::type val);

    void __set_positionSize(const double val);

    void __set_positionSizeLimitType(const PositionSizeLimitType::type val);

    void __set_positionSizeLimit(const double val);

    bool operator == (const PositionSizing& rhs) const
    {
      if (!(initialCapital == rhs.initialCapital))
        return false;
      if (!(maxOpenPositions == rhs.maxOpenPositions))
        return false;
      if (!(positionSizeType == rhs.positionSizeType))
        return false;
      if (!(positionSize == rhs.positionSize))
        return false;
      if (!(positionSizeLimitType == rhs.positionSizeLimitType))
        return false;
      if (!(positionSizeLimit == rhs.positionSizeLimit))
        return false;
      return true;
    }
  };

  class Range{
  public:

    Range(const Range&);
    Range& operator=(const Range&);
    Range() : startDate(0), endDate(0) {
    }

    virtual ~Range() throw();
    int64_t startDate;
    int64_t endDate;

    void __set_startDate(const int64_t val);

    void __set_endDate(const int64_t val);

    bool operator == (const Range& rhs) const
    {
      if (!(startDate == rhs.startDate))
        return false;
      if (!(endDate == rhs.endDate))
        return false;
      return true;
    }
  };

  struct DataErrorHandling {
    enum type {
      FATAL = 0,
      WARNING = 1,
      NONE = 2
    };
  };


  struct RuntimeStatus {
    enum type {
      READY = 0,
      RUNNING = 1,
      CANCELING = 2,
      ENDED = 3,
      CANCELED = 4
    };
  };

  class RuntimeStats {
  public:

    RuntimeStats(const RuntimeStats&);
    RuntimeStats() : sessionId(), duration(0), processedSymbolCount(0), symbolProcessedWithErrorsCount(0), totalSymbolCount(0), systemCount(0), rawTradeCount(0), processedTradeCount(0), signalCount(0), processedSignalCount(0), totalBarCount(0), totalRuns(0), errorCount(0), percentageDone(0), currentSymbol(), status((RuntimeStatus::type)0), message() {
    }

    virtual ~RuntimeStats() {};
    std::string sessionId;
    int32_t duration;
    int32_t processedSymbolCount;
    int32_t symbolProcessedWithErrorsCount;
    int32_t totalSymbolCount;
    int32_t systemCount;
    int32_t rawTradeCount;
    int32_t processedTradeCount;
    int32_t signalCount;
    int32_t processedSignalCount;
    int32_t totalBarCount;
    int32_t totalRuns;
    int32_t errorCount;
    double percentageDone;
    std::string currentSymbol;
    RuntimeStatus::type status;
    std::string message;

    RuntimeStats& operator=(const RuntimeStats& other25) {
      if (this != &other25) {
        sessionId = other25.sessionId;
        duration = other25.duration;
        processedSymbolCount = other25.processedSymbolCount;
        symbolProcessedWithErrorsCount = other25.symbolProcessedWithErrorsCount;
        totalSymbolCount = other25.totalSymbolCount;
        systemCount = other25.systemCount;
        rawTradeCount = other25.rawTradeCount;
        processedTradeCount = other25.processedTradeCount;
        signalCount = other25.signalCount;
        processedSignalCount = other25.processedSignalCount;
        totalBarCount = other25.totalBarCount;
        totalRuns = other25.totalRuns;
        errorCount = other25.errorCount;
        percentageDone = other25.percentageDone;
        currentSymbol = other25.currentSymbol;
        status = other25.status;
        message = other25.message;
      }
      return *this;
    }
  };
}
