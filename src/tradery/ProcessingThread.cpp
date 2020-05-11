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

#include "RunnablePluginBuilder.h"
#include "ProcessingThread.h"
#include "runsystem.h"
#include <path.h>

#define PROCESSING_THREAD "ProcessingThread"

void ProcessingThread::run(ThreadContext* threadContext) {
  LOG(log_info, m_config.getSessionId(), "in ProcessingThread run");
  std::wstring signalFile;
  try {
    // first build

    SessionResult sessionResult = SessionResult::normal;

    RunnablePluginBuilder builder(m_config, _cancel);
    LOG(log_info, m_config.getSessionId(), " In run, after builder - ", (builder.success() ? "success" : "failure"));

    if (builder.success()) {
      try {
        RunSystem runSystem(m_config);
        runSystem.run();
      } 
      catch (const RunSystemException& e) {
        LOG(log_debug, "run system exception: ", e.message(), ", error code: ", e.errorCode());
      }
    }
    else {
      sessionResult = SessionResult::failed;
    }

    if (m_config.hasEndRunSignalFile()) {
      LOG(log_info, "****Writing end file: ", m_config.endRunSignalFile());
      // todo - handle file error
      std::ofstream ofs(m_config.endRunSignalFile().c_str());

      LOG(log_info, "done!");
      ofs << sessionResult << std::endl;
    }
  }
  catch (const RunnablePluginBuilderException & e) {
    LOG(log_error, "RunnablePluginBuilderException: ", e.what());
  }
  catch (const RunProcessException& e) {
    LOG(log_error, "RunProcessException: ", e.message());
  }
  catch (ConfigurationException& e) {
    LOG(log_error, "ConfigurationException: ", e.what());
  }
}


SessionResult timeoutHandler(const Configuration& config, bool& _cancel,
                             const SessionController& sessionController) {
  SessionResult status = SessionResult::normal;

  Timer heartBeatTimer;
  Timer reverseHeartBeatTimer;

  for (bool run = true; run;) {
    if (!sessionController.isActive()) {
      // logging
      LOG(log_info, PROCESSING_THREAD, "Session ", config.getSessionId(), " process has finished processing, exiting");
      break;
    }

    if (Path{ config.heartBeatFile() }.exists()) {
      LOG(log_debug, "process - hearbeat event");
      DeleteFile(s2ws(config.heartBeatFile()).c_str());
      heartBeatTimer.restart();
    }
    else if (heartBeatTimer.elapsed() > config.heartBeatTimeout()) {
      LOG(log_info, "heart beat elapsed: ", heartBeatTimer.elapsed());

      LOG(log_info, "Session ", config.getSessionId(), " has not received heart beat signal within ", config.heartBeatTimeout(), " seconds, terminating session");
      // no heartbeat within the heartbeat time out, cancel run
      sessionController.terminate();
      status = SessionResult::timeout;
      break;
    }

    bool fe = Path{ config.cancelFile() }.exists();
    if (fe || _cancel) {
      // received a cancel signal
      LOG(log_info, PROCESSING_THREAD, "Session ", config.getSessionId(), " received cancel signal through ", (fe ? "cancel file" : "cancel method call"));

      sessionController.terminate();
      status = SessionResult::cancel;
      break;
    }

    if (reverseHeartBeatTimer.elapsed() > config.reverseHeartBeatPeriod() && !Path{ config.reverseHeartBeatFile() }.exists()) {
      LOG(log_debug, "reverse heartbeat");
      ofstream rhb(config.reverseHeartBeatFile().c_str());
      rhb << "reverse heart beat";
      reverseHeartBeatTimer.restart();
    }
    Sleep(50);
  }

  LOG(log_debug, "returning ProcessResult");
  return status;
}

ProcessingThread::ProcessingThread(const Configuration& config)
try
    : m_config( config), Thread("Processing thread"), _cancel(false) {
  LOG(log_debug, "in Processingthread constructor");
}
catch (const ConfigurationException& e) {
  LOG(log_error, "ConfigurationException: ", e.what());
}
catch (const std::exception& e) {
  LOG(log_error, "std::exception: ", e.what());
}
catch (...) {
  LOG(log_info, "unknown exception");
  //	std::cout << context->getCmdLineString() << std::endl;
}
