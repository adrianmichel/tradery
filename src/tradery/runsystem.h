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

#include <common.h>

#define TRACE_RUNSYSTEM

class ErrorEventSinkImpl : public tradery::ErrorEventSink {
private:
  using ErrorEventVector = std::vector<ErrorEventPtr >;
  ErrorEventVector _events;
  mutable std::mutex _m;
  RuntimeStats& _errorsCounter;

public:
  ErrorEventSinkImpl(RuntimeStats& errorsCounter)
    : _errorsCounter(errorsCounter) {}

  virtual void push(const ErrorEventPtr event) {
    std::scoped_lock lock(_m);
    _events.push_back(event);
    _errorsCounter.incErrors();
    // std::cout << event.toString() << std::endl;
  }
  void pop() override {
    std::scoped_lock lock(_m);
    if (!_events.empty()) _events.pop_back();
  }
  ErrorEventPtr front() const override {
    std::scoped_lock lock(_m);
    return _events.empty() ? 0 : _events.back();
  }
  virtual bool empty() const {
    std::scoped_lock lock(_m);
    return _events.empty();
  }
  virtual size_t size() const {
    std::scoped_lock lock(_m);
    return _events.size();
  }

  void toHTML(std::ostream& os) const {
    std::scoped_lock lock(_m);
    os << "<table class=\"list_table\">" << std::endl;
    os << "<tr class='h'><td class='h'></td><td class='h'>Type</td><td "
      "class='h'>Symbol</td><td class='h'>System</td><td "
      "class='h'>Message</td></tr>"
      << std::endl;

    unsigned __int64 count = 0;

    for (ErrorEventVector::const_iterator i = _events.begin(); i != _events.end(); i++, count++) {
      std::string typeClass;
      if ((*i)->category() == ErrorEvent::Category::error) {
        typeClass = "errorLine";
      }
      else if ((*i)->category() == ErrorEvent::Category::warning) {
        typeClass = "warningLine";
      }

      os << "<tr class=\"" << (count % 2 ? "d0 " : "d1 ") << "\">" << std::endl;
      os << "<td style='font-weight: bold' class='" << typeClass << "'>" << (*i)->categoryToString() << "</td>" << std::endl;
      os << "<td class='" << typeClass << "'>" << (*i)->typeToString() << "</td>" << std::endl;
      os << "<td class='" << typeClass << "'>" << (*i)->symbol() << "</td>" << std::endl;
      os << "<td class='" << typeClass << "'>" << (*i)->systemName() << "</td>" << std::endl;
      os << "<td class='" << typeClass << "'>" << (*i)->message() << "</td>" << std::endl;
      os << "</tr>" << std::endl;
    }

    os << "</table>" << std::endl;
  }
};

enum CoreErrorCode {
  success,
  build_error,
  system_run_error,
  process_run_error,
  cmd_line_error,
  config_error,
  mfc_init_error,
  unknown_error
};

class RunSystemException {
 private:
  CoreErrorCode _errorCode;
  const std::string _message;

 public:
  RunSystemException(CoreErrorCode errorCode, const std::string& message)
      : _errorCode(errorCode), _message(message) {}

  CoreErrorCode errorCode() const { return _errorCode; }
  const std::string& message() const { return _message; }
};

class RunSystem {
 private:
  const Configuration& m_config;

private:
  void saveTradesDescriptionFile(const PositionsContainer& pos) const;
  void saveTradesCSVFile(const PositionsContainer& pos) const;
  void saveErrorsFile(const ErrorEventSinkImpl& errsink) const;
  void saveSymbolTimeoutError() const;
  void saveMaxBarsExceededError( long totalBarCount ) const;

 public:
  RunSystem(const Configuration& config);

  void run();

  ~RunSystem() {}
};
