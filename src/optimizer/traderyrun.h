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

#include <differential_evolution.hpp>
#include <de_types.hpp>

class TraderyRunException : public std::exception {
 public:
  TraderyRunException(const std::wstring& message)
      : std::exception(message.c_str()) {}
};

class TraderyRun {
  TraderyAuthTokenPtr m_authToken;
  ParametersPtr m_parameters;
  VariablesNamesPtr m_variablesNames;
  TraderySessionEventListener& m_listener;
  tradery::StringPtr m_symbolsList;
  SystemIdsPtr m_systemIds;
  OptionsPtr m_options;
  PositionSizingPtr m_positionSizing;
  const std::wstring m_name;
  tradery::Date m_startTradesDate;

  NullTraderySessionEventListener m_nullListener;

 public:
  TraderyRun(const std::wstring& name, TraderyCredentialsPtr credentials,
             ParametersPtr parameters, VariablesNamesPtr vn,
             TraderySessionEventListener& listener,
             tradery::StringPtr symbolsList, SystemIdsPtr systemIds,
             OptionsPtr options, PositionSizingPtr positionSizing,
             tradery::Date startTradesDate)
      : m_parameters(parameters),
        m_variablesNames(vn),
        m_listener(listener),
        m_authToken(boost::make_shared<TraderyAuthToken>(*credentials)),
        m_symbolsList(symbolsList),
        m_systemIds(systemIds),
        m_options(options),
        m_positionSizing(positionSizing),
        m_name(name),
        m_startTradesDate(startTradesDate) {
    // varables names is allowed to be null, in which case there are no
    // variables and default values are used by the system
    assert(credentials);
    assert(parameters);
    assert(symbolsList);
    assert(systemIds);
    assert(options);
  }

  typedef boost::tuple<PerformanceStatsPtr, ExternalVarsPtr> RunInfo;
  RunInfo run(de::DVectorPtr vars) {
    try {
      //			std::cout << std::endl << _T( "Setting variables
      // for " ) << m_name;

      ExternalVarsPtr ev(
          boost::make_shared<ExternalVars>(m_variablesNames, vars));

      OptimizerTraderySession ts(m_name, m_authToken.get(), m_listener,
                                 m_nullListener);

      ts.start(m_systemIds, m_symbolsList, m_parameters, m_positionSizing, ev,
               m_options, m_startTradesDate);

      ts.waitForThread();

      if (ts.success())
        return RunInfo(ts.performanceStats(), ev);
      else {
        m_listener.errorEvent(new TraderySessionErrorEvent("Session failed"),
                              m_name);
        throw TraderyRunException("Tradery session failed");
      }
    } catch (const ExternalVarsException&) {
      m_listener.errorEvent(
          new TraderySessionErrorEvent("External vars exception"), m_name);
      throw TraderyRunException("External vars error");
    } catch (const StatsException&) {
      m_listener.errorEvent(new TraderySessionErrorEvent("Stats exception"),
                            m_name);
      throw TraderyRunException("Stats error");
    }
  }

  const std::wstring& name() const { return m_name; }
};
