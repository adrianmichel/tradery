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

#include "System.h"

class ConfigurationData {
 protected:
  UniqueId m_sessionId;
  std::string m_inputCommandLine;
  TradingSystems m_systems;

  std::string m_pluginPath;
  std::string m_pluginExt;
  vector<std::string> m_includePaths;
  vector<std::string> m_libPath;
  std::string m_outputPath;
  std::string m_projectPath;
  std::string m_toolsPath;
  std::string m_symbolsSource;
  std::string m_statsHandler;
  std::string m_dataSource;
  std::string m_symbolsSourceFile;
  std::string m_dataSourcePath;
  std::string m_tradesFile;
  std::string m_statsFile;
  std::string m_outputFile;
  std::string m_dllPath;

  std::string m_tradesCSVFile;
  std::string m_rawTradesCSVFile;
  std::string m_signalsCSVFile;
  std::string m_rawSignalsCSVFile;
  std::string m_statsCSVFile;
  // this is the base file name, various instances take
  // various file extensions: csv, htm, jpg
  std::string m_equityCurveFile;

  unsigned long m_timeout;
  std::string m_zipFile;

  bool m_runSimulator = false;
  std::string m_errorsFile;
  std::string m_toDateTime;
  std::string m_fromDateTime;
  std::string m_startTradesDateTime;
  std::string m_logFile;
  std::string m_endRunSignalFile;
  bool m_asyncRun;

  std::string m_heartBeatFile;
  std::string m_reverseHeartBeatFile;
  std::string m_cancelFile;
  size_t m_symbolTimeout;
  size_t m_reverseHeartBeatPeriod;
  size_t m_heartBeatTimeout;

  PositionSizingParams m_posSizingParams;

  size_t m_cacheSize;

  double m_defSlippageValue;
  double m_defCommissionValue;

  std::string m_defSlippageId;
  std::string m_defCommissionId;

  std::string m_runtimeStatsFile;

  // all the args on the command line without the app name
  std::vector<std::string> m_args;

  size_t m_maxLines;
  size_t m_maxTotalBarCount;
  std::string m_sessionParentPath;
  std::string m_symbolsToChartFile;
  std::string m_chartDescriptionFile;
  std::string m_chartRootPath;

  std::optional< std::string > m_extTriggersFile;
  std::string m_systemNameKeyword;
  std::string m_classNameKeyword;
  std::string m_osPath;
  std::string m_tradesDescriptionFile;
  std::string m_signalsDescriptionFile;
  size_t m_linesPerPage;
  unsigned long m_cpuCount;
  unsigned long m_threads;
  ThreadAlgorithm m_threadAlg;
  std::string m_explicitTradesExt;
  ErrorHandlingMode m_dataErrorHandlingMode;

  std::string m_envPath;
  std::string m_envInclude;
  std::string m_envLib;
  bool m_enableRunAsUser;
};
