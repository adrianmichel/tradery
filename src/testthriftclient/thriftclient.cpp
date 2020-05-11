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

#define BOOST_TEST_MODULE test tradery service thrift api
//#define BOOST_TEST_NO_MAIN

#include <boost/test/included/unit_test.hpp>

#include <Tradery.h>

#include "resource.h"
#include <resourcewrapper.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::boost::uuids;

using namespace std;
namespace utf = boost::unit_test;

typedef tradery_thrift_api::System ThriftSystem;

BOOST_AUTO_TEST_CASE(test1) {
  TextResource system1(IDR_SYSTEM1);
  TextResource symbols(IDR_SYMBOLS);

  boost::shared_ptr<TTransport> socket(new TSocket("localhost", 9091));
  boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  tradery_thrift_api::TraderyClient client(protocol);

  try {
    transport->open();

    tradery_thrift_api::SessionParams sessionParams;

    ThriftSystem s1;
    s1.code = system1;
    s1.name = "system1";
    s1.description = "system1_description1";
    s1.dbId = "dbid1";

    ThriftSystem s2;
    s2.code = system1;
    s2.name = "system2";
    s2.description = "system1_description2";
    s2.dbId = "dbid2";

    sessionParams.systems.push_back(s1);
    //		sessionContext.systems.push_back(s2);

    boost::split(sessionParams.symbols, std::string(symbols.get()),
                 boost::is_any_of(" \t\r\n"));

    sessionParams.generateTrades = true;
    sessionParams.generateStats = true;
    sessionParams.generateEquityCurve = true;
    sessionParams.generateSignals = true;
    sessionParams.generateOutput = true;
    sessionParams.generateCharts = true;

    sessionParams.range.startDate =
        (tradery::DateTime(tradery::Date("1/1/2017")) - tradery::Days(1000))
            .to_epoch_time();
    sessionParams.range.endDate = tradery::LocalTimeSec().to_epoch_time();

    sessionParams.dataErrorHandling =
        tradery_thrift_api::DataErrorHandling::WARNING;

    sessionParams.commission = 10;
    sessionParams.slippage = 0.5;

    sessionParams.positionSizing.initialCapital = 50000;
    sessionParams.positionSizing.maxOpenPositions = 2;
    sessionParams.positionSizing.positionSizeType =
        tradery_thrift_api::PositionSizeType::type::SHARES;
    sessionParams.positionSizing.positionSize = 100;
    sessionParams.positionSizing.positionSizeLimitType =
        tradery_thrift_api::PositionSizeLimitType::type::PCT_VOL;
    sessionParams.positionSizing.positionSizeLimit = 0.1;

    //		sessionContext.positionSizing.positionSize = 1000;

    std::vector<tradery_thrift_api::ID> sessionIds;

    for (int n = 0; n < 4; ++n) {
      tradery_thrift_api::ID sessionId;
      client.startSession(sessionId, sessionParams);

      sessionIds.push_back(sessionId);

      std::cout << "session id: " << sessionId << std::endl;
    }

    // waiting for all sessions to be done
    tradery_thrift_api::RuntimeStats rs;

    bool done = true;
    do {
      done = true;
      for (size_t n = 0; n < sessionIds.size(); ++n) {
        client.getRuntimeStats(rs, sessionIds[n]);
        std::cout << sessionIds[n] << ", " << rs.message << std::endl;
        if (rs.status != tradery_thrift_api::RuntimeStatus::ENDED) {
          done = false;
        }
        Sleep(250);
      }
    } while (!done);

    transport->close();
  } catch (TException& tx) {
    std::cout << "TException: " << tx.what() << std::endl;
    BOOST_TEST(false);
  } catch (exception& e) {
    std::cout << "exception: " << e.what() << std::endl;
    BOOST_TEST(false);
  } catch (...) {
    std::cout << "unknown error" << std::endl;
    BOOST_TEST(false);
  }
}

bool init_unit_test() {
  std::cout << "using custom init" << std::endl;
  return true;
}
