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

#include "traderyrun.h"

class TraderyRunDEObjectiveFunction : public TraderyRun {
 private:
  size_t m_statToOptimize;
  StatsValue::ValueType m_statGroupToOptimize;

 public:
  TraderyRunDEObjectiveFunction(
      const std::wstring& name, TraderyCredentialsPtr credentials,
      ParametersPtr parameters, VariablesNamesPtr vn,
      TraderySessionEventListener& listener, tradery::StringPtr symbolsList,
      SystemIdsPtr systemIds, OptionsPtr options,
      PositionSizingPtr positionSizing, size_t statToOptimize,
      StatsValue::ValueType statGroupToOptimize, tradery::Date startTradesDate)
      : TraderyRun(name, credentials, parameters, vn, listener, symbolsList,
                   systemIds, options, positionSizing, startTradesDate),
        m_statToOptimize(statToOptimize),
        m_statGroupToOptimize(statGroupToOptimize) {}

  virtual double operator()(de::DVectorPtr vars) {
    try {
      TraderyRun::RunInfo runInfo(TraderyRun::run(vars));

      PerformanceStatsPtr stats(runInfo.get<0>());
      ExternalVarsPtr ev(runInfo.get<1>());
      assert(stats);
      assert(ev);

      double value(
          stats->getStatValue(m_statToOptimize, m_statGroupToOptimize));
      std::cout << std::endl
                << "[" << TraderyRun::name() << "] " << value
                << ", vars: " << (*ev->toString());
      return value;

    } catch (const TraderyRunException& e) {
      throw de::objective_function_exception(e.what());
    }
  }
};

typedef boost::shared_ptr<TraderyRunDEObjectiveFunction>
    TraderyRunDEObjectiveFunctionPtr;

class TraderyRunDEObjectiveFunctionFactory
    : public de::objective_function_factory<TraderyRunDEObjectiveFunction> {
 private:
  TraderyCredentialsPtr m_credentials;
  ParametersPtr m_parameters;
  VariablesNamesPtr m_variablesNames;
  TraderySessionEventListener& m_listener;
  tradery::StringPtr m_symbolsList;
  SystemIdsPtr m_systemIds;
  OptionsPtr m_options;
  PositionSizingPtr m_positionSizing;
  tradery::Date m_startTradesDate;
  size_t m_statToOptimize;
  StatsValue::ValueType m_statGroupToOptimize;
  std::wstring m_diagName;

 public:
  TraderyRunDEObjectiveFunctionFactory(
      const std::wstring& diagName, TraderyCredentialsPtr credentials,
      ParametersPtr parameters, VariablesNamesPtr vn,
      TraderySessionEventListener& listener, tradery::StringPtr symbolsList,
      SystemIdsPtr systemIds, OptionsPtr options,
      PositionSizingPtr positionSizing, size_t statToOptimize,
      StatsValue::ValueType statGroupToOptimize, tradery::Date startTradesDate)
      : m_credentials(credentials),
        m_parameters(parameters),
        m_variablesNames(vn),
        m_listener(listener),
        m_symbolsList(symbolsList),
        m_systemIds(systemIds),
        m_options(options),
        m_positionSizing(positionSizing),
        m_startTradesDate(startTradesDate),
        m_statToOptimize(statToOptimize),
        m_statGroupToOptimize(statGroupToOptimize),
        m_diagName(diagName) {
    assert(credentials);
  }

  TraderyRunDEObjectiveFunctionPtr make() {
    // will create a new token for each instance (for parallel runs the token
    // must be distinct)
    return TraderyRunDEObjectiveFunctionPtr(new TraderyRunDEObjectiveFunction(
        m_diagName, boost::make_shared<TraderyAuthToken>(*m_credentials),
        m_parameters, m_variablesNames, m_listener, m_symbolsList, m_systemIds,
        m_options, m_positionSizing, m_statToOptimize, m_statGroupToOptimize,
        m_startTradesDate));
  }
};

typedef boost::shared_ptr<TraderyRunDEObjectiveFunctionFactory>
    TraderyRunDEObjectiveFunctionFactoryPtr;
