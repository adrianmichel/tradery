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

#include <string>
#include <vector>
#include <tchar.h>
#include <assert.h>

#include <misc.h>
#include <exceptions.h>

/* @cond */
namespace tradery {
/* @endcond */

#ifdef MISCWIN_EXPORTS
#define MISCWIN_API __declspec(dllexport)
#else
#define MISCWIN_API __declspec(dllimport)
#endif

using namespace std;

#include <memory>

class RunProcessException {
 private:
  DWORD _errorCode;
  const std::string _processFileName;
  const std::string _cmdLine;

 public:
  RunProcessException(const std::string& processFileName, const std::string& cmdLine, DWORD errorCode)
      : _errorCode(errorCode), _processFileName( processFileName ), _cmdLine( cmdLine )  {}

  const DWORD errorCode() const { return _errorCode; }

  const std::string& processFileName() const { return _processFileName; }
  const std::string& cmdLine() const { return _cmdLine; }

  const std::string message() const {
    std::ostringstream o;

    o << "Attempting to run \"" << _processFileName << "\" with command line " << _cmdLine << " has failed with error " << _errorCode;
    return o.str();
  }
};

class RunProcessResult {
 private:
  bool _timeout;
  DWORD _exitCode;

 public:
  RunProcessResult(bool timeout, DWORD exitCode)
      : _timeout(timeout), _exitCode(exitCode) {}

  bool timeout() const { return _timeout; }
  DWORD exitCode() const { return _exitCode; }
};

using StrPairVector = std::vector<std::pair<std::string, std::string> > ;

class Environment {
 private:
  mutable bool _valid;
  mutable std::string _envStr;

  StrVector _envStrings;

 public:
  Environment() {}

  Environment(const Environment& env) {
    _envStrings = env._envStrings;
    _valid = env._valid;
    _envStr = env._envStr;
  }

  Environment(const StrPairVector& env) : _valid(false) {
    for (auto envEntry : env ) {
      add(envEntry.first, envEntry.second);
    }
  }

  Environment(const StrVector& env) : _valid(false) { _envStrings = env; }

  void add(LPCSTR name, LPCSTR value) {
    add(std::string(name), std::string(value));
  }

  void add(const std::string& name, const std::string& value) {
    _valid = false;
    _envStrings.push_back(name + '=' + value);
  }

  operator LPVOID() const {
    return _envStr.empty() && _envStrings.empty() ? nullptr : (LPVOID)get().data();
  }

  const std::string& get() const {
    if (!_valid) {
      for (auto env : _envStrings) {
        _envStr += env + '\0';
      }

      _envStr += '\0';
      _valid = true;
    }

    return _envStr;
  }

  const std::string toString() const {
    std::string str;
    for (auto env : _envStrings) {
      str += env + "\n";
    }

    return str;
  }
};

using EnvironmentPtr = std::shared_ptr<Environment>;

MISCWIN_API const RunProcessResult runProcess(const std::string& processFileName, const std::string& cmdLine, bool waitForProcess, 
  const std::string* startingDirectory = 0, const Environment& env = Environment(), unsigned long timeout = 0);
}
