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

/*
IMPORTANT DISCLAIMER

These sample trading systems are included for illustrative purpose only.

Using them with real money can lead to substantial financial losses.

DO NOT USE FOR REAL MONEY TRADING OR INVESTMENT DECISIONS.
*/

#include "stdafx.h"
#include "systemsplugin2.h"

using namespace tradery;

class RunnablePluginImpl : public SimplePlugin<Runnable> {
 public:
  RunnablePluginImpl()
      : SimplePlugin<Runnable>(
            Info("2944F9DD-D094-4f20-AFF1-EEB440B03DB4", "Test systems plugin",
                 "This plugin contains a series of test systems")) {}

  virtual void init() {
    insert<SystemTest0>();
    insert<SystemTest1>();
    insert<SystemTest2>();
    insert<SystemTest3>();
    insert<SystemTest4>();
    insert<SystemTest5>();
    insert<SystemTest6>();
    insert<SystemTest7>();
    insert<SystemTest8>();
    insert<SystemTest9>();
    insert<SystemTest10>();
    insert<SystemTest11>();
    insert<SystemTest12>();
    insert<SystemTest13>();
    insert<SystemTest14>();
    insert<SystemTest15>();
    insert<SystemTest16>();
    insert<SystemTest17>();
    insert<SystemTest18>();
    insert<SystemTest19>();
    insert<SystemTest20>();
    insert<SystemTest21>();
    insert<SystemTest22>();
    insert<SystemTest23>();
    insert<SystemTest24>();
    insert<SystemTest25>();
    insert<SystemTest26>();
    insert<SystemTest27>();
    insert<SystemTest28>();
    insert<SystemTest29>();
    insert<SystemTest30>();
    insert<SystemTest31>();
    insert<SystemTest32>();
    insert<SystemTest33>();
    insert<SystemTest34>();
    insert<SystemTest35>();
    insert<SystemTest36>();
    insert<SystemTest37>();
    insert<SystemTest38>();
    insert<SystemTest39>();
    insert<SystemTest40>();
    insert<SystemTest41>();
    insert<SystemTest42>();
    insert<SystemTest43>();
    insert<SystemTest44>();
    insert<SystemTest45>();
    insert<SystemTest46>();
    insert<SystemTest47>();
    insert<SystemTest48>();
    insert<SystemTest49>();
  }
};

RunnablePluginImpl rplugin;

PLUGIN_API tradery::RunnablePlugin* getRunnablePlugin() { return &rplugin; }

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
      break;
  }
  return TRUE;
}