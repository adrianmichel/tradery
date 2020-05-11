#pragma once
#include <string>
#include <vector>

namespace tradery_x {
  class System {
  public:

    std::string dbId;
    std::string name;
    std::string description;
    std::string code;

    System(const ::tradery_x::System& system)
      : dbId(system.dbId), name(system.name), description(system.description),
      code(system.code) {
    }
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

    RuntimeStats& RuntimeStats::operator=(const RuntimeStats& other25) {
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


    void __set_sessionId(const std::string& val);

    void __set_duration(const int32_t val);

    void __set_processedSymbolCount(const int32_t val);

    void __set_symbolProcessedWithErrorsCount(const int32_t val);

    void __set_totalSymbolCount(const int32_t val);

    void __set_systemCount(const int32_t val);

    void __set_rawTradeCount(const int32_t val);

    void __set_processedTradeCount(const int32_t val);

    void __set_signalCount(const int32_t val);

    void __set_processedSignalCount(const int32_t val);

    void __set_totalBarCount(const int32_t val);

    void __set_totalRuns(const int32_t val);

    void __set_errorCount(const int32_t val);

    void __set_percentageDone(const double val);

    void __set_currentSymbol(const std::string& val);

    void __set_status(const RuntimeStatus::type val);

    void __set_message(const std::string& val);

    bool operator == (const RuntimeStats& rhs) const
    {
      if (!(sessionId == rhs.sessionId))
        return false;
      if (!(duration == rhs.duration))
        return false;
      if (!(processedSymbolCount == rhs.processedSymbolCount))
        return false;
      if (!(symbolProcessedWithErrorsCount == rhs.symbolProcessedWithErrorsCount))
        return false;
      if (!(totalSymbolCount == rhs.totalSymbolCount))
        return false;
      if (!(systemCount == rhs.systemCount))
        return false;
      if (!(rawTradeCount == rhs.rawTradeCount))
        return false;
      if (!(processedTradeCount == rhs.processedTradeCount))
        return false;
      if (!(signalCount == rhs.signalCount))
        return false;
      if (!(processedSignalCount == rhs.processedSignalCount))
        return false;
      if (!(totalBarCount == rhs.totalBarCount))
        return false;
      if (!(totalRuns == rhs.totalRuns))
        return false;
      if (!(errorCount == rhs.errorCount))
        return false;
      if (!(percentageDone == rhs.percentageDone))
        return false;
      if (!(currentSymbol == rhs.currentSymbol))
        return false;
      if (!(status == rhs.status))
        return false;
      if (!(message == rhs.message))
        return false;
      return true;
    }
    bool operator != (const RuntimeStats& rhs) const {
      return !(*this == rhs);
    }

  };

  class SessionParams{
  public:

    SessionParams(const SessionParams&);
    SessionParams& operator=(const SessionParams&);
    SessionParams() : explicitTradesExt(), generateSignals(0), generateTrades(0), generateStats(0), generateOutput(0), generateEquityCurve(0), generateCharts(0), generateZipFile(0), maxChartsCount(0), timeout(0), symbolTimeout(0), heartBeatTimeout(0), reverseHeartBeatTimeout(0), slippage(0), commission(0), dataErrorHandling((DataErrorHandling::type)0), maxLines(0), startTradesDate(0) {
    }

    virtual ~SessionParams() throw();
    std::string explicitTradesExt;
    std::vector<std::string>  symbols;
    std::vector<System>  systems;
    bool generateSignals;
    bool generateTrades;
    bool generateStats;
    bool generateOutput;
    bool generateEquityCurve;
    bool generateCharts;
    bool generateZipFile;
    int32_t maxChartsCount;
    int32_t timeout;
    int32_t symbolTimeout;
    int32_t heartBeatTimeout;
    int32_t reverseHeartBeatTimeout;
    PositionSizing positionSizing;
    double slippage;
    double commission;
    Range range;
    DataErrorHandling::type dataErrorHandling;
    int32_t maxLines;
    Date startTradesDate;

    _SessionParams__isset __isset;

    void __set_explicitTradesExt(const std::string& val);

    void __set_symbols(const std::vector<std::string>& val);

    void __set_systems(const std::vector<System>& val);

    void __set_generateSignals(const bool val);

    void __set_generateTrades(const bool val);

    void __set_generateStats(const bool val);

    void __set_generateOutput(const bool val);

    void __set_generateEquityCurve(const bool val);

    void __set_generateCharts(const bool val);

    void __set_generateZipFile(const bool val);

    void __set_maxChartsCount(const int32_t val);

    void __set_timeout(const int32_t val);

    void __set_symbolTimeout(const int32_t val);

    void __set_heartBeatTimeout(const int32_t val);

    void __set_reverseHeartBeatTimeout(const int32_t val);

    void __set_positionSizing(const PositionSizing& val);

    void __set_slippage(const double val);

    void __set_commission(const double val);

    void __set_range(const Range& val);

    void __set_dataErrorHandling(const DataErrorHandling::type val);

    void __set_maxLines(const int32_t val);

    void __set_startTradesDate(const Date val);

    bool operator == (const SessionParams& rhs) const
    {
      if (!(explicitTradesExt == rhs.explicitTradesExt))
        return false;
      if (!(symbols == rhs.symbols))
        return false;
      if (!(systems == rhs.systems))
        return false;
      if (!(generateSignals == rhs.generateSignals))
        return false;
      if (!(generateTrades == rhs.generateTrades))
        return false;
      if (!(generateStats == rhs.generateStats))
        return false;
      if (!(generateOutput == rhs.generateOutput))
        return false;
      if (!(generateEquityCurve == rhs.generateEquityCurve))
        return false;
      if (!(generateCharts == rhs.generateCharts))
        return false;
      if (!(generateZipFile == rhs.generateZipFile))
        return false;
      if (!(maxChartsCount == rhs.maxChartsCount))
        return false;
      if (!(timeout == rhs.timeout))
        return false;
      if (!(symbolTimeout == rhs.symbolTimeout))
        return false;
      if (!(heartBeatTimeout == rhs.heartBeatTimeout))
        return false;
      if (!(reverseHeartBeatTimeout == rhs.reverseHeartBeatTimeout))
        return false;
      if (!(positionSizing == rhs.positionSizing))
        return false;
      if (!(slippage == rhs.slippage))
        return false;
      if (!(commission == rhs.commission))
        return false;
      if (!(range == rhs.range))
        return false;
      if (!(dataErrorHandling == rhs.dataErrorHandling))
        return false;
      if (!(maxLines == rhs.maxLines))
        return false;
      if (!(startTradesDate == rhs.startTradesDate))
        return false;
      return true;
    }
  };
}
