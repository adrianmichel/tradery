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

// disallowed keywords

#include "runtimeproj.h"

// disabled should be right before defines
#include "disabled.h"
#include "defines.h"
#include <windows.h>

class RunnablePluginImpl : public SimplePlugin<Runnable> {
 public:
  PLUGIN_INIT_METHOD

 public:
  RunnablePluginImpl()
      : SimplePlugin<Runnable>(Info("Sample systems plugin",
                                    "This plugin illustrates some of the "
                                    "techniques employed in writing plugins, "
                                    "and more specifically system (or "
                                    "Runnable) plugins")) {}
};

RunnablePluginImpl rplugin;

PLUGIN_API tradery::RunnablePlugin* getRunnablePlugin() { return &rplugin; }

//////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
      rplugin.init();
      break;
    case DLL_THREAD_ATTACH:
      break;
    case DLL_THREAD_DETACH:
      break;
    case DLL_PROCESS_DETACH:
      //		uninitRunnables();
      break;
  }
  return TRUE;
}
