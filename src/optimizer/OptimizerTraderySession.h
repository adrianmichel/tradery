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

#include <traderysession.h>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <stringformat.h>
#include <json/json.h>
#include <datasource.h>
#include <params.h>
#include <tokenizer.h>
#include <statsdefines.h>
#include "variables.h"

#include <de_types.hpp>

class Parameters {
 private:
  tradery::Date m_from;
  tradery::Date m_to;

  double m_commission;
  double m_slippage;

 public:
  Parameters(tradery::Date& from, tradery::Date& to, double commission,
             double slippage)
      : m_from(from), m_to(to), m_commission(commission), m_slippage(slippage) {
    assert(from <= to);
    assert(commission >= 0);
    assert(slippage >= 0);
    assert(slippage <= 100);
  }

  tradery::StringPtr toJsonString() const {
    Json::Value json;

    json["fromDate"] = m_from.toString(us);
    json["toDate"] = m_to.toString(us);
    json["commissionValue"] = m_commission;
    json["slippageValue"] = m_slippage;

    Json::StyledWriter writer;

    return tradery::StringPtr(new std::wstring(writer.write(json)));
  }
};
typedef boost::shared_ptr<Parameters> ParametersPtr;

class PositionSizing : public PositionSizingParams {
 public:
  PositionSizing() : PositionSizingParams() {}
  PositionSizing(double initialCapital, const MaxOpenPositions& maxOpenPos,
                 PosSizeType posSizeType, double posSize,
                 PosSizeLimitType posSizeLimitType, double posSizeLimit)
      : PositionSizingParams(initialCapital, maxOpenPos, posSizeType, posSize,
                             posSizeLimitType, posSizeLimit) {}

  tradery::StringPtr toJsonString() const {
    Json::Value json;

    json["initialCapital"] = __super::initialCapital();
    json["maxOpenPos"] = __super::maxOpenPos().getValue();
    json["posSizeType"] = __super::posSizeType();
    json["posSizeValue"] = __super::posSize();
    json["posSizeLimitType"] = __super::posSizeLimitType();
    json["posSizeLimitValue"] = __super::posSizeLimit();

    Json::StyledWriter writer;

    return tradery::StringPtr(new std::wstring(writer.write(json)));
  }
};
typedef boost::shared_ptr<PositionSizing> PositionSizingPtr;

class Options {
  bool m_charts;
  bool m_stats;
  bool m_equity;
  bool m_trades;
  bool m_signals;
  bool m_output;
  ErrorHandlingMode m_dataErrorHandling;

 public:
  Options(bool charts, bool stats, bool equity, bool trades, bool signals,
          bool output, ErrorHandlingMode dataErrorHandling)
      : m_charts(charts),
        m_stats(stats),
        m_equity(equity),
        m_trades(trades),
        m_signals(signals),
        m_output(output),
        m_dataErrorHandling(dataErrorHandling) {}

  tradery::StringPtr toJsonString() const {
    Json::Value json;

    json["charts"] = m_charts;
    json["stats"] = m_stats;
    json["equity"] = m_equity;
    json["trades"] = m_trades;
    json["signals"] = m_signals;
    json["output"] = m_output;
    json["dataErrorHandling"] = m_dataErrorHandling;

    Json::StyledWriter writer;

    return tradery::StringPtr(new std::wstring(writer.write(json)));
  }
};
typedef boost::shared_ptr<Options> OptionsPtr;

class StatsException {};

class StatsValue {
 private:
  std::wstring m_name;
  double m_total;
  double m_long;
  double m_short;
  double m_bh;

 public:
  enum ValueType { total_value, long_value, short_value, bh_value };

  StatsValue(const std::wstring& csv) {
    Tokenizer fields(csv, ",");

    if (fields.size() == 5) {
      m_name = fields[0];
      m_total = strtod(fields[1].c_str(), 0);
      m_long = strtod(fields[2].c_str(), 0);
      m_short = strtod(fields[3].c_str(), 0);
      m_bh = strtod(fields[4].c_str(), 0);
    }
  }

  StatsValue(const std::wstring& name, double total, double _long,
             double _short, double bh)
      : m_name(name),
        m_total(total),
        m_long(_long),
        m_short(_short),
        m_bh(bh) {}

  const std::wstring& name() const { return m_name; }
  double totalValue() const { return m_total; }
  double longValue() const { return m_long; }
  double shortValue() const { return m_short; }
  double bhValue() const { return m_bh; }

  double value(ValueType type) {
    switch (type) {
      case total_value:
        return totalValue();
      case long_value:
        return longValue();
      case short_value:
        return shortValue();
      case bh_value:
        return bhValue();
      default:
        throw StatsException();
    }
  }

  std::wstring toString() const {
    return tradery::format("%1%: %2%, %3%, %4%, %5%", m_name, totalValue(),
                           longValue(), shortValue(), bhValue());
  }
};

typedef boost::shared_ptr<StatsValue> StatsValuePtr;

typedef std::map<std::wstring, StatsValuePtr> PerformanceStatsBase;
// performance statistics

class PerformanceStats : public PerformanceStatsBase {
 private:
  static char* StatsX[];

 public:
  PerformanceStats(JsonValuePtr json) {
    std::wstring stats(json->asString());

    Tokenizer lines(stats, "\n\r");

    for (Tokenizer::size_type n = 0; n < lines.size(); ++n) {
      const std::wstring& line(lines[n]);

      StatsValuePtr sv(boost::make_shared<StatsValue>(line));

      __super::insert(PerformanceStatsBase::value_type(sv->name(), sv));
    }
  }

  tradery::StringPtr toString() const {
    tradery::StringPtr str(new std::wstring());

    BOOST_FOREACH (const PerformanceStatsBase::value_type& sv, *this) {
      *str += sv.second->toString() + "\n";
    }

    return str;
  }

  double getValue(const std::wstring& valueName,
                  StatsValue::ValueType type) const {
    PerformanceStatsBase::const_iterator i(__super::find(valueName));

    if (i != __super::end()) {
      return i->second->value(type);
    } else
      throw StatsException();
  }

  double getStatValue(size_t index, StatsValue::ValueType type) const;

  double getTraderyScore(StatsValue::ValueType type) const {
    return getValue(TRADERY_SCORE, type);
  }

  double getTotalPctGainLoss(StatsValue::ValueType type) const {
    return getValue(TOTAL_PCT_GAIN_LOSS, type);
  }

  double getTotalGainLoss(StatsValue::ValueType type) const {
    return getValue(TOTAL_GAIN_LOSS, type);
  }

  double getAnnualizedPctGain(StatsValue::ValueType type) const {
    return getValue(ANNUALIZED_PCT_GAIN, type);
  }

  double getExposure(StatsValue::ValueType type) const {
    return getValue(EXPOSURE, type);
  }

  double getExpectancy(StatsValue::ValueType type) const {
    return getValue(EXPECTANCY, type);
  }

  double getUlcerIndex(StatsValue::ValueType type) const {
    return getValue(ULCER_INDEX, type);
  }

  double getMaxDrawdown(StatsValue::ValueType type) const {
    return getValue(MAX_DRAWDOWN, type);
  }

  double getMaxDrawdownPct(StatsValue::ValueType type) const {
    return getValue(MAX_DRAWDOWN_PCT, type);
  }

  double getMaxDrawdownDuration(StatsValue::ValueType type) const {
    return getValue(MAX_DRAWDOWN_DURATION, type);
  }
};

typedef boost::shared_ptr<PerformanceStats> PerformanceStatsPtr;

class SystemIds : public tradery::StrVector {
 public:
  tradery::StringPtr toJsonString() const {
    Json::Value json;

    for (StrVector::size_type i = 0; i < __super::size(); ++i)
      json.append(__super::at(i));

    Json::StyledWriter writer;

    return tradery::StringPtr(new std::wstring(writer.write(json)));
  }

  const SystemIds& operator=(const std::vector<std::wstring>& systems) {
    if (this != &systems) __super::operator=(systems);

    return *this;
  }
};
typedef boost::shared_ptr<SystemIds> SystemIdsPtr;

class OptimizerTraderySession : public TraderySession {
  SystemIdsPtr m_systems;
  tradery::StringPtr m_symbolsList;
  ParametersPtr m_parameters;
  PositionSizingPtr m_positionSizing;
  ExternalVarsPtr m_externalVars;
  OptionsPtr m_options;

  PerformanceStatsPtr m_stats;
  tradery::Date m_startTradesDate;

 public:
  OptimizerTraderySession(const std::wstring& name, TraderyAuthToken* authToken,
                          TraderySessionEventListener& listener,
                          TraderySessionEventListener& listener1)
      : TraderySession(name, authToken, listener, listener1) {}

  void start(SystemIdsPtr systems, tradery::StringPtr symbolsList,
             ParametersPtr parameters, const PositionSizingPtr positionSizing,
             ExternalVarsPtr externalVars, OptionsPtr options,
             tradery::Date startTradesDate) {
    m_systems = systems;
    m_symbolsList = symbolsList;
    m_parameters = parameters;
    m_positionSizing = positionSizing;
    m_externalVars = externalVars;
    m_options = options;
    m_startTradesDate = startTradesDate;

    __super::start();
  }

#define MAX_RETRIES 4

  ~OptimizerTraderySession(void);

  virtual void run(ThreadContext*) {
    try {
      for (size_t retries = 0;;) {
        try {
          LOG_ENTRY_EXIT(log_debug, "");

          __super::listeners().startOfSession(__super::name());
          TraderyConnection traderyConnection(*__super::authToken());

          runy(traderyConnection);
          return;
        } catch (const TraderySessionException& e) {
          if (retries >= MAX_RETRIES)
            throw;
          else
            __super::listeners().retryingSession(++retries, e.message(),
                                                 __super::name());
        }
      }
    } catch (const TraderyConnectionException& e) {
      LOG(log_debug, _T( "" ));
      __super::setFailed();
      listeners().errorEvent(new TraderySessionErrorEvent(e.message()),
                             __super::name());
      listeners().sessionFailed();
    } catch (const TraderySessionException& e) {
      LOG(log_debug, _T( "" ));
      __super::setFailed();
      listeners().errorEvent(new TraderySessionErrorEvent(e.message()),
                             __super::name());
      listeners().sessionFailed();
    } catch (std::runtime_error& e) {
      LOG(log_debug, _T( "" ));
      __super::setFailed();
      listeners().errorEvent(
          new TraderySessionErrorEvent(
              tradery::format("Invalid start session response: %1%", e.what())),
          __super::name());
      listeners().sessionFailed();
    } catch (...) {
      LOG(log_debug, _T( "" ));
      __super::setFailed();
      listeners().errorEvent(new TraderySessionErrorEvent("Unknown error"),
                             __super::name());
      listeners().sessionFailed();
    }
  }

  void runy(TraderyConnection& traderyConnection) {
    LOG_ENTRY_EXIT(log_debug, "");
    // first start the session

    tradery::StringPtr externalVariables(
        m_externalVars ? m_externalVars->toJsonString()
                       : tradery::StringPtr(new std::wstring()));

    RunSystemsAPI rsAPI(
        m_systems->toJsonString(), m_symbolsList, m_parameters->toJsonString(),
        m_positionSizing->toJsonString(), externalVariables,
        m_options->toJsonString(), m_startTradesDate.toString(us));

    //		std::cout << rsAPI.toString();

    Json::Value startResult(
        (*traderyConnection.request(rsAPI))["systemsStart"]);

#pragma message( \
    __TODO__     \
    "show message from server (have to send it first) if error staring session ")
    if (!startResult.isNull()) {
      bool result = startResult.asInt();

      if (result) {
        // handle start session result

        maintainRun(traderyConnection);

        m_stats = boost::make_shared<PerformanceStats>(
            __super::getStats(traderyConnection));

        listeners().endOfsession(traderyConnection.responseTimeStamp());

        __super::setSuccess();
      } else {
        LOG(log_debug, _T( "session error: failed to start session" ));
        __super::setFailed();
        listeners().errorEvent(
            new TraderySessionErrorEvent("Failed to start session"));
        listeners().sessionFailed();
      }
    } else {
      LOG(log_debug, _T( "session error: invalid session response" ));
      __super::setFailed();
      listeners().errorEvent(
          new TraderySessionErrorEvent("Invalid start session response"));
      listeners().sessionFailed();
    }
  }

  PerformanceStatsPtr performanceStats() const { return m_stats; }
};

typedef boost::shared_ptr<OptimizerTraderySession> OptimizerTraderySessionPtr;

class OptimizerTraderySessionEventListener
    : public TraderySessionEventListener {
  OBJ_COUNTER(OptimizerTraderySessionEventListener)
 public:
  virtual ~OptimizerTraderySessionEventListener() {}

  virtual void startOfSession(const std::wstring& name = std::wstring()) {
    // std::cout << "Start of session: " << name << std::endl;
    std::cout << ".";
  }

  virtual void statsEvent(TraderySessionStatsPtr stats,
                          const std::wstring& name = std::wstring()) {
    //	  std::cout << tradery::format( "%1%, %2% - %3%", name,
    // stats->percentage(), stats->status() ) << std::endl;
    std::cout << ".";
  }

  virtual void endOfsession(unsigned __int64 timeStamp,
                            const std::wstring& name = std::wstring()) {
    //	  std::cout << "end of session: " << name << std::endl;
    std::cout << ".";
  }

  virtual void sessionCanceled(const std::wstring& name = std::wstring()) {
    std::cout << "session canceled: " << name << std::endl;
  }

  virtual void errorEvent(TraderySessionErrorEventPtr error,
                          const std::wstring& name = std::wstring()) {
    std::cout << name << "error event: " << error->error() << std::endl;
  }

  virtual void sessionFailed(const std::wstring& name = std::wstring()) {
    std::cout << "Session failed: " << name << std::endl;
  }

  virtual void retryingSession(size_t n, const std::wstring message,
                               const std::wstring& name = std::wstring()) {
    std::cout << tradery::format("Retrying session (%1%): %2%, error: %3%", n,
                                 name, message)
              << std::endl;
  }
};
