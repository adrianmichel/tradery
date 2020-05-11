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

#include <path.h>
#include "ConfigurationData.h"

namespace po = boost::program_options;

class ConfigurationException : public exception {
 public:
  ConfigurationException(const std::string& message)
      : exception(message.c_str()) {}
};
// cmd line:
// simapp -dpath:D:\dev\sim\test\data\quotes -sympath: -sym:symbols.txt th:5

class Configuration : private ConfigurationData {
 private:
   bool m_help;
  void init(const std::string& cmdLine, bool validate);

 private:
  void help(const po::options_description& desc) {
    std::cout << "Cmd line: " << std::endl
      << "******" << std::endl
      << m_inputCommandLine << "******" << std::endl
      << std::endl;
  }

 public:
  Configuration(bool validate = true);
  Configuration(const std::string& cmdLine, bool validate = true);
  Configuration(const char* cmdLine, bool validate = true);
  Configuration(const Configuration& config) : ConfigurationData(config) {}

  const PositionSizingParams& positionSizingParams() const {
    return m_posSizingParams;
  }

  std::string makeSessionPath(const std::string& name) const {
    return ws2s(Path{ s2ws( m_sessionParentPath ) }.makePath(s2ws( m_sessionId ), name).c_str());
  }

  std::string makeSessionPath() const {
    return ws2s(Path{ s2ws(m_sessionParentPath) }.makePath(s2ws(m_sessionId)).c_str());
  }

  bool help() const { return m_help;  }

  std::string getSessionPath() const { return makeSessionPath(); }
  const UniqueId& getSessionId() const { return m_sessionId; }
  const TradingSystems& getSystems() const { return m_systems; }
  virtual UniqueIdVector getRunnableIds() const {
    UniqueIdVector ids;

    std::transform(m_systems.begin(), m_systems.end(), std::back_inserter(ids), [](const TradingSystem& ts) { return ts.getId(); });
    return ids;
  }

  size_t getTradingSystemCount() const { return m_systems.size(); }
  bool hasTradingSystems() const { return getTradingSystemCount() > 0; }
  const std::string& getPluginPath() const { return m_pluginPath; }
  const std::string& getPluginExt() const { return m_pluginExt; }
  const vector<std::string>& includePaths() const { return m_includePaths; }
  const vector<std::string>& libPath() const { return m_libPath; }
  const std::string& outputPath() const { return m_outputPath; }
  const std::string& dllPath() const { return m_dllPath; }
  const std::string& projectPath() const { return m_projectPath; }
  const std::string& toolsPath() const { return m_toolsPath; }
  const std::string& symbolsSource() const { return m_symbolsSource; }
  const std::string& dataSource() const { return m_dataSource; }
  const std::string& symbolsSourceFile() const { return m_symbolsSourceFile; }
  const std::string& dataSourcePath() const { return m_dataSourcePath; }
  const std::string tradesFile() const { return makeSessionPath( m_tradesFile ); }
  bool generateTrades() const { return !m_tradesFile.empty(); }
  const std::string statsFile() const { return makeSessionPath(m_statsFile); }
  bool generateStats() const { return !m_statsFile.empty(); }
  const std::string outputFile() const { return makeSessionPath(m_outputFile); }
  bool runSimulator() const { return m_runSimulator; }
  const std::string errorsFile() const { return makeSessionPath( m_errorsFile ); }
  bool hasErrorsFile() const { return !m_errorsFile.empty(); }
  const std::string& fromDateTime() const { return m_fromDateTime; }
  const std::string& toDateTime() const { return m_toDateTime; }
  const std::string& statsHandler() const { return m_statsHandler; }
  std::string statsCSVFile() const { return makeSessionPath( m_statsCSVFile ); }
  std::string signalsCSVFile() const { return makeSessionPath( m_signalsCSVFile ); }
  std::string rawSignalsCSVFile() const { return makeSessionPath(m_rawSignalsCSVFile); }
  std::string tradesCSVFile() const { return makeSessionPath( m_tradesCSVFile ); }
  std::string rawTradesCSVFile() const { return makeSessionPath(m_rawTradesCSVFile); }
  std::string equityCurveFile() const { return makeSessionPath(m_equityCurveFile ); }
  bool generateEquityCurve() const { return !m_equityCurveFile.empty(); }
  unsigned long timeout() const { return m_timeout; }

  const std::string& zipFile() const { return m_zipFile; }
  bool hasZipFile() const { return !m_zipFile.empty(); }

  std::string logFile() const { return makeSessionPath(m_logFile); }
  bool hasLogFile() const { return !m_logFile.empty(); }

  bool asyncRun() const { return m_asyncRun; }
  bool hasEndRunSignalFile() const { return !m_endRunSignalFile.empty(); }
  std::string endRunSignalFile() const { return makeSessionPath(m_endRunSignalFile); }
  std::string heartBeatFile() const { return makeSessionPath(m_heartBeatFile); }
  std::string reverseHeartBeatFile() const { return makeSessionPath(m_reverseHeartBeatFile); }
  std::string cancelFile() const { return makeSessionPath(m_cancelFile); }

  size_t symbolTimeout() const { return m_symbolTimeout; }
  size_t reverseHeartBeatPeriod() const { return m_reverseHeartBeatPeriod; }
  size_t heartBeatTimeout() const { return m_heartBeatTimeout; }
  size_t cacheSize() const { return m_cacheSize; }
  double defCommissionValue() const { return m_defCommissionValue; }
  double defSlippageValue() const { return m_defSlippageValue; }
  const std::string& defSlippageId() const { return m_defSlippageId; }
  const std::string& defCommissionId() const { return m_defCommissionId; }
  const std::string runtimeStatsFile() const { return makeSessionPath(m_runtimeStatsFile); }
  size_t maxLines() const { return m_maxLines; }
  size_t maxTotalBarCount() const { return m_maxTotalBarCount; }
  const std::string& sessionParentPath() const { return m_sessionParentPath; }
  const std::string& symbolsToChartFile() const { return m_symbolsToChartFile; }
  std::string chartDescriptionFile() const { return makeSessionPath( m_chartDescriptionFile ); }
  bool generateCharts() const { return !m_chartDescriptionFile.empty(); }
  std::string chartRootPath() const { return makeSessionPath(m_chartRootPath); }
  const std::string& osPath() const { return m_osPath; }
  const std::string& getSystemNameKeyword() const { return m_systemNameKeyword; }
  const std::string& getClassNameKeyword() const { return m_classNameKeyword; }
  std::string getTradesDescriptionFile() const { return makeSessionPath(m_tradesDescriptionFile);  }
  std::string getSignalsDescriptionFile() const { return makeSessionPath(m_signalsDescriptionFile); }
  size_t getLinesPerPage() const { return m_linesPerPage; }
  unsigned long getCPUCount() const { return m_cpuCount; }
  unsigned long getThreads() const { return m_threads; }
  ThreadAlgorithm getThreadAlg() const { return m_threadAlg; }
  virtual void setThreads(unsigned long threads) { m_threads = threads; }
  virtual void setRunSimulator(bool run = true) { m_runSimulator = true; }
  void setSessionPath(const std::string& sessionPath) { m_sessionParentPath = sessionPath; }

  std::optional< std::string > externalTriggersFile() const { return m_extTriggersFile; }
  const std::string& explicitTradesExt() const { return m_explicitTradesExt; }
  ErrorHandlingMode dataErrorHandlingMode() const {
    return m_dataErrorHandlingMode;
  }

  const std::string& startTradesDateTime() const {
    return m_startTradesDateTime;
  }

  const std::string& envPath() const { return m_envPath; }
  const std::string& envInclude() const { return m_envInclude; }
  const std::string& envLib() const { return m_envLib; }

  tradery::EnvironmentPtr getEnvironment() const;

  bool enableRunAsUser() const { return m_enableRunAsUser; }
  bool hasInclude() const { return !m_includePaths.empty(); }
  bool hasLib() const { return !m_libPath.empty(); }
  bool hasToolsPath() const { return !m_toolsPath.empty(); }
  bool hasSymbolsSourceId() const { return !m_symbolsSource.empty(); }
  bool hasSymbolsSourceFile() const { return !m_symbolsSource.empty(); }
  bool hasDataSourcePath() const { return !m_dataSourcePath.empty(); }
  bool hasDataSourceId() const { return !m_dataSource.empty(); }
  bool hasProjectPath() const { return !m_projectPath.empty(); }
  bool hasOutputPath() const { return !m_outputPath.empty(); }
  bool hasOsPath() const { return !m_osPath.empty(); }
  bool hasSessionParentPath() const { return !m_sessionParentPath.empty(); }
  bool hasExplicitTradesExt() const { return !m_explicitTradesExt.empty(); }
  bool hasStatsHandler() const { return !m_statsHandler.empty(); }
  bool hasTradesFile() const { return !m_tradesFile.empty(); }
  bool hasEquityCurveFile() const { return !m_equityCurveFile.empty(); }
  bool hasStatsFile() const { return !m_statsFile.empty(); }
  bool hasChartDescriptionFile() const {
    return !m_chartDescriptionFile.empty();
  }

  void setTradesDescriptionFile(const std::string& tradesDescriptionFile) {
    m_tradesDescriptionFile = tradesDescriptionFile;
  }

  void setTradesFile(const std::string& tradesFile) {
    m_tradesFile = tradesFile;
  }

  void setErrorsFile(const std::string& errorsFile) {
    m_errorsFile = errorsFile;
  }

  void setRuntimeStatsFile(const std::string& runtimeStatsFile) {
    m_runtimeStatsFile = runtimeStatsFile;
  }

  void setTradesCSVFile(const std::string& tradesCSVFile) {
    m_tradesCSVFile = tradesCSVFile;
  }

  void setSymbolsToChartFile(const std::string& symbolsToChartFile) {
    m_symbolsToChartFile = symbolsToChartFile;
  }

  void setChartRootPath(const std::string& chartRootPath) {
    m_chartRootPath = chartRootPath;
  }

  void setChartDescription(const std::string& chartDescriptionFile) {
    m_chartDescriptionFile = chartDescriptionFile;
  }

  void setSignalsCSVFile(const std::string& signalsCSVFile) {
    m_signalsCSVFile = signalsCSVFile;
  }

  void setSignalsDescriptionFile(const std::string& signalsDescriptionFile) {
    m_signalsDescriptionFile = signalsDescriptionFile;
  }

  void setStatsFile(const std::string& statsFile) { m_statsFile = statsFile; }

  void setStatsCSVFile(const std::string& statsCSVFile) {
    m_statsCSVFile = statsCSVFile;
  }

  void setOutputFile(const std::string& outputFile) {
    m_outputFile = outputFile;
  }

  void setEquityCurveFile(const std::string& equityCurveFile) {
    m_equityCurveFile = equityCurveFile;
  }

  void setSymbolsFileName(const std::string& symbolsFileName) {
    m_symbolsSourceFile = symbolsFileName;
  }

  void setStartDate(const DateTime startDate) {
    m_fromDateTime = startDate.date().toString(DateFormat::us);
  }

  void setEndDate(const DateTime endDate) {
    m_toDateTime = endDate.date().toString(DateFormat::us);
  }

  void setDataErrorHandlingMode(
      tradery_x::DataErrorHandling::type dataErrorHandlingMode) {
    switch (dataErrorHandlingMode) {
      case tradery_x::DataErrorHandling::type::FATAL:
        m_dataErrorHandlingMode = ErrorHandlingMode::fatal;
        break;
      case tradery_x::DataErrorHandling::type::WARNING:
        m_dataErrorHandlingMode = ErrorHandlingMode::warning;
        break;
      case tradery_x::DataErrorHandling::type::NONE:
        m_dataErrorHandlingMode = ErrorHandlingMode::ignore;
        break;
      default:
        throw ConfigurationException("Invalid data error handling value");
    }
  }

  void setSlippage(double slippage) { __super::m_defSlippageValue = slippage; }

  void setCommission(double commission) {
    __super::m_defCommissionValue = commission;
  }

  class PositionSizingParamsWrapper : public PositionSizingParams {
   public:
    PositionSizingParamsWrapper(const tradery_x::PositionSizing& positionSizing)
        : PositionSizingParams(positionSizing.initialCapital, positionSizing.maxOpenPositions,
              toPosSizeType(positionSizing.positionSizeType), positionSizing.positionSize,
              toPosSizeLimitType(positionSizing.positionSizeLimitType), positionSizing.positionSizeLimit) {}

   private:
    PosSizeLimitType toPosSizeLimitType(
      tradery_x::PositionSizeLimitType::type posSizeLimitType) {
      switch (posSizeLimitType) {
        case tradery_x::PositionSizeLimitType::type::NONE:
          return none;
        case tradery_x::PositionSizeLimitType::type::PCT_VOL:
          return pctVolume;
        case tradery_x::PositionSizeLimitType::type::VALUE:
          return limit;
        default:
          throw ConfigurationException("Invalid position size limit type");
      }
    }

    PosSizeType toPosSizeType(
      tradery_x::PositionSizeType::type posSizeType) {
      switch (posSizeType) {
        case tradery_x::PositionSizeType::type::SYSTEM:
          return PosSizeType::system_defined;
        case tradery_x::PositionSizeType::type::SHARES:
          return PosSizeType::shares;
        case tradery_x::PositionSizeType::type::VALUE:
          return PosSizeType::size;
        case tradery_x::PositionSizeType::type::PCT_EQUITY:
          return PosSizeType::pctEquity;
        case tradery_x::PositionSizeType::type::PCT_CASH:
          return PosSizeType::pctCash;
        default:
          throw ConfigurationException("Invalid position size type");
      }
    }
  };

  void setPositionSizingParams(
      const tradery_x::PositionSizing& positionSizing) {
    __super::m_posSizingParams = PositionSizingParamsWrapper(positionSizing);
  }

  const std::string getCmdLineString() const {
    std::ostringstream cmdLine;

    for (size_t n = 0; n < m_args.size(); n++) {
      bool b = m_args[n][0] == TCHAR('-');
      cmdLine << (!b ? "\"" : " ") << m_args[n] << (!b ? "\"" : " ");
    }

    return cmdLine.str();
  }

  void validate() const;
};

using ConfigurationPtr = std::shared_ptr<Configuration>;
