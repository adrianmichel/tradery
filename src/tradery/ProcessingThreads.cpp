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

#include <log.h>
#include "ProcessingThreads.h"

ProcessingThreads ProcessingThreads::processingThreads;
std::atomic_uint32_t ProcessingThread::InstanceCounter::_instances = 0;

void ProcessingThreads::run(const Configuration& config) {
  processingThreads.clean();
  // if the first char of the request is a *, than this is a file request from
  // a running simulation
  // otherwise is a command line to start a simulation
  //    std::cout << std::endl << "Before run" << std::endl;

  LOG(log_debug, config.getSessionId(), " get answer to request after cmdline");
  //  std::cout << cmdLine.getCmdLineString() << std::endl;
  // this is the session name
  // todo: handle uniqueid exception
  // the runnable name is the session id
  UniqueId runnableName(config.getSessionId());

  if (config.asyncRun()) {
    LOG(log_debug, config.getSessionId(), " starting async run");

    processingThreads.addAndRunAsync(runnableName, config);
    LOG(log_debug, config.getSessionId(), " started async run");
  }
  else {
    LOG(log_debug, config.getSessionId(), " starting sync run");
    ProcessingThread pt(config);
    LOG(log_debug, config.getSessionId(), " created processing thread");

    pt.startSync();
    LOG(log_debug, config.getSessionId(), " sync run completed");
  }
}

void ProcessingThreads::addAndRunAsync(const UniqueId& id, const Configuration& config) {
  std::scoped_lock lock(_mutex);

  std::shared_ptr<ProcessingThread> pt;

  for (IdToProcessingThreadMap::iterator i = _map.find(id); i != _map.end(); i = _map.find(id)) {
    // if the thread is still running, then cancel
    if (i->second->isRunning()) i->second->cancel();

    // clean the inactive processing threads, including this one
    clean();
  }

  pt = std::shared_ptr<ProcessingThread>(std::make_shared< ProcessingThread >(config));
  _map.insert(IdToProcessingThreadMap::value_type(id, pt));

  LOG(log_debug, config.getSessionId(), " async run");
  pt->start();
}

void ProcessingThreads::clean() {
  std::scoped_lock lock(_mutex);

  for (IdToProcessingThreadMap::iterator i = _map.begin(); i != _map.end();) {
    if (!i->second->isRunning()) {
      i = _map.erase(i);
    }
    else {
      i++;
    }
  }
}
