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

#include "runsystem.h"
#include "ProcessingThreads.h"

PluginTree globalPluginTree;
ConfigurationPtr _config;

PluginTree& getGlobalPluginTree() { return globalPluginTree; }

const Configuration& getConfig() { return *_config; }
void setConfig(ConfigurationPtr config) { _config = config; }

int buildRunnables(const Configuration& config) {
  ProcessingThreads::run(config);
  return 0;
}

int runSimulator(const Configuration& config) {
  int nRetCode = CoreErrorCode::success;

  LOG(log_debug, "tradery.cpp in simulator");
  std::string cmdLineStr = config.getCmdLineString();

  LOG(log_debug, "config: ", config.getCmdLineString());
  try {
    LOG(log_debug, "before run system");

    RunSystem runSystem(config);
    runSystem.run();
    LOG(log_debug, "after run system");

    nRetCode = success;
  }
  catch (const RunSystemException & e) {
    LOG(log_debug, "run system exception: ", e.errorCode());
    nRetCode = e.errorCode();
  }

  return nRetCode;
}

class InitUninit {
public:
  InitUninit() {
    tradery::init(getConfig().cacheSize());
  }

  ~InitUninit() {
    tradery::uninit();
  }
};
int run() {
  getGlobalPluginTree().clear();
  getGlobalPluginTree().explore(getConfig().getPluginPath(), getConfig().getPluginExt(), false, 0);

  InitUninit init;

  return getConfig().runSimulator() ? runSimulator(getConfig()) : buildRunnables(getConfig());
}