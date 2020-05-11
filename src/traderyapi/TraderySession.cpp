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

#include "stdafx.h"
#include ".\traderysession.h"
#include <stringformat.h>

void TraderySessionStatsEvent::init() {
  __super::push_back(tradery::NameValue("Date range", ""));
  __super::push_back(tradery::NameValue("Duration", ""));
  __super::push_back(tradery::NameValue("Symbols processed", "0"));
  __super::push_back(tradery::NameValue("Symbols processed with errors", "0"));
  __super::push_back(tradery::NameValue("Total symbols", "0"));
  __super::push_back(tradery::NameValue("Raw trades", "0"));
  __super::push_back(tradery::NameValue("Processed trades", "0"));
  __super::push_back(tradery::NameValue("Raw signals", "0"));
  __super::push_back(tradery::NameValue("Processed signals", "0"));
  m_percentage = 0;
  m_status = "Preparing session. Please wait...";
}

TraderySessionStatsEvent::TraderySessionStatsEvent() { init(); }

TraderySessionStatsEvent::TraderySessionStatsEvent(const Json::Value& stats) {
  Json::Value v = stats["DataAvailable"];

  m_dataAvailable = v.asBool();

  if (m_dataAvailable) {
    __super::push_back(
        tradery::NameValue("Date range", stats["DateRange"].asString()));
    __super::push_back(
        tradery::NameValue("Duration", stats["Duration"].asString()));
    __super::push_back(tradery::NameValue(
        "Symbols processed", stats["SymbolsProcessed"].asString()));
    __super::push_back(
        tradery::NameValue("Symbols processed with errors",
                           stats["SymbolsProcessedWithErrors"].asString()));
    __super::push_back(
        tradery::NameValue("Total symbols", stats["TotalSymbols"].asString()));
    __super::push_back(
        tradery::NameValue("Raw trades", stats["RawTrades"].asString()));
    __super::push_back(tradery::NameValue("Processed trades",
                                          stats["ProcessedTrades"].asString()));
    __super::push_back(
        tradery::NameValue("Raw signals", stats["RawSignals"].asString()));
    __super::push_back(tradery::NameValue(
        "Processed signals", stats["ProcessedSignals"].asString()));

    std::wstring x;
    std::istringstream is;

    is.clear();
    is.str(stats["Percentage"].asString());
    is >> m_percentage;
    m_status = stats["Status"].asString();
    if (m_status.empty())
      m_status = "Session in progress";
    else {
      boost::regex pattern("\\[/?b\\]");

      m_status = boost::regex_replace(m_status, pattern, "");
    }
  }
}

JsonValuePtr TraderySession::getStats(TraderyConnection& traderyConnection) {
  LOG_ENTRY_EXIT(log_debug, "");
  try {
    // get runtime stats every sec
    JsonValuePtr stats(new Json::Value(
        (*traderyConnection.request(PerformanceStatsAPI()))["sessionstats"]));

    if (!stats->isNull())
      return stats;
    else
      throw TraderySessionException("Invalid performance stats response");
  } catch (const TraderyConnectionException& e) {
    throw TraderySessionException(e.message());
  } catch (const std::runtime_error& e) {
    // this is most likely a json exception
    throw TraderySessionException(tradery::format(
        "Invalid performance stats response, error: %1%", e.what()));
  }
}

JsonValuePtr TraderySession::getSignals(TraderyConnection& traderyConnection,
                                        const std::wstring& id) {
  LOG_ENTRY_EXIT(log_debug, "");
  try {
    // get runtime stats every sec
    JsonValuePtr signals(new Json::Value(
        (*traderyConnection.request(SignalsAPI(id)))["signals"]));

    if (!signals->isNull())
      return signals;
    else
      throw TraderySessionException("Invalid signals response");
  } catch (const TraderyConnectionException& e) {
    throw TraderySessionException(e.message());
  } catch (const std::runtime_error& e) {
    // this is most likely a json exception
    throw TraderySessionException(
        tradery::format("Invalid signals response, error: %1%", e.what()));
  }
}

TraderySessionStatsPtr TraderySession::getRuntimeStats(
    TraderyConnection& traderyConnection) {
  LOG_ENTRY_EXIT(log_debug, "");
  try {
    // get runtime stats every sec
    Json::Value stats(
        (*traderyConnection.request(RuntimeStatsAPI()))["rtstats"]);

    if (!stats.isNull()) {
      return new TraderySessionStatsEvent(stats);
    } else
      throw TraderySessionException("Invalid runtime stats response");
  } catch (const TraderyConnectionException& e) {
    throw TraderySessionException(e.message());
  } catch (const std::runtime_error& e) {
    throw TraderySessionException(tradery::format(
        "Invalid runtime stats response, error: %1%", e.what()));
  }
}

TraderySession::HeartBeatResult TraderySession::heartBeat(
    TraderyConnection& traderyConnection) {
  LOG_ENTRY_EXIT(log_debug, "");
  try {
    Json::Value json(
        (*traderyConnection.request(HeartBeatAPI()))["sessionstatus"]);

    if (!json.isNull()) {
      std::wstring response(json.asString());

      // return false means not done yet (the current hb is ok, but the session
      // hasn't finished) true means the session has finished succesfully,
      if (response == "ok")
        return HeartBeatResult(false, traderyConnection.responseTimeStamp());
      if (response == "finished")
        return HeartBeatResult(true, traderyConnection.responseTimeStamp());
      else if (response == "noresponse")
        throw TraderySessionException("No response from the trading engine");
      else
        throw TraderySessionException(std::wstring("Unknown response: ")
                                      << response << " in session heartbeat");
    } else
      throw TraderySessionException("Invalid heartbeat response");
  } catch (const TraderyConnectionException& e) {
    throw TraderySessionException(e.message());
  } catch (const std::runtime_error& e) {
    throw TraderySessionException(
        tradery::format("Invalid heartbeat response, error: %1%", e.what()));
  }
}

void TraderySession::start() {
  LOG_ENTRY_EXIT(log_debug, "");

  m_canceling = false;

  __super::start();
}

JsonValuePtr TraderySession::getActiveEntrySignals(
    TraderyConnection& traderyConnection, const std::wstring& id) {
  LOG_ENTRY_EXIT(log_debug, "");
  try {
    // get runtime stats every sec
    JsonValuePtr activeSignals(new Json::Value(
        (*traderyConnection.request(GetActiveEntrySignalsAPI(id)))["signals"]));

    if (!activeSignals->isNull()) {
      return activeSignals;
    } else
      throw TraderySessionException(
          "Invalid active entry signals response response");
  } catch (const TraderyConnectionException& e) {
    throw TraderySessionException(e.message());
  } catch (const std::runtime_error& e) {
    throw TraderySessionException(tradery::format(
        "Invalid active entry signals response, error: %1%", e.what()));
  }
}

void TraderySession::stop() {
  LOG_ENTRY_EXIT(log_debug, "");
  m_canceling = true;
  __super::stopSync();
}

void TraderySession::cancelSession(TraderyConnection& traderyConnection) {
  LOG_ENTRY_EXIT(log_debug, "");
  try {
    // cancel the session
    JsonValuePtr cancel(
        new Json::Value((*traderyConnection.request(CancelSession()))));
#pragma message(__TODO__ "handle the cancel session response")
  } catch (const TraderyConnectionException& e) {
    throw TraderySessionException(e.message());
  } catch (const std::runtime_error& e) {
    throw TraderySessionException(
        tradery::format("Invalid cancel response, error: %1%", e.what()));
  }
}

/* a tradery session:

1. Determine open positions generated by the current session
        a. Get active signals from tradery (entry signals with order ids,
meaning they have been submitted, but which don't have matching active exit
signals, meaning with an order id, and not canceled)' b. Check the status of the
corresponding order (shares filled, if any, time stamp if filled). c. check the
status of an existing position with the same symbol (to make sure the user
hasn't closed some of the shares manually for example)' d. The corresponding
active position, that will be sent back: min of shares filled by order and the
existing position size of same symbol. Make sure to take into account multiple
fills for the amount and also for the entry date/time.
2. Get account info ( available cash, buying power etc)
3. Send open pos and account info to tradery
4. Start session
5. If succesful, get signals from tradery and put them in the queue
6. As signals get submitted, update the signal status in tradery with teh
corresponding order id
*/

/*
                // this will generate the current account data to be posted
                Json::Value json;

                JsonFormatter f( json );

                balancesAndPositions->format( f );

                tradery::StringPtr str( f.write() );

                assert( str );
*/

void TraderySession::maintainRun(TraderyConnection& traderyConnection) {
  // now do the heartbeat
  while (true) {
    LOG(log_debug, _T( "in the session loop" ));

    for (int n = 0; n < 5; ++n) {
      if (m_canceling) {
        cancelSession(traderyConnection);
        _listeners.sessionCanceled(m_name);
#pragma message( \
    __TODO__     \
    "Cancel now doesn't wait for the server response that the session has been actually canceled. It may make more sense to wait for that response before showing the session was canceled on the client side")
        m_result = false;
        return;
      }
      Sleep(200);

      TraderySessionStatsPtr stats(getRuntimeStats(traderyConnection));

      _listeners.statsEvent(stats, m_name);
    }

    LOG(log_debug, _T( "sending heartbeat" ));

    HeartBeatResult result(heartBeat(traderyConnection));
    if (result) {
      LOG(log_debug,
          _T( "heartbeat returned true (session finished succesfully), breaking from the session loop" ));
      break;
    }
  }
}
