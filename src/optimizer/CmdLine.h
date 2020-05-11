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

#pragma warning(disable : 4482)

#include <strings.h>
#include <datetime.h>
#include <boost/regex.hpp>
#include <misc.h>
#include <tokenizer.h>
#include <tchar.h>
#include <boost/foreach.hpp>
#include <stringformat.h>

#include "variables.h"
#include "optimizertraderysession.h"

typedef Settable<tradery::Date> DateSettableBase;

class DateSettable : public DateSettableBase {
 public:
  const DateSettable& operator=(const std::wstring& date) {
    if (!date.empty())
      __super::set(tradery::Date(date, tradery::DateFormat::us));
    else
      __super::set(tradery::Date());

    return *this;
  }

  std::wstring toString() const { return __super::getValue().toString(us); }
};

class RunTypeException {};

class RunType {
 public:
  enum Type {
    de_optimization,
    de_walk_forward,
    adrian_optimization,
    adrian_walk_forward,
    unknown
  };

 private:
  Type m_type;

 public:
  RunType() : m_type(unknown) {}

  RunType(const std::wstring& str) {
    if (tradery::to_lower_case(str) == "de_optimization")
      m_type = de_optimization;
    else if (tradery::to_lower_case(str) == "de_walkforward")
      m_type = de_walk_forward;
    else if (tradery::to_lower_case(str) == "adrian_optimization")
      m_type = adrian_optimization;
    else if (tradery::to_lower_case(str) == "adrian_walkforward")
      m_type = adrian_walk_forward;
    else
      throw RunTypeException();
  }

  Type type() const { return m_type; }
};

class DirectionException {};

class DirectionSettable : public Settable<bool> {
 public:
  const DirectionSettable& operator=(const std::wstring& direction) {
    const std::wstring d(tradery::to_lower_case(direction));
    if (d == "min" || d == "minimize")
      __super::set(true);
    else if (d == "max" || d == "maximize")
      __super::set(false);
    else
      throw DirectionException();

    return *this;
  }

  bool minimize() const { return __super::getValue(); }
  bool maximize() const { return !minimize(); }
};

typedef boost::shared_ptr<DirectionSettable> DirectionSettablePtr;

class Configuration {
 private:
  std::wstring _description;
  std::wstring _userName;
  std::wstring _password;
  std::wstring _server;
  double _initialCapital;
  size_t _maxOpenPos;
  unsigned int _posSizeType;
  double _posSizeValue;
  unsigned int _posSizeLimitType;
  double _posSizeLimitValue;
  Variables _variables;
  unsigned int _leadIn;
  DateSettable _startDate;
  DateSettable _endDate;
  unsigned int _optimizationPeriod;
  unsigned int _walkForwardPeriod;
  unsigned int _step;
  double _weight;
  double _crossover;
  unsigned int _populationSize;
  unsigned int _maxGenerations;
  unsigned int _mutationStrategy;

  SystemIdsPtr _systems;
  tradery::StringPtr _symbolsList;
  std::optional< double > _commission;
  std::optional< double > _slippage;
  RunType _runType;

  DirectionSettablePtr _directionToOptimize;

  size_t _statToOptimize;
  size_t _statGroupToOptimize;
  unsigned int _processorsCount;
  double _worstAcceptableResult;

 public:
  Configuration();
  bool process();

  std::wstring getUsage() const;

  void showUsage();
  void showError(const std::wstring& error);

  const std::wstring& userName() const { return _userName; }
  const std::wstring& password() const { return _password; }
  const std::wstring& server() const { return _server; }
  size_t maxOpenPos() const { return _maxOpenPos; }
  unsigned int posSizeType() const { return _posSizeType; }
  unsigned int posSizeLimitType() const { return _posSizeLimitType; }
  unsigned int leadIn() const { return _leadIn; }
  const Variables& variables() const { return _variables; }
  unsigned int populationSize() const { return _populationSize; }
  unsigned int maxGenerations() const { return _maxGenerations; }
  unsigned int mutationStrategy() const { return _mutationStrategy; }
  SystemIdsPtr systems() const { return _systems; }
  tradery::StringPtr symbolsList() const { return _symbolsList; }
  const DateSettable& startDate() const { return _startDate; }
  const DateSettable& endDate() const { return _endDate; }
  unsigned int optimizationPeriod() const { return _optimizationPeriod; }
  unsigned int walkForwardPeriod() const { return _walkForwardPeriod; }
  unsigned int step() const { return _step; }
  unsigned int processorsCount() const { return _processorsCount; }
  double initialCapital() const { return _initialCapital; }
  double posSizeValue() const { return _posSizeValue; }
  double posSizeLimitValue() const { return _posSizeLimitValue; }
  double weight() const { return _weight; }
  double crossover() const { return _crossover; }
  double commission() const { return _commission; }
  double slippage() const { return _slippage; }
  RunType::Type runType() const { return _runType.type(); }
  bool minimize() const { return _directionToOptimize->minimize(); }
  size_t statToOptimize() const { return _statToOptimize; }
  StatsValue::ValueType statGroupToOptimize() const {
    return (StatsValue::ValueType)_statGroupToOptimize;
  }
  double worstAcceptableResult() const { return _worstAcceptableResult; }

  tradery::StringPtr toString() const {
    tradery::StringPtr str(new std::wstring());

    (*str) += tradery::format("user name: %1%\n", userName());
    (*str) += tradery::format("password: %1%\n", password());
    (*str) += tradery::format("start date: %1%\n", startDate().toString());
    (*str) += tradery::format("end date: %1%\n", endDate().toString());
    (*str) += tradery::format("lead-in: %1%\n", leadIn());

    return str;
  }
};

class ConfigurationException : public std::exception {
 public:
  ConfigurationException(const std::string& message)
      : exception(message.c_str()) {}
};

typedef boost::shared_ptr<Configuration> CmdLinePtr;
