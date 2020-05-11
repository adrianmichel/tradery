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

#include "Configuration.h"

enum SessionResult {
  normal,
  cancel,
  timeout,
  failed,
};

class ProcessResult {
 private:
  SessionResult _status;
  DWORD _exitCode;

 public:
  ProcessResult(SessionResult status, DWORD exitCode)
      : _status(status), _exitCode(exitCode) {}

  DWORD exitCode() const { return _exitCode; }
  SessionResult status() const { return _status; }
};

class SessionController {
 public:
  virtual ~SessionController() {}
  virtual void terminate() const = 0;
  virtual bool isActive() const = 0;
};

class ProcessSessionController : public SessionController {
 private:
  const HANDLE _hProcess = 0;
  const HANDLE _hThread = 0;

 public:
  ProcessSessionController(HANDLE hProcess, HANDLE hThread)
      : _hProcess(hProcess), _hThread(hThread) {}

  virtual ~ProcessSessionController() {
    LOG(log_debug, "Closing handles");
    if (_hProcess != 0) CloseHandle(_hProcess);
    if (_hThread != 0) CloseHandle(_hThread);
  }

  virtual void terminate() const { TerminateProcess(_hProcess, 0); }

  virtual bool isActive() const {
    return WaitForSingleObject(_hProcess, 0) != WAIT_OBJECT_0;
  }

  DWORD getExitCode() const {
    DWORD exitCode;
    GetExitCodeProcess(_hProcess, &exitCode);
    LOG(log_debug, "Process exit code: ", exitCode);
    return exitCode;
  }
};

SessionResult timeoutHandler(const Configuration& config, bool& _cancel, const SessionController& sessionController);

const ProcessResult process(const Configuration& config, bool& _cancel, const std::string& processFileName,
                            const std::string& cmdLine, const std::string* startingDirectory, const Environment& env);
