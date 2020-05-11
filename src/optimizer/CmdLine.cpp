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

// to add:
//  - volume multiplier
//	- log file
// 	- hidden diagnostic log
//  - symbol timeout
//  - autoselect output path

#include "stdafx.h"
#include "CmdLine.h"
#include <fstream>
#include <misc.h>
#include <tokenizer.h>

// abcdefghijklmnopqrstuvwxyz
// ******* * *** ************
//
// ABCDEFGHIJKLMNOPQRSTUVWXYZ
//   *   *     *  * ***  *

#define COMMAND_LINE_HELP_SHORT _T( "?"                  )
#define COMMAND_LINE_HELP_LONG _T( "help"               )

#define SYSTEM_SHORT _T( "m"                  )
#define SYSTEM_LONG _T( "system"             )

#define SYMBOLS_LIST_SHORT _T( "l"                  )
#define SYMBOLS_LIST_LONG _T( "symbolslist")

#define INITIAL_CAPITAL_SHORT _T( "i"                  )
#define INITIAL_CAPITAL_LONG _T( "initialcapital"     )

#define COMMISSION_SHORT _T( "c"                  )
#define COMMISSION_LONG _T( "commission"         )

#define SLIPPAGE_SHORT _T( "g"                  )
#define SLIPPAGE_LONG _T( "slippage"           )

#define USER_NAME_SHORT _T( "u"                  )
#define USER_NAME_LONG _T( "username"           )

#define PASSWORD_SHORT _T( "p"                  )
#define PASSWORD_LONG _T( "password"           )

#define VARIABLE_SHORT _T( "v"                  )
#define VARIABLE_LONG _T( "variable"           )

#define MAX_OPEN_POS_SHORT _T( "x"                  )
#define MAX_OPEN_POS_LONG _T( "maxopenpos"         )

#define POS_SIZE_TYPE_SHORT _T( "t"                  )
#define POS_SIZE_TYPE_LONG _T( "possizetype"        )

#define POS_SIZE_VALUE_SHORT _T( "z"                  )
#define POS_SIZE_VALUE_LONG _T( "possizevalue"       )

#define POS_SIZE_LIMIT_TYPE_SHORT _T( "y"                  )
#define POS_SIZE_LIMIT_TYPE_LONG _T( "possizelimittype"   )

#define POS_SIZE_LIMIT_VALUE_SHORT _T( "q"                  )
#define POS_SIZE_LIMIT_VALUE_LONG _T( "possizelimitvalue"  )

#define RUN_TYPE_SHORT \
  _T( "r"                  )  // simple optimization or walkforward
#define RUN_TYPE_LONG _T( "runtype"            )

#define LEAD_IN_SHORT _T( "d"                  )
#define LEAD_IN_LONG \
  _T( "leadin"             )  // lead-in in days (not bars). todo: leadinin bars

#define START_DATE_SHORT _T( "s"                  )
#define START_DATE_LONG _T( "startdate"          )

#define END_DATE_SHORT _T( "e"                  )
#define END_DATE_LONG _T( "enddate"            )

#define OPTIMIZATION_PERIOD_SHORT _T( "o"                  )  // in days
#define OPTIMIZATION_PERIOD_LONG _T( "optimizationperiod" )

#define WALK_FORWARD_PERIOD_SHORT \
  _T( "w"                  )  // walk forwad period, for walk forward only
                              // (apply optimized parameters to a run right
                              // after the optimized period
#define WALK_FORWARD_PERIOD_LONG _T( "walkforwardperiod"  )

#define STEP_SHORT \
  _T( "k"                  )  // step forward in days (to advance to the next
                              // optimization period)
#define STEP_LONG _T( "step"               )

#define WEIGHT_SHORT _T( "W"                  )  // weight factor
#define WEIGHT_LONG _T( "weight"             )

#define CROSSOVER_SHORT _T( "C"                  )  // crossover factor
#define CROSSOVER_LONG _T( "crossover"          )

#define POPULATION_SIZE_SHORT _T( "P"                  )
#define POPULATION_SIZE_LONG _T( "population"         )  // population size

#define MAX_GENERATIONS_SHORT \
  _T( "G"                  )  // max # of generations
#define MAX_GENERATIONS_LONG _T( "generations"        )

#define MUTATION_STRATEGY_SHORT _T( "M"                  )  // 1-5
#define MUTATION_STRATEGY_LONG _T( "mutationstrategy"   )

#define CONFIG_FILE_SHORT _T( "f"                  )
#define CONFIG_FILE_LONG _T( "configfile"         )

#define SERVER_SHORT _T( "S"                  )
#define SERVER_LONG _T( "server"             )

#define STAT_TO_OPTIMIZE_SHORT _T( "a"                  )
#define STAT_TO_OPTIMIZE_LONG _T( "optimize"           )

#define STAT_GROUP_TO_OPTIMIZE_SHORT _T( "T"                  )
#define STAT_GROUP_TO_OPTIMIZE_LONG _T( "groupToOptimize"    )

#define DIRECTION_TO_OPTIMIZE_SHORT _T( "b"                  )
#define DIRECTION_TO_OPTIMIZE_LONG _T( "direction"          )

#define PROCESSORS_COUNT_SHORT _T( "R"                  )
#define PROCESSORS_COUNT_LONG _T( "processors"         )

#define WORST_ACCEPTABLE_RESULT_SHORT _T( "w")
#define WORST_ACCEPTABLE_RESULT_LONG _T( "worstacceptableresult" )

#define ARGS(x) x##_LONG "," x##_SHORT

namespace po = boost::program_options;
using namespace std;

Configuration::Configuration()
    : _symbolsList(new std::wstring()),
      _directionToOptimize(boost::make_shared<DirectionSettable>()),
      _systems(boost::make_shared<SystemIds>()) {}

std::wstring Configuration::getUsage() const {
  std::ostringstream o;
  o << "Command line usage: " << std::endl
    << (std::string&)_description << std::endl;
  return o.str();
}

void Configuration::showUsage() {
  //_showInfo( getUsage().c_str(), _T( "Optimizer command line usage" ) );
}

void Configuration::showError(const std::wstring& error) {
  /*	ostringstream o;
          o << error << std::endl << std::endl << getUsage();
          _showInfo( o.str().c_str(), _T( "Optimizer command line error" ) );
  */
}

// Additional command line parser which interprets '@something' as a
// option "config-file" with the value "something"
pair<string, string> at_option_parser(string const& s) {
  if ('@' == s[0])
    return std::make_pair(string(CONFIG_FILE_LONG), s.substr(1));
  else
    return pair<string, string>();
}

typedef boost::shared_ptr<tradery::StrVector> StrVectorPtr;

typedef Tokenizer Tok;

std::wstring parse_config_file(const std::wstring& configFile) {
  ifstream ifs(configFile.c_str());
  if (!ifs)
    throw ConfigurationException(
        std::wstring("Could not open the configuration file ") << configFile);

  stringstream ss;

  ss << ifs.rdbuf();

  std::wstring sep("\n\r");
  Tok tok(ss.str(), sep);

  std::wstring s;

  for (Tok::const_iterator i = tok.begin(); i != tok.end(); ++i) {
    std::wstring token(tradery::trim(*i));

    if (!token.empty() && token[0] != '#') {
      std::wstring::size_type n = token.find_first_of('=');

      token[n] = ' ';
      s += "--" + token + " ";
    }
  }

  return s;
}

struct EscapedString {
 public:
  EscapedString(const std::string& str) : _str(str) {}
  std::string _str;
};

void validate(boost::any& v, const std::vector<std::string>& values,
              EscapedString*, int) {
  using namespace boost::program_options;

  // Make sure no previous assignment to 'a' was made.
  validators::check_first_occurrence(v);
  // Extract the first string from 'values'. If there is more than
  // one string, it's an error, and exception will be thrown.
  const string& s = validators::get_single_string(values);

  v = boost::any(EscapedString(tradery::unescape(s)));
}

#define INITIAL_CAPITAL_DEFAULT 100000
#define DEFAULT_END_DATE ""

bool Configuration::process() {
  //	::MessageBox(0,  AUTO_START_LONG.c_str(), 0, 0 );

  try {
    std::wstring today(tradery::LocalTimeSec().date().toString(us));
    po::options_description desc;
    desc.add_options()(ARGS(COMMAND_LINE_HELP), "shows this message")(
        ARGS(SYSTEM), po::value<std::vector<std::wstring> >()->required(),
        "system")(ARGS(SYMBOLS_LIST), po::value<std::wstring>()->required(),
                  "symbols list")(
        ARGS(INITIAL_CAPITAL),
        po::value<double>()->default_value(INITIAL_CAPITAL_DEFAULT),
        "initial capital")(ARGS(COMMISSION),
                           po::value<double>()->default_value(0.25),
                           "commission")(
        ARGS(SLIPPAGE), po::value<double>()->default_value(10), "slippage")(
        ARGS(USER_NAME), po::value<std::wstring>()->required(), "user name")(
        ARGS(PASSWORD), po::value<std::wstring>()->required(), "password")(
        ARGS(VARIABLE), po::value<std::vector<std::wstring> >()->required(),
        "variable as \"$name,type,min,max\" where type is real/integer")(
        ARGS(MAX_OPEN_POS), po::value<size_t>()->default_value(0),
        "max number of positions, 0 meaning unlimited")(
        ARGS(POS_SIZE_TYPE), po::value<unsigned int>()->default_value(0),
        "position size type: 0: system, 1: shares, 2: value; 3: pct equity, 4: "
        "pct cash ...")(ARGS(POS_SIZE_VALUE),
                        po::value<double>()->default_value(0),
                        "position size value")(
        ARGS(POS_SIZE_LIMIT_TYPE), po::value<unsigned int>()->default_value(0),
        "position size limit type: 0: ...")(
        ARGS(POS_SIZE_LIMIT_VALUE), po::value<double>()->default_value(0),
        "position size limit value")(ARGS(RUN_TYPE),
                                     po::value<std::wstring>()->required(),
                                     "run type: optimization/walkforward")(
        ARGS(LEAD_IN), po::value<unsigned int>()->default_value(30),
        "lead-in, in days")(ARGS(START_DATE),
                            po::value<std::wstring>()->required(),
                            "start date as mm/dd/yyyy")(
        ARGS(END_DATE),
        po::value<std::wstring>()->default_value(today)->implicit_value(today),
        "end date as mm/dd/yyyy or empty for most recent")(
        ARGS(OPTIMIZATION_PERIOD), po::value<unsigned int>()->required(),
        "optimization period in days")(ARGS(WALK_FORWARD_PERIOD),
                                       po::value<unsigned int>()->required(),
                                       "walk forward period in days")(
        ARGS(STEP), po::value<unsigned int>()->required()->default_value(0),
        "step,  in days. If 0, will use the whole date range")(
        ARGS(WEIGHT), po::value<double>()->required(),
        "differential evolution weight factor")(
        ARGS(CROSSOVER), po::value<double>()->required(),
        "differential evolution crossover factor")(
        ARGS(POPULATION_SIZE), po::value<unsigned int>()->required(),
        "population size")(ARGS(MAX_GENERATIONS),
                           po::value<unsigned int>()->required(),
                           "max generations")(
        ARGS(MUTATION_STRATEGY), po::value<unsigned int>()->required(),
        "mutation strategy")(ARGS(SERVER),
                             po::value<std::wstring>()->required(), "server")(
        ARGS(STAT_TO_OPTIMIZE), po::value<size_t>()->required(),
        "what statistic measure to optimize( 1: Tradery score, 2: etc..")(
        ARGS(DIRECTION_TO_OPTIMIZE), po::value<std::wstring>()->required(),
        "in what direction to optimize, min or max")(
        ARGS(STAT_GROUP_TO_OPTIMIZE),
        po::value<size_t>()->default_value(StatsValue::ValueType::total_value),
        "what group of statistics to optimize (0 - long+short, 1: long, 2: "
        "short")(ARGS(CONFIG_FILE), po::value<std::vector<std::wstring> >(),
                 "config file")(ARGS(PROCESSORS_COUNT),
                                po::value<unsigned int>()->required(),
                                "number of system processors")(
        ARGS(WORST_ACCEPTABLE_RESULT), po::value<double>()->default_value(0),
        "worst acceptable result in case of adrian optimization");

    _description << desc;

    std::vector<std::string> args =
        tradery::cmdLineSplitter(::GetCommandLine());

    // remove the executable name
    args.erase(args.begin());

    po::variables_map vm;
    po::store(po::command_line_parser(args).options(desc).run(), vm);

    if (vm.count("help")) {
      showUsage();
      return false;
    } else {
      // now loading the arguments from the configuration file (if any)
      if (vm.count(CONFIG_FILE_LONG) > 0) {
        std::vector<std::string> configFiles(
            vm[CONFIG_FILE_LONG].as<std::vector<std::string> >());

        for (std::vector<std::string>::size_type n = 0; n < configFiles.size();
             ++n) {
          std::wstring s(parse_config_file(configFiles[n].c_str()));
          po::store(po::command_line_parser(tradery::cmdLineSplitter(s))
                        .options(desc)
                        .run(),
                    vm);
        }
      }

      po::notify(vm);

#define SET_(name, var, type)        \
  assert(vm.count(name##_LONG) > 0); \
  var = vm[name##_LONG].as<type>();

      SET_(SYMBOLS_LIST, *_symbolsList, string)
      SET_(USER_NAME, _userName, string)
      SET_(PASSWORD, _password, string)
      SET_(RUN_TYPE, _runType, string)
      SET_(SERVER, _server, string)
      SET_(DIRECTION_TO_OPTIMIZE, *_directionToOptimize, string)
      SET_(START_DATE, _startDate, string)
      SET_(END_DATE, _endDate, string)
      SET_(INITIAL_CAPITAL, _initialCapital, double)
      SET_(MAX_OPEN_POS, _maxOpenPos, size_t)
      SET_(POS_SIZE_TYPE, _posSizeType, unsigned int)
      SET_(POS_SIZE_VALUE, _posSizeValue, double)
      SET_(POS_SIZE_LIMIT_TYPE, _posSizeLimitType, unsigned int)
      SET_(POS_SIZE_LIMIT_VALUE, _posSizeLimitValue, double)
      SET_(SLIPPAGE, _slippage, double)
      SET_(COMMISSION, _commission, double)
      SET_(WEIGHT, _weight, double)
      SET_(CROSSOVER, _crossover, double)
      SET_(POPULATION_SIZE, _populationSize, unsigned int)
      SET_(MAX_GENERATIONS, _maxGenerations, unsigned int)
      SET_(MUTATION_STRATEGY, _mutationStrategy, unsigned int)
      SET_(STEP, _step, unsigned int)
      SET_(LEAD_IN, _leadIn, unsigned int)
      SET_(OPTIMIZATION_PERIOD, _optimizationPeriod, unsigned int)
      SET_(WALK_FORWARD_PERIOD, _walkForwardPeriod, unsigned int)
      SET_(VARIABLE, _variables, std::vector<string>)
      SET_(SYSTEM, *_systems, std::vector<string>)
      SET_(PROCESSORS_COUNT, _processorsCount, unsigned int)
      SET_(STAT_TO_OPTIMIZE, _statToOptimize, size_t)
      SET_(STAT_GROUP_TO_OPTIMIZE, _statGroupToOptimize, size_t)
      SET_(WORST_ACCEPTABLE_RESULT, _worstAcceptableResult, double)

      return true;
    }
  } catch (const VariableException& e) {
    throw ConfigurationException(e.what());
  } catch (const tradery::DateException& e) {
    throw ConfigurationException(e.message());
  } catch (const RunTypeException&) {
    throw ConfigurationException("unknown run type");
  } catch (const ConfigurationException& e) {
    std::wstring newMessage(e.what());

    newMessage << "\n\nSupported command line arguments:\n" << _description;

    throw ConfigurationException(newMessage);
  } catch (const std::exception& e) {
    std::wstring newMessage(e.what());

    newMessage << "\n\nSupported command line arguments:\n" << _description;

    throw ConfigurationException(newMessage.c_str());
  }
}

void notice(std::ostream& os) {
  os << _T( "-------------------------------------------------" ) << std::endl
     << _T( "|               Tradery Optimizer               |" ) << std::endl
     << _T( "|          Copyright (C) 2018 Tradery           |" ) << std::endl
     << _T( "|            http://www.tradery.com             |" ) << std::endl
     << _T( "-------------------------------------------------" )
     << std::endl
     //		<< std::endl << getRegistrationInfoString() << std::endl
     << std::endl;
}
