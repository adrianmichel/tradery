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

#include "pch.h"
#include "Configuration.h"
#include "strings.h"
#include <tradery.h>
#include <memory>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::shared_ptr<Log> _log;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[]) {
  LOG(log_debug, "start");
  try {
    setConfig(std::make_shared<Configuration>());

    if (getConfig().help()) {
      return success;
    }

    return run();
  }
  catch (const ConfigurationException & e) {
    LOG(log_debug, "Exiting with ConfigurationException: ", e.what());
    return config_error;
  }
  catch (exception & e) {
    LOG(log_debug, "Exiting with exception: ", e.what());
    return unknown_error;
  }
  catch (...) {
    LOG(log_debug, "Unknown error");
    return unknown_error;
  }
}


