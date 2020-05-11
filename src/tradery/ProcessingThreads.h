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

#include "ProcessingThread.h"

class ProcessingThreadsException {};

class ProcessingThreads {
 private:
   using IdToProcessingThreadMap = std::map<UniqueId, std::shared_ptr<ProcessingThread> >;

  IdToProcessingThreadMap _map;
  mutable std::mutex _mutex;

  static ProcessingThreads processingThreads;

 public:
  ProcessingThreads() {}

  // will remove non-running elements in the map
  void clean();

  /*  void cancel( const UniqueId& id )
  {
  Lock lock( _mutex );
  IdToProcessingThreadMap::iterator i = _map.find( id );

  if( i != _map.end() && i->second->isRunning() )
  i->second->cancel();
  }
  */
  static void run(const Configuration& config);
  void addAndRunAsync(const UniqueId& id, const Configuration& context);
};
