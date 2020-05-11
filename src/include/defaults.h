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

constexpr auto DEFAULT_SYMBOL_TIMEOUT = 10;
constexpr auto DEFAULT_HEARTBEAT_TIMEOUT = 10;
constexpr auto DEFAULT_REVERSE_HEARTBEAT_PERIOD = 10;
constexpr auto DEFAULT_INITIAL_CAPITAL = 100000.00;
constexpr auto DEFAULT_CACHE_SIZE = 200;
constexpr auto DEFAULT_SLIPPAGE_VALUE = 0;
constexpr auto DEFAULT_COMMISION_VALUE = 0;
constexpr auto DEFAULT_MAX_LINES_PER_FILE = 200;
constexpr auto DEFAULT_MAX_BARS_PER_SESSION = 0;
constexpr auto DEFAULT_MAX_OPEN_POSITIONS = 0;
constexpr auto DEFAULT_OS_DIR_ROOT = "c:\\windows";
#define DEFAULT_POS_SIZE_TYPE PosSizeType::system_defined
constexpr auto DEFAULT_POSITION_SIZE = 0;
#define DEFAULT_POS_SIZE_LIMIT_TYPE PosSizeLimitType::none
constexpr auto DEFAULT_POSITION_SIZE_LIMIT = 0;
constexpr auto DEFAULT_LINES_PER_PAGE = 25;
constexpr auto DEFAULT_CPU_COUNT = 2;
constexpr auto DEFAULT_THREAD_COUNT = 2;
constexpr auto DEFAULT_THREADING_ALGORITHM = 1; // default is one system will be run on multiple threads if possible
#define DEFAULT_DATA_ERROR_HANDLING_MODE ErrorHandlingMode::fatal
constexpr auto DEFAULT_RUN_AS_USER = false;
constexpr auto DEFAULT_TRADES_FILE = "trades.htm";
constexpr auto DEFAULT_STATS_FILE = "stats.htm";
constexpr auto DEFAULT_ERRORS_FILE = "errors.htm";
constexpr auto DEFAULT_CANCEL_FILE = "cancel.txt";
constexpr auto DEFAULT_TRADES_CSV_FILE = "trades.csv";
constexpr auto DEFAULT_RAW_TRADES_CSV_FILE = "trades_raw.csv";
constexpr auto DEFAULT_RAW_SIGNALS_CSV_FILE = "signals_raw.csv";
constexpr auto DEFAULT_SIGNALS_CSV_FILE = "signals.csv";
constexpr auto DEFAULT_TRADES_DESC_FILE = "tradesDesc.htm";
constexpr auto DEFAULT_SIGNALS_DESC_FILE = "signalsDesc.txt";
constexpr auto DEFAULT_HEARTBEAT_FILE = "heartBeat.txt";
constexpr auto DEFAULT_RUNTIMESTATS_FILE = "runtimeStats.txt";
constexpr auto DEFAULT_SESSION_LOG_FILE = "session.log";
constexpr auto DEFAULT_END_RUN_SIGNAL_FILE = "endRun.txt";
constexpr auto DEFAULT_OUTPUT_FILE = "output.txt";
constexpr auto DEFAULT_FROM_DATETIME = "";
constexpr auto DEFAULT_STATS_CSV_FILE = "stats.csv";
constexpr auto DEFAULT_TO_DATETIME = "";
constexpr auto DEFAULT_REVERSE_HEARTBEAT_FILE = "reverseHeartBeat.txt";
constexpr auto DEFAULT_FLAT_DATA = false;
constexpr auto DEFAULT_EQUITY_CURVE_FILE = "equityCurve";
constexpr auto DEFAULT_CHARTS_DESCRIPTION_FILE = "charts_description.xml";
constexpr auto DEFAULT_CHARTS_DIR = "charts";
constexpr auto DEFAULT_START_TRADES_DATE = "";
constexpr auto DEFAULT_PLUGIN_EXT = "dll";
constexpr auto DEFAULT_EXPLICIT_TRADES_EXT = "etr";
