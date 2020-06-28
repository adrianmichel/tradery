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
#include "Configuration.h"
#include <positionsizingparams.h>
#include <tokenizer.h>
#include "tradery.h"
#include "defaults.h"
#include <loggers.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LPCSTR HELP[] = { "help,?", "shows this help message" };
#define _TEST

// low case
constexpr char* DATASOURCEPATH[] = {"datasourcepath,a", "the datasource path"};
constexpr char* SESSION_PARENT_PATH[] = { "sessionparentpath,b", "path to session parent directory"};
constexpr char* CONFIGFILEPATH[] = { "configfilepath,c", "the config file path" };
constexpr char* DATASOURCEID[] = { "datasourceid,d", "the id of the data source plugin for the session" };
constexpr char* TRADES_FILE[] = {"tradesfile,e", "the file that will contain the simulated historical trades generated during the run, absolute or relative to the session path"};
constexpr char* DEFINE_MACRO[] = { "define,f", "defines a macro that can be used in other command line parameters. Format is \"MACRO_NAME:VALUE\"" };
//g
constexpr char* STATSHANDLERID[] = { "statshandlerid,h", "the stats handler for the session - will calculate statistics and save them to a file" };
constexpr char* INCLUDEPATH[] = {"includepath,i", "the path to include files, can appear multiple times"};
constexpr char* OUTPUTPATH[] = { "outputpath,j", "the runnable plug-in project path" };
//k
constexpr char* LIBPATH[] = { "libpath,l", "sets path to the simlib lib files, can appear multiple times" };
constexpr char* TIMEOUT[] = { "timeout,m", "timeout in miliseconds, after which the run stops"};
constexpr char* SESSIONID[] = { "sessionid,n", "the session unique id" };
constexpr char* SYMBOLSSOURCEID[] = { "symbolssourceid,o", "the symbols source for the session" };
constexpr char* PLUGIN_PATH[] = { "pluginpath,p", "the path for the built-in plug-ins (datasource, symbolssource etc)" };
constexpr char* UNIQUE_ID[] = { "uniqueid,q", "generates and unique id on the command line"};
constexpr char* PROJECT_PATH[] = {"projectpath,r", "path to the project that will build the plug-ins"};
constexpr char* STATS_FILE[] = { "statsfile,s", "the file that will contain the back-test statistics for all the trades generated during the run"};
constexpr char* TOOLSPATH[] = {"toolspath,t", "sets the path for MS tools, such as nmake, cl etc"};
constexpr char* SYSTEM_FILE[] = {"systemfile,u", "trading system file name"};
constexpr char* RUNSIMULATOR[] = {"runsimulator,v", "run just the simulation part. If this option is not on the command line, then build first then call itself with this option"};
//w
constexpr char* PLUGIN_EXT[] = { "pluginext,x", "sets the plugin file extesion"};
constexpr char* SYMBOLSSOURCEFILE[] = { "symbolssourcefile,y", "the file containing the symbols for the session" };
constexpr char* ERRORSFILE[] = { "errorsfile,z", "the file containing the build errors, if any, to be reported to the user"};

// caps
constexpr char* ASYNCHRONOUS_RUN[] = { "asyncrun,A", "when present on the command line, it indicates that the run will be asynchronous - the call will return immediately, while the processing is still going on"};
constexpr char* SYMBOL_TIMEOUT[] = { "symboltimeout,B", "symbol timeout in seconds - if a symbol processing takes longer than this amount, the processing will be canceled"};
constexpr char* CANCELFILE[] = { "cancelfile,C", "file that will be created as a result of the client requesting that the server cancels the processing"};
constexpr char* HEARTBEAT_TIMEOUT[]= { "heartbeattimeout,D", "heartbeat timeout in seconds - used to keep the server processing a session alive, if a heartbeat signal is not received within this number of seconds, the processing is canceled"};
constexpr char* TRADES_CSV_FILE[] = { "tradescsvfile,E", "file containing trades in csv format"};
constexpr char* SIGNALS_CSV_FILE[] = { "signalscsvfile,F", "file containing signals in csv format"};
constexpr char* RAW_SIGNALS_CSV_FILE[] = { "rawsignalscsvfile,G", "file containing the raw signals (before applying signal sizing) in csv format" };
constexpr char* TRADES_DESCRIPTION_FILE[] = { "tradesdescfile", "The file where the trades description will be stored"};
constexpr char* SIGNALS_DESCRIPTION_FILE[] = { "signalsdescfile", "The file where the signals description will be stored"};
constexpr char* LINES_PER_PAGE[] = { "linesperpage", "The number of lines per page - used to store info about line positions in the file for faster access" };
constexpr char* HEARTBEATFILE[] = { "heartbeatfile,H", "file that will signal to the running process that the client is still waiting for the result. If the file doesn't exist for a specified amount of time, the process will be terminated" };
constexpr char* ZIPFILE[] = {"zipfile,I", "zip file containing the result of the run (all cvs files"};
constexpr char* REVERSE_HEARTBEAT_PERIOD[] = {"reverseheartbeatperiod,J", "the period of the hearbeat signal generated during processing, it will be used to keep the client alive"};
constexpr char* RUNTIME_STATS_FILE[] = { "runtimestatsfile,K", "file that will contain runtime stats such elapsed time, number of errors, of trades etc"};
//LPCSTR LOGFILE[] = { "logfile,L", "log file name" };
constexpr char* DEFCOMMISSIONVALUE[] = { "defcommissionvalue,M", "the default commission value", };
constexpr char* ENDRUNSIGNALFILE[] = { "endrunsignalfile,N", "if this is present on the command line, the end of a run will be signaled by creating this file - used in case of an asynchronous run" };
constexpr char* OUTPUT_FILE[] = { "outputfile,O", "the file that will contain the formatted text output by the system" };
constexpr char* DLLPATH[] = { "dllpath,P", "indicates where additional dlls may be located when loading plugins that depend on other dll" };
constexpr char* DEFSLIPPAGEVALUE[] = { "defslippagevalue,Q", "the default slippage value" };
constexpr char* FROM_DATETIME[] = { "fromdatetime,R", "time range starting date/time in iso format. If absent, it will mean Negative Infinity" };
constexpr char* STATS_CSV_FILE[] = { "statscsvfile,S", "file containing stats in csv format" };
constexpr char* TO_DATETIME[] = { "todatetime,T", "time range ending date/time in iso format. If absent, it will mean Positive Infinity" };
//U
constexpr char* REVERSEHEARTBEATFILE[] = { "reverseheartbeatfile,V", "file generated with a certain period, that indicates that the process is still running. If the file is not generated for a speicified amount of time, the client can assume that the server process is not running any more" };
constexpr char* DEFSLIPPAGEID[] = { "defslippageid,W", "the default slippage plugin config id" };
constexpr char* DEFCOMMISSIONID[] = { "defcommissionid,X", "the default commission plugin config id" };
constexpr char* CACHESIZE[] = { "cachesize,Y", "the internal simlib cache max number of items, currently used only because symbol info is needed to calculate stats" };
constexpr char* RAW_TRADES_CSV_FILE[] = { "rawtradescsvfile,Z", "the list of raw trades before applying position sizing" };
//Z

// no one character shortcut options
constexpr char* MAX_LINES[] = { "maxlines", "max number of lines in file such as trades, etc. If this file exceeded, show a message" };
constexpr char* MAX_TOTAL_BAR_COUNT[] = { "maxtotalbarcount", "max total number of bars per session - used to limit the the usage to a max number of total bars, this may translate in a lot of symbols, few bars, or few symbols, many bars" };
constexpr char* FLAT_DATA[] = { "flatdata", "" };
constexpr char* EQUITY_CURVE_FILE[] = { "equitycurvefile", "the base name for files containing equity curve data: csv, htm, jpg" };
constexpr char* SYMBOLS_TO_CHART_FILE[] = { "symchartfile", "The file containing a list of symbols for which the trading engine will generate charting data. The trading engine will ignore the charting statements when running on any other symbol" };
constexpr char* CHART_DESCRIPTION_FILE[] = { "chartdescriptionfile", "The file containing the description of all charting info generated during the run. Will be used by the php script to generate the actual charts" };
constexpr char* CHART_PARENT_PATH[] = { "chartrootpath", "The directory where all the chart info will be stored - description files, data, images" };

// position sizing options
constexpr char* INITIAL_CAPITAL[] = { "initialcapital", "the intial capital value used in simulation" };
constexpr char* MAX_OPEN_POS[] = { "maxopenpos", "the maximum position open at any one time during simulation" };
constexpr char* POS_SIZE_TYPE[] = { "possizetype", "position size type: 0-4 (system, shares, value, pct eq, pct cash" };
constexpr char* POS_SIZE[] = { "possize", "position size actual value" };
constexpr char* POS_SIZE_LIMIT_TYPE[] = { "possizelimittype", "position size type: 0-2 (none, pct volume, value" };
constexpr char* POS_SIZE_LIMIT[] = { "possizelimit", "position size limit value" };
constexpr char* OS_PATH[] = { "ospath", "The OS path" };
constexpr char* SYSTEM_NAME_KEYWORD[] = { "sysnamekword", "The system name keyword" };
constexpr char* CLASS_NAME_KEYWORD[] = { "classnamekword", "The class name keyword" };

constexpr char* CPU_COUNT[] = { "cpucount", "number of cpus" };
constexpr char* THREADS[] = { "threads", "number of threads" };
constexpr char* THREAD_ALG[] = { "threadalg", "threading algorithm" };

constexpr char* EXT_TRIGGERS_FILE[] = { "exttriggersfile", "if this option is present, this will be interpreted as a file containing a list of triggers that the simulator will use to generate trades" };

constexpr char* EXPLICIT_TRADES_EXT[] = { "explicittradesext", "Explicit trades files extension" };
constexpr char* DATA_ERROR_HANDLING_MODE[] = { "dataerrorhandling", "Data error handling mode - 0: fatal, 1: warning, 2: ignore" };

constexpr char* START_TRADES_DATE[] = { "starttradesdate", "cutoff date for trades (no trades before this date, used for automated trading)" };

constexpr char* ENV_PATH[] = { "envpath", "environment variable PATH" };
constexpr char* ENV_INCLUDE[] = { "envinclude", "environment variable include" };
constexpr char* ENV_LIB[] = { "envlib", "enviroment variable lib" };

constexpr char* ENABLE_RUN_AS_USER[] = { "enablerunasuser", "enable/disable running as user" };

constexpr char* CONFIG_FILE[] = { "configfile", "configuration file(s) name" };

std::string longName(const LPCSTR* option) {
  Tokenizer tok(option[0], ",");
  assert(tok.size() == 1 || tok.size() == 2);

  return tok[0];
}

std::optional< std::string > shortName(const std::string& option) {
  Tokenizer tok(option, ",");
  assert(tok.size() == 1 || tok.size() == 2);

  return tok.size() == 2 ? std::optional< std::string >( tok[1] ) : std::nullopt;
}

#define SEP "\n\r"

std::string parse_config_file(const std::string& configFile) {
  ifstream ifs(configFile.c_str());
  if (!ifs)
    throw ConfigurationException( "Could not open the configuration file "s + configFile);

  stringstream ss;

  ss << ifs.rdbuf();
  Tokenizer tok(ss.str(), SEP);

  std::string s;

  for (Tokenizer::const_iterator i = tok.begin(); i != tok.end(); ++i) {
    std::string token(tradery::trim(*i));

    if (!token.empty() && token[0] != '#') {
      std::wstring::size_type n = token.find_first_of(TCHAR('='));

      if (n != std::string::npos) {
        token[n] = ' ';
      }
      if (token[0] != '-') {
        s += "--" + token + " ";
      }
      else {
        s += token + " ";
      }
    }
  }

  return s;
}

#define PO( a, b ) \
  ( a[0], po::value< b >(), a[1] )

#define PO_STR( a) PO( a, std::string )

#define PO_DEF( a, b, c ) \
  ( a[0], po::value< c >()->default_value( b), a[1] )

#define PO_BOOL( a ) \
  ( a[0], a[1] )
 
#define PO_MULTI( a ) \
  ( a[0], po::value< std::vector<std::string >>()->composing(), a[1] )

po::options_description makeOptionsDescription(){
  po::options_description desc("Tradery options");
  desc.add_options()
    PO_BOOL(HELP)
    PO_STR(DATASOURCEPATH)
    PO_STR(SESSION_PARENT_PATH)
    PO_STR(CONFIGFILEPATH)
    PO_STR(DATASOURCEID)
    PO_DEF(TRADES_FILE, DEFAULT_TRADES_FILE, std::string)
    PO_MULTI(DEFINE_MACRO)
    PO_STR(STATSHANDLERID)
    PO_MULTI(INCLUDEPATH)
    PO_STR(OUTPUTPATH)
    PO_MULTI(LIBPATH)
    PO(TIMEOUT, unsigned long)
    PO_STR(SESSIONID)
    PO_STR(SYMBOLSSOURCEID)
    PO_STR(PLUGIN_PATH)
    PO_BOOL(UNIQUE_ID)
    PO_STR(PROJECT_PATH)
    PO_DEF(STATS_FILE, DEFAULT_STATS_FILE, std::string)
    PO_STR(TOOLSPATH)
    PO_MULTI(SYSTEM_FILE)
    PO_BOOL(RUNSIMULATOR)
    PO_DEF(PLUGIN_EXT, DEFAULT_PLUGIN_EXT, std::string)
    PO_STR(SYMBOLSSOURCEFILE)
    PO_DEF(ERRORSFILE, DEFAULT_ERRORS_FILE, std::string)
    PO_BOOL(ASYNCHRONOUS_RUN)
    PO_DEF(SYMBOL_TIMEOUT, DEFAULT_SYMBOL_TIMEOUT, unsigned __int64)
    PO_DEF(CANCELFILE, DEFAULT_CANCEL_FILE, std::string)
    PO_DEF(HEARTBEAT_TIMEOUT, DEFAULT_HEARTBEAT_TIMEOUT, unsigned __int64)
    PO_DEF(TRADES_CSV_FILE, DEFAULT_TRADES_CSV_FILE, std::string)
    PO_DEF(RAW_TRADES_CSV_FILE, DEFAULT_RAW_TRADES_CSV_FILE, std::string)
    PO_DEF(SIGNALS_CSV_FILE, DEFAULT_SIGNALS_CSV_FILE, std::string)
    PO_DEF(RAW_SIGNALS_CSV_FILE, DEFAULT_RAW_SIGNALS_CSV_FILE, std::string)
    PO_DEF(TRADES_DESCRIPTION_FILE, DEFAULT_TRADES_DESC_FILE, std::string)
    PO_DEF(SIGNALS_DESCRIPTION_FILE, DEFAULT_SIGNALS_DESC_FILE, std::string)
    PO_DEF(LINES_PER_PAGE, DEFAULT_LINES_PER_PAGE, size_t)
    PO_DEF(HEARTBEATFILE, DEFAULT_HEARTBEAT_FILE, std::string)
    PO_STR(ZIPFILE)
    PO_DEF(REVERSE_HEARTBEAT_PERIOD, DEFAULT_REVERSE_HEARTBEAT_PERIOD, unsigned __int64)
    PO_DEF(RUNTIME_STATS_FILE, DEFAULT_RUNTIMESTATS_FILE, std::string)
    //PO_DEF(LOGFILE, DEFAULT_SESSION_LOG_FILE, std::string)
    PO_DEF(DEFCOMMISSIONVALUE, DEFAULT_COMMISION_VALUE, double)
    PO_DEF(ENDRUNSIGNALFILE, DEFAULT_END_RUN_SIGNAL_FILE, std::string)
    PO_DEF(OUTPUT_FILE, DEFAULT_OUTPUT_FILE, std::string)
    PO_STR(DLLPATH)
    PO_DEF(DEFSLIPPAGEVALUE, DEFAULT_SLIPPAGE_VALUE, double)
    PO_DEF(FROM_DATETIME, DEFAULT_FROM_DATETIME, std::string)
    PO_DEF(STATS_CSV_FILE, DEFAULT_STATS_CSV_FILE, std::string)
    PO_DEF(TO_DATETIME, DEFAULT_TO_DATETIME, std::string)
    PO_DEF(REVERSEHEARTBEATFILE, DEFAULT_REVERSE_HEARTBEAT_FILE, std::string)
    PO_STR(DEFSLIPPAGEID)
    PO_STR(DEFCOMMISSIONID)
    PO_DEF(CACHESIZE, DEFAULT_CACHE_SIZE, unsigned __int64)
    PO_DEF(MAX_LINES, DEFAULT_MAX_LINES_PER_FILE, unsigned __int64)
    PO_DEF(MAX_TOTAL_BAR_COUNT, DEFAULT_MAX_BARS_PER_SESSION, unsigned __int64)
    PO_DEF(FLAT_DATA, DEFAULT_FLAT_DATA, bool)
    PO_DEF(EQUITY_CURVE_FILE, DEFAULT_EQUITY_CURVE_FILE, std::string)
    PO_STR(SYMBOLS_TO_CHART_FILE)
    PO_DEF(CHART_DESCRIPTION_FILE, DEFAULT_CHARTS_DESCRIPTION_FILE, std::string)
    PO_DEF(CHART_PARENT_PATH, DEFAULT_CHARTS_DIR, std::string)
    PO_DEF(INITIAL_CAPITAL, DEFAULT_INITIAL_CAPITAL, double)
    PO_DEF(MAX_OPEN_POS, DEFAULT_MAX_OPEN_POSITIONS, unsigned __int64)
    PO_DEF(POS_SIZE_TYPE, DEFAULT_POS_SIZE_TYPE, unsigned long)
    PO(POS_SIZE, double)
    PO_DEF(POS_SIZE_LIMIT_TYPE, DEFAULT_POS_SIZE_LIMIT_TYPE, unsigned long)
    PO(POS_SIZE_LIMIT, double)
    PO_DEF(OS_PATH, DEFAULT_OS_DIR_ROOT, std::string)
    PO_STR(SYSTEM_NAME_KEYWORD)
    PO_STR(CLASS_NAME_KEYWORD)
    PO_DEF(CPU_COUNT, DEFAULT_CPU_COUNT, unsigned long)
    PO_DEF(THREADS, DEFAULT_THREAD_COUNT, unsigned long)
    PO_DEF(THREAD_ALG, DEFAULT_THREADING_ALGORITHM, unsigned long)
    PO_STR(EXT_TRIGGERS_FILE)
    PO_DEF(EXPLICIT_TRADES_EXT, DEFAULT_EXPLICIT_TRADES_EXT, std::string)
    PO_DEF(DATA_ERROR_HANDLING_MODE, DEFAULT_DATA_ERROR_HANDLING_MODE, unsigned int)
    PO_DEF(START_TRADES_DATE, DEFAULT_START_TRADES_DATE, std::string)
    PO_STR(ENV_PATH)
    PO_STR(ENV_INCLUDE)
    PO_STR(ENV_LIB)
    PO_DEF(ENABLE_RUN_AS_USER, DEFAULT_RUN_AS_USER, bool)
    PO_MULTI(CONFIG_FILE)
    ;

  return desc;
}

struct Macro {
private:
  const std::string m_name;
  const std::string m_value;

public:
  Macro( const std::string& name, const std::string& value )
    : m_name( name ), m_value( value ){}

  const std::string& name() const { return m_name; }
  const std::string& value() const { return m_value; }
};


class Macros {
private:
  std::vector< Macro > m_macros;

public:
  // each string has a macro in the format: name=value
  Macros(const std::vector< Macro >& predefinedMacros, const std::vector< string >& macros)
    : m_macros(predefinedMacros) {
    static const Sep sep(":");

    for (auto macro : macros) {
      Tokenizer tok(macro, sep, 2);
      if (tok.size() != 2) {
        throw ConfigurationException("invalid macro: "s + macro);
      }
      m_macros.emplace_back( "${"s + tok[0] + "}", tok[1]);
    }
  }

  std::string substitute(const std::string& str) {
    std::string s = str;
    for (Macro macro : m_macros) {
      std::string v = boost::replace_all_copy(macro.value(), "\"", "");
      boost::ireplace_all(s, macro.name(), v);
    }
    return s;
  }

  std::vector< std::string > substitute(const std::vector< std::string >in) {
    std::vector< std::string > out;
    std::transform(in.begin(), in.end(), std::back_inserter(out), [this](const std::string& str) { return substitute(str); });
    return out;
  }
};

void Configuration::init(const std::string& cmdLine, bool validate) {
  Log::log().addLogger(std::make_shared< tradery::ConsoleLogger >(Level::log_debug));
  m_inputCommandLine = cmdLine;
  po::options_description cmd;
  po::options_description visible("Allowed options");

  try {
    // Declare a group of options that will be
    // allowed only on command line
    po::options_description desc = makeOptionsDescription();
    po::positional_options_description p;
    p.add("runnable", -1);

    m_args = tradery::cmdLineSplitter(cmdLine);

    po::variables_map vm;
    po::store(po::command_line_parser(m_args).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.contains(longName( CONFIG_FILE ))) {
      std::vector<std::string> configFiles( vm[longName(CONFIG_FILE)].as<std::vector<std::string> >());

      for (auto configFile : configFiles ) {
        std::string s(parse_config_file(configFile.c_str()));

        std::vector<std::string> configFileArgs = cmdLineSplitter(s);
        po::store(po::command_line_parser(configFileArgs).options(desc).positional(p).run(), vm);
      }
    }

    LOG(log_debug, "setting session id" );
    m_sessionId = vm.contains(longName(SESSIONID)) ? UniqueId{ vm[longName(SESSIONID)].as<std::string >() } : UniqueId{};
    if (vm.contains(longName(SESSIONID))) {
      LOG(log_debug, "set session id from configuration to: ", m_sessionId);
    }
    else {
      LOG(log_debug, "set session id randomly to: ", m_sessionId);
    }

    if (vm.contains(longName(SESSION_PARENT_PATH))) m_sessionParentPath = vm[longName(SESSION_PARENT_PATH)].as<std::string>();

    std::error_code error;
    if (!fs::create_directories(getSessionPath(), error)) {
      throw ConfigurationException("Creating session directory: "s + getSessionPath() + " failed with error: " + std::to_string(error.value()));
    }

    // start logging immediately
    LogFileConfig config(getSessionPath(), ".log"s, Level::log_debug, 100, 1000000, false);
    Log::log().addLogger(std::make_shared < tradery::FileLogger >(config, "session_"));

    LOG(log_debug, "input command line: ", cmdLine);
    LOG(log_debug, "args");
    for (size_t i = 0; i < m_args.size(); ++i) {
      LOG(log_debug, "args[", i, "] = ", m_args[i]);
    }
    // handle standalone command line options

    if (vm.contains(longName(HELP))) {
      help(visible);
      m_help = true;
      return;
    }
    else {
      m_help = false;
    }
    LOG(log_debug, "Processing command line: ", cmdLine);
    m_fromDateTime = vm[longName(FROM_DATETIME )].as<std::string>();
    m_toDateTime = vm[longName(TO_DATETIME)].as<std::string>();
    m_startTradesDateTime = vm[longName( START_TRADES_DATE )].as<std::string>();
    LOG(log_debug, "from - to: ", m_fromDateTime, "-", m_toDateTime);

    if (m_fromDateTime > m_toDateTime) {
      // error - do something
    }

    m_runSimulator = vm.contains(longName(RUNSIMULATOR ));
    LOG(log_debug, "reading the runnables");
    if (vm.contains(longName(SYSTEM_FILE))) {
      std::vector< std::string > v = vm[longName(SYSTEM_FILE)].as<vector<std::string> >();
      std::transform(v.begin(), v.end(), std::back_inserter(m_systems), [](const std::string& str) { return FileTradingSystem(str); });
    }
    LOG(log_debug, "systems count: ", m_systems.size());


    std::vector< std::string > config_macros;

    if (vm.contains(longName(DEFINE_MACRO))) {
      config_macros = vm[longName(DEFINE_MACRO)].as<vector<std::string> >();
    }

    const std::vector< Macro > predefinedMacros = {
      {"${VcpkgPath}", EXPAND(VCPKG_PATH) },
      {"${LibPath}", EXPAND(LIB_PATH)},
      {"${BinPath}", EXPAND(BIN_PATH)},
      {"${SessionPath}", getSessionPath()},
      {"${IncludePath}", EXPAND( INCLUDE_PATH )},
      {"${BoostIncludePath}", EXPAND( BOOST_INCLUDE_PATH )},
      {"${RuntimeProjPath}", EXPAND(RUNTIME_PROJ_PATH)},
      {"${ExternalPath}", EXPAND(EXTERNAL_PATH)},
      {"${SolutionPath}", EXPAND(SOLUTION_PATH)}
    };

    Macros macros(predefinedMacros, config_macros);

    m_includePaths = macros.substitute(vm[longName(INCLUDEPATH)].as<vector<std::string> >());
    m_libPath = macros.substitute(vm[longName(LIBPATH)].as<vector<std::string> >());

    m_toolsPath = macros.substitute(vm[longName(TOOLSPATH)].as<std::string>());
    if (vm.contains(longName( PROJECT_PATH ))) m_projectPath = macros.substitute(vm[longName(PROJECT_PATH)].as<std::string>());
    LOG(log_debug, "reading outputpath");
    if (vm.contains(longName(OUTPUTPATH))) {
      m_outputPath = macros.substitute(vm[longName(OUTPUTPATH)].as<std::string>());
    }

    LOG(log_debug, "reading symbolssourceid");
    if (vm.contains(longName(SYMBOLSSOURCEID))) m_symbolsSource = vm[longName(SYMBOLSSOURCEID)].as<std::string>();
    if (vm.contains(longName(STATSHANDLERID))) m_statsHandler = vm[longName(STATSHANDLERID)].as<std::string>();
    LOG(log_debug, "reading symbolssourcefile");
    if (vm.contains(longName(SYMBOLSSOURCEFILE ))) m_symbolsSourceFile = vm[longName(SYMBOLSSOURCEFILE)].as<std::string>();
    LOG(log_debug, "reading datasourcepath");
    if (vm.contains(longName(DATASOURCEPATH))) m_dataSourcePath = vm[longName(DATASOURCEPATH)].as<std::string>();
    LOG(log_debug, "reading datasourceid");
    if (vm.contains(longName(DATASOURCEID))) m_dataSource = vm[longName(DATASOURCEID)].as<std::string>();

    LOG(log_debug, "reading pluginpath");
    m_pluginPath = macros.substitute( vm[longName(PLUGIN_PATH)].as<std::string>());

    LOG(log_debug, "plugin path: ", m_pluginPath);

    m_errorsFile = vm[longName(ERRORSFILE)].as<std::string>();
    LOG(log_debug, "reading pluginext");
    m_pluginExt = vm[longName(PLUGIN_EXT )].as<std::string>();

    LOG(log_debug, "reading tradesfile");
    m_tradesFile = vm[longName( TRADES_FILE )].as<std::string>();

    LOG(log_debug, "reading statsfile");
    m_statsFile = vm[longName( STATS_FILE )].as<std::string>();
    LOG(log_debug, "reading outputfile");
    m_outputFile = vm[longName(OUTPUT_FILE)].as<std::string>();

    LOG(log_debug, "reading dllpath");
    m_dllPath = macros.substitute(vm[longName(DLLPATH)].as<std::string>());
    LOG(log_debug, "setting dll directory to: "s + m_dllPath);
    if (!m_dllPath.empty()) {
      if (!SetDllDirectory(s2ws(m_dllPath).c_str())){
        throw ConfigurationException("Could not set dll path to: "s + m_dllPath + ", last error: " + std::to_string(GetLastError()));
      }
    }

    LOG(log_debug, "reading tradescsvfile");
    m_tradesCSVFile = vm[longName( TRADES_CSV_FILE )].as<std::string>();
    LOG(log_debug, "reading rawtradescsvfile");
    m_rawTradesCSVFile = vm[longName(RAW_TRADES_CSV_FILE)].as<std::string >();

    LOG(log_debug, "reading signalscsvfile");
    m_signalsCSVFile = vm[longName( SIGNALS_CSV_FILE)].as<std::string>();
    LOG(log_debug, "reading rawsignalscsvfile");
    m_rawSignalsCSVFile = vm[longName(RAW_SIGNALS_CSV_FILE)].as<std::string>();
    LOG(log_debug, "reading statscsvfile");
    m_statsCSVFile = vm[longName( STATS_CSV_FILE)].as<std::string>();
    m_equityCurveFile = vm[longName( EQUITY_CURVE_FILE )].as<std::string>();
    LOG(log_debug, "reading timeout");
    if (vm.contains( longName( TIMEOUT ))) m_timeout = vm[longName( TIMEOUT )].as<unsigned long>();
    LOG(log_debug, "reading zipfile");
    if (vm.contains(longName( ZIPFILE ))) m_zipFile = vm[longName( ZIPFILE )].as<std::string>();
    LOG(log_debug, "reading heartbeatfile");
    if (vm.contains(longName( HEARTBEATFILE ))) m_heartBeatFile = vm[longName(HEARTBEATFILE)].as<std::string>();
    LOG(log_debug, "reading reverse heartbeat file");
    if (vm.contains(longName( REVERSEHEARTBEATFILE))) m_reverseHeartBeatFile = vm[longName(REVERSEHEARTBEATFILE)].as<std::string>();
    LOG(log_debug, "reading cancel file");
    if (vm.contains(longName( CANCELFILE ))) m_cancelFile = vm[longName(CANCELFILE)].as<std::string>();
    LOG(log_debug, "reading symbol timeout");
    m_symbolTimeout = vm[longName( SYMBOL_TIMEOUT )].as<unsigned __int64>();
    LOG(log_debug, "reading reverse heart beat period");
    m_reverseHeartBeatPeriod = vm[longName(REVERSE_HEARTBEAT_PERIOD)].as<unsigned __int64>();
    LOG(log_debug, "reading heartbeat timeout");
    m_heartBeatTimeout = vm[longName( HEARTBEAT_TIMEOUT)].as<unsigned __int64>();
    LOG(log_debug, "reading end run signal file");
    m_endRunSignalFile = vm[longName( ENDRUNSIGNALFILE)].as<std::string>();
    LOG(log_debug, "reading asynchronous run");
    m_asyncRun = vm.contains(longName( ASYNCHRONOUS_RUN));
    LOG(log_debug, "reading initial capital");
    if (vm.contains(longName( INITIAL_CAPITAL))) m_posSizingParams.setInitialCapital(vm[longName(INITIAL_CAPITAL)].as<double>());
    LOG(log_debug, "reading max open positions");
    if (vm.contains(longName( MAX_OPEN_POS))) m_posSizingParams.setMaxOpenPos(vm[longName(MAX_OPEN_POS)].as<unsigned __int64>());
    LOG(log_debug, "reading position size type");
    if (vm.contains(longName( POS_SIZE_TYPE))) m_posSizingParams.setPosSizeType( (PosSizeType)vm[longName(POS_SIZE_TYPE)].as<unsigned long>());
    LOG(log_debug, "reading position size");
    if (vm.contains(longName( POS_SIZE))) m_posSizingParams.setPosSize(vm[longName(POS_SIZE)].as<double>());
    LOG(log_debug, "reading position size limit type");
    if (vm.contains(longName(POS_SIZE_LIMIT_TYPE))) m_posSizingParams.setPosSizeLimitType( (PosSizeLimitType)vm[longName( POS_SIZE_LIMIT_TYPE)].as<unsigned long>());
    LOG(log_debug, "reading position size limit");
    if (vm.contains(longName(POS_SIZE_LIMIT))) m_posSizingParams.setPosSizeLimit(vm[longName(POS_SIZE_LIMIT)].as<double>());
    m_runtimeStatsFile = vm[longName( RUNTIME_STATS_FILE)].as<std::string>();
    LOG(log_debug, "reading cache size");
    m_cacheSize = vm[longName( CACHESIZE )].as<unsigned __int64>();
    LOG(log_debug, "reading default slippage value");
    m_defSlippageValue = vm[longName(DEFSLIPPAGEVALUE)].as<double>();
    LOG(log_debug, "reading default commission value");
    m_defCommissionValue = vm[longName(DEFCOMMISSIONVALUE)].as<double>();
    LOG(log_debug, "reading default commission id");
    m_defCommissionId = vm[longName(DEFCOMMISSIONID)].as<std::string>();
    LOG(log_debug, "reading default slippage id");
    m_defSlippageId = vm[longName(DEFSLIPPAGEID )].as<std::string>();
    LOG(log_debug, "reading max lines");
    m_maxLines = vm[longName( MAX_LINES )].as<unsigned __int64>();
    m_maxTotalBarCount = vm[longName( MAX_TOTAL_BAR_COUNT )].as<unsigned __int64>();
    LOG(log_debug, "reading symbols to chart file name");
    m_symbolsToChartFile = vm[longName( SYMBOLS_TO_CHART_FILE) ].as<std::string>();
    LOG(log_debug, "reading chart parent path");
    m_chartRootPath = vm[longName( CHART_PARENT_PATH )].as<std::string>();
    m_chartDescriptionFile = vm[longName( CHART_DESCRIPTION_FILE )].as<std::string>();
    LOG(log_debug, "reading external triggers file");
    m_extTriggersFile = vm.contains(longName(EXT_TRIGGERS_FILE)) ? std::optional< std::string >( vm[longName(EXT_TRIGGERS_FILE)].as<std::string >() ) : std::nullopt;
    LOG(log_debug, "reading system name keyword");
    m_systemNameKeyword = vm[longName( SYSTEM_NAME_KEYWORD )].as<std::string>();
    LOG(log_debug, "reading class name keyword");
    m_classNameKeyword = vm[longName( CLASS_NAME_KEYWORD )].as<std::string>();
    LOG(log_debug, "reading trades description file name");
    m_tradesDescriptionFile = vm[longName( TRADES_DESCRIPTION_FILE )].as<std::string>();
    LOG(log_debug, "reading trades description file name");
    m_signalsDescriptionFile = vm[longName( SIGNALS_DESCRIPTION_FILE )].as<std::string>();
    LOG(log_debug, "reading number of lines per page");
    m_linesPerPage = vm[longName( LINES_PER_PAGE )].as<size_t>();
    LOG(log_debug, "reading number of cpus");
    m_cpuCount = vm[longName( CPU_COUNT )].as<unsigned long>();
    LOG(log_debug, "reading number of threads");
    m_threads = vm[longName( THREADS )].as<unsigned long>();
    LOG(log_debug, "reading threading algorithm");
    m_threadAlg = ThreadAlgorithm(vm[longName( THREAD_ALG )].as<unsigned long>());
    LOG(log_debug, "reading explicit trades file extension");
    m_explicitTradesExt = vm[longName( EXPLICIT_TRADES_EXT )].as<std::string>();
    LOG(log_debug, "reading data error handling mode");
    m_dataErrorHandlingMode = (ErrorHandlingMode)vm[longName( DATA_ERROR_HANDLING_MODE )].as<unsigned int>();
    LOG(log_debug, "reading os path");
    if (vm.contains(longName( OS_PATH ))) m_osPath = vm[longName( OS_PATH )].as<std::string>();
    LOG(log_debug, "reading environment path");
    m_envPath = macros.substitute( vm[longName(ENV_PATH)].as<std::string>());

    TCHAR path[8000] = {0};
    GetEnvironmentVariable(_T( "path" ), path, 8000);

    boost::replace_all(m_envPath, "%path%", ws2s(path));
    m_envInclude = macros.substitute(vm[longName(ENV_INCLUDE)].as<std::string>());
    m_envLib = macros.substitute(vm[longName(ENV_LIB)].as<std::string>());
    m_enableRunAsUser = vm[longName( ENABLE_RUN_AS_USER )].as<bool>();

    LOG(log_debug, "cmd line processing done");

    if (validate) this->validate();
  }
  catch (exception& e) {
    LOG(log_error, "exception: ", e.what());
    throw;
  }
}

Configuration::Configuration(const std::string& cmdLine, bool validate) {
  init(cmdLine, validate);
}

Configuration::Configuration(bool validate) {
  init(ws2s(::GetCommandLine()), validate);
}

Configuration::Configuration(const char* cmdLine, bool validate) {
  init(std::string(cmdLine), validate);
}

tradery::EnvironmentPtr Configuration::getEnvironment() const {
  tradery::StrVector envVars;

  envVars.push_back("SystemRoot="s + osPath());
  envVars.push_back("path= "s + envPath());
  envVars.push_back("include="s + envInclude());
  envVars.push_back("lib="s + envLib());

  tradery::EnvironmentPtr env(std::make_shared< tradery::Environment >(envVars));

  return env;
}

void Configuration::validate() const {
  if (!hasTradingSystems()) {
    throw ConfigurationException("missing trading system file argument(s)");
  }

  if (!hasInclude()) {
    throw ConfigurationException("missing include argument");
  }

  if (!hasLib()) {
    throw ConfigurationException("missing lib argument");
  }

  if (!hasToolsPath()) {
    throw ConfigurationException("missing toolspath argument");
  }

  if (!hasProjectPath()) {
    throw ConfigurationException("missing projectpath argument");
  }

  if (!hasOutputPath()) {
    throw ConfigurationException("missing outputpath argument");
  }

  if (!hasSymbolsSourceId()) {
    throw ConfigurationException("missing symbolssourceid argument");
  }

  if (!hasStatsHandler()) {
    throw ConfigurationException("missing statshandlerid argument");
  }

  if (!hasSymbolsSourceFile()) {
    throw ConfigurationException("missing symbolssourcefile argument");
  }

  if (!hasDataSourcePath()) {
    throw ConfigurationException("missing datasourcepath argument");
  }

  if (!hasDataSourceId()) {
    throw ConfigurationException("missing datasourceid argument");
  }

  if (!hasOsPath()) {
    throw ConfigurationException("missing ospath argument");
  }

  if (!hasSessionParentPath()) {
    throw ConfigurationException("missing sessionpath argument");
  }

  if (!hasExplicitTradesExt()) {
    throw ConfigurationException("missing explicit trades extension");
  }

/*  if (!hasPluginPath()) {
    throw ConfigurationException("missing plugin path"), true );
  }
  */
}
