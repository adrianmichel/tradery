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

#include "propertieslist.h"
#include <positionsizingparams.h>
#include "wchart.h"
#include <tokenizer.h>
#include <explicittrades.h>
#include "Configuration.h"
#include <runtimeparams.h>

class DocumentException {
 private:
  const std::string _message;

 public:
  DocumentException(const std::string& message) : _message(message) {}

  const std::string& message() const { return _message; }
};

class Document {
private:
  const std::string _name;
  const UniqueIdPtr _defDataSource;
  const UniqueIdPtr _defSymbolsSource;
  const UniqueIdPtr _defStatsHandler;
  const UniqueIdPtr _defSlippage;
  const UniqueIdPtr _defCommission;
  UniqueIdVector _runnables;
  mutable size_t _runnablesIterator;
  RuntimeParams _runtimeParams;
  std::vector<std::string> _symbolsSourceStrings;
  std::vector<std::string> _dataSourceStrings;
  std::vector<std::string> _statsHandlerStrings;
  std::vector<std::string> _slippageStrings;
  std::vector<std::string> _commissionStrings;
  using ExplicitTradesMap = std::map<UniqueId, ExplicitTradesPtr>;
  ExplicitTradesMap _explicitTrades;

  const std::string _sessionPath;
  PluginTree _sessionPluginTree;

  ChartManagerPtr _chartManager;
  const UniqueId _sessionId;

public:
  Document(const Configuration& config) try
    : _runnablesIterator(0),
    _runnables(config.getRunnableIds()),
    _sessionPath(config.sessionParentPath()),
    _defDataSource(std::make_shared< UniqueId >(config.dataSource())),
    _defSymbolsSource(std::make_shared< UniqueId >(config.symbolsSource())),
    _defStatsHandler(config.hasStatsHandler() ? std::make_shared< UniqueId >(config.statsHandler()) : nullptr ),
    _defSlippage(config.defSlippageValue() == 0 ? nullptr : std::make_shared< UniqueId >(config.defSlippageId())),
    _defCommission(config.defCommissionValue() == 0 ? nullptr : std::make_shared< UniqueId >(config.defCommissionId())),
    _chartManager(std::make_shared< WebChartManager >("", config.symbolsToChartFile(), config.chartRootPath(),
      config.chartDescriptionFile(), config.getRunnableIds().size() > 1 /*multi system has reduced charts*/)),
    _sessionId(config.getSessionId()) {
    try {
      _sessionPluginTree.explore(config.getSessionPath(), config.getPluginExt(), false, 0);
      LOG(log_debug, "Run system after explore");

      DateTime _from(isBlanc(config.fromDateTime()) ? NegInfinityDateTime() : DateTime(Date(config.fromDateTime(), us)));
      DateTime _to(isBlanc(config.toDateTime()) ? PosInfinityDateTime() : DateTime(Date(config.toDateTime(), us)));
      DateTime _startTradesDateTime(isBlanc(config.startTradesDateTime()) ? NotADateTime() : DateTime(Date(config.startTradesDateTime(), us)));
      _runtimeParams.setPositionSizingParams(config.positionSizingParams());
      _runtimeParams.setChartsEnabled(config.generateCharts());
      _runtimeParams.setStatsEnabled(config.generateStats());
      _runtimeParams.setEquityEnabled(config.generateEquityCurve());
      _runtimeParams.setTradesEnabled(config.generateTrades());
      _runtimeParams.setThreads(config.getThreads());
      _runtimeParams.setThreadAlgorithm(config.getThreadAlg());

      try {
        _runtimeParams.setRange(std::make_shared<DateTimeRange>(_from, _to));
      }
      catch (const DateTimeRangeException&) {
        std::string message = tradery::format("Invalid date/time range - \"From\" must occur before \"To\": "
          , _from.to_simple_string(), " - ", _to.to_simple_string());
        LOG(log_error, message);
        throw DocumentException(message);
      }

      //TODO don't rely on arbitrary indexes to comminicate data to plugin,
      // at the very least use some sort of named variables
      _statsHandlerStrings.push_back(config.statsCSVFile());                   // index 0
      _statsHandlerStrings.push_back(config.statsFile());                      // index 1
      _statsHandlerStrings.push_back(config.equityCurveFile());                // index 2
      _statsHandlerStrings.push_back(config.signalsCSVFile());                 // index 3
      _statsHandlerStrings.push_back(config.getSignalsDescriptionFile());       // index 4
      _statsHandlerStrings.push_back(std::to_string(config.getLinesPerPage())); // index 5
      _statsHandlerStrings.push_back(config.rawSignalsCSVFile());               // index 6
      _statsHandlerStrings.push_back(config.getSessionId());                    // index 7

      _symbolsSourceStrings.push_back( config.symbolsSourceFile());
      _dataSourceStrings.push_back(config.dataSourcePath());
      _dataSourceStrings.push_back(errorHandlingModeAsString( (tradery::ErrorHandlingMode)config.dataErrorHandlingMode()));
      _slippageStrings.push_back(std::to_string(config.defSlippageValue()));
      _commissionStrings.push_back(std::to_string(config.defCommissionValue()));

      // creating explicit trades
      // each runnable has its own explicit trades file
      // to allow for multi-system sessions with different explicit trades per
      // system the name of the file is implicit: the runnable id + the
      // extension
      for (auto id : config.getRunnableIds()) {
        std::string explicitTradesFile = _sessionPath + "\\" + id.str() + "." + config.explicitTradesExt();

        if (Path{ explicitTradesFile }.exists()) {
          _explicitTrades.insert(ExplicitTradesMap::value_type(id, ExplicitTradesPtr(new FileExplicitTrades(explicitTradesFile))));
        }
      }
      LOG(log_info, "exiting constructor");
    }
    catch (const DateException & e) {
      LOG(log_error, "DateException: ", e.message());
      throw DocumentException(e.message());
    }
  }
  catch (const ExplicitTradesException & e) {
    LOG(log_error, "TriggersException");
    throw DocumentException(e.message());
  }

  virtual const ExplicitTrades* getExplicitTrades(const UniqueId& id) const {
    ExplicitTradesMap::const_iterator i = _explicitTrades.find(id);

    return i != _explicitTrades.end() ? i->second.get() : 0;
  }

  virtual const std::string name() { return _name; }

  virtual const UniqueId* getNextRunnableId() const {
    return _runnablesIterator < _runnables.size() ? &_runnables[_runnablesIterator++] : 0;
  }

  virtual bool hasDefaultCommission() const {
    return _defCommission.get() != 0;
  }
  virtual const UniqueId* getDefaultDataSourceId() const {
    return _defDataSource.get();
  }

  virtual const UniqueId* getDefaultCommissionId() const {
    return _defCommission.get();
  }
  virtual bool hasDefaultDataSource() const {
    return _defDataSource.get() != 0;
  }
  virtual bool hasDefaultSignalHandler() const {
    return _defStatsHandler.get() != 0;
  }
  virtual const UniqueId* getDefaultSignalHandlerId() const {
    return _defStatsHandler.get();
  }
  virtual RuntimeParams& getRuntimeParams() { return _runtimeParams; }
  virtual bool hasDefaultSlippage() const {
    return _defSlippage.get() != 0;
  }
  virtual const UniqueId* getFirstRunnableId() const {
    _runnablesIterator = 0;
    return getNextRunnableId();
  }
  virtual const UniqueId* getDefaultSlippageId() const {
    return _defSlippage.get();
  }
  virtual bool hasDefaultSymbolsSource() const {
    return _defSymbolsSource.get() != 0;
  }
  virtual size_t runnablesCount() const { return _runnables.size(); }
  virtual const UniqueId* getDefaultSymbolsSourceId() const {
    return _defSymbolsSource.get();
  }

  virtual const std::vector<std::string>* defaultSymbolsSourceStrings() const {
    return &_symbolsSourceStrings;
  }

  virtual const std::vector<std::string>* defaultDataSourceStrings() const {
    return &_dataSourceStrings;
  }

  virtual const std::vector<std::string>* defaultSignalHandlerStrings() const {
    return &_statsHandlerStrings;
  }

  virtual const std::vector<std::string>* defaultSlippageStrings() const {
    return &_slippageStrings;
  }

  virtual const std::vector<std::string>* defaultCommissionStrings() const {
    return &_commissionStrings;
  }

  virtual const std::vector<std::string>* getRunnablesStrings() const {
    return 0;
  }

  virtual const PositionSizingParams* positionSizingParams() const {
    return _runtimeParams.positionSizing();
  }

  virtual ChartManager* chartManager() {
    assert(_chartManager.get());
    return _chartManager.get();
  }

  virtual const PluginTree& getSessionPluginTree() const {
    return _sessionPluginTree;
  }
  virtual PluginTree& getSessionPluginTree() { return _sessionPluginTree; }
  virtual const UniqueId& getSessionId() const { return _sessionId; }
};
