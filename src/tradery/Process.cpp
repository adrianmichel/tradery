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
#include "TraderyProcess.h"

const ProcessResult process(const Configuration& config, bool& _cancel, const std::string& processFileName,
                            const std::string& cmdLine, const std::string* startingDirectory, const Environment& env) {
  try {
    LOG(log_debug, "\tprocess file name: ", processFileName);
    LOG(log_debug, "\tcmd line: ", cmdLine);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Run the autoupdate program.
    if (CreateProcess(
            s2ws(processFileName).c_str(),
            const_cast<LPTSTR>(s2ws(cmdLine).c_str()),  // Command line.
            NULL,  // Process handle not inheritable.
            NULL,  // Thread handle not inheritable.
            TRUE,  // Set handle inheritance to FALSE.
            0,     // No creation flags.
            env,   // Use parent's environment block.
            startingDirectory != 0 ? s2ws(*startingDirectory).c_str()
                                   : 0,  // Use parent's starting directory.
            &si,                         // Pointer to STARTUPINFO structure.
            &pi)  // Pointer to PROCESS_INFORMATION structure.
    ) {
      LOG(log_info, "Process \"", cmdLine, "\" created, hProcess: ", pi.hProcess);
      ProcessSessionController psc(pi.hProcess, pi.hThread);
      SessionResult status = timeoutHandler(config, _cancel, psc);
      return ProcessResult(status, psc.getExitCode());
    }
    else {
      LOG(log_info, "ProcessCreate failed, GetLastError: ", GetLastError());
      throw RunProcessException(processFileName, cmdLine, GetLastError());
    }
  }
  catch (...) {
    LOG(log_info, "unknown exception, GetLastError: ", GetLastError());
    throw RunProcessException(processFileName, cmdLine, GetLastError());
  }
}
