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

#include "builderrorsparser.h"
#include "Configuration.h"
#include "TraderyProcess.h"

#pragma once

class ProcessingThread : public Thread {
 private:
  const Configuration& m_config;
  bool _cancel;

  class InstanceCounter {
    friend ProcessingThread;

   private:
    static std::atomic_uint32_t _instances;

   public:
    InstanceCounter() {
      ++_instances;
    }

    ~InstanceCounter() {
      assert(_instances > 0);
      --_instances;
    }

    static unsigned int getInstances() {
      return _instances.load();
    }
  };

 public:
  ProcessingThread(const Configuration& config);

  void cancel() {
    _cancel = true;

    while (isRunning()) Sleep(50);
  }

  void run(ThreadContext* context = 0);
};
