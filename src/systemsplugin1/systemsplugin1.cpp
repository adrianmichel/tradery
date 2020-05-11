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
#include <macros.h>
#include "systemsplugin1.h"
#include "system1.h"
#include "system2.h"
#include "system3.h"
#include "system4.h"
#include "system5.h"
#include "system6.h"
#include "system7.h"
#include "system8.h"
#include "system11.h"
#include "systemTemplate.h"
#include "bollingerbandsystem.h"
#include "exttradessystem.h"
#include "simpletradingsystem.h"
#include "mama.h"
#include "adaptivebbands.h"
#include "lindqlongshort.h"
#include "calendarsystem.h"

#include "IBCommission.h"
#include "sampleslippage.h"
#include "mfi.h"

class RunnablePluginImpl : public SimplePlugin<Runnable> {
 public:
  virtual void init() {
    insert<System1>();
    insert<System2>();
    insert<System3>();
    insert<System4>();
    insert<System5>();
    insert<System6>();
    insert<System7>();
    insert<System8>();
    insert<System11>();
    insert<BollingerBandSystem>();
    insert<ExtTradesSystem>();
    insert<SimpleTradingSystem>();
    insert<BollingerBandClassicSystem>();
    insert<MAMA_ttrcrep>();
    insert<AdaptiveBBands>();
    insert<LongShortKeltnerBands>();
    insert<MFISystem>();
    insert<CalendarSystem>();
  }

 public:
  RunnablePluginImpl()
      : SimplePlugin<Runnable>(Info(
            "cd365c65-91aa-11d9-b3a4-000c6e067d35", "Sample systems plugin",
            "This plugin illustrates some of the techniques employed in "
            "writing plugins, and more specifically system (or Runnable) "
            "plugins")) {}
};

///////////////////////////////////////////////////////////////////
// commission plugin
class SampleCommissionPlugin : public SimplePlugin<Commission> {
 public:
  virtual void init() { insert<IBCommission>(); }

  SampleCommissionPlugin()
      : SimplePlugin<Commission>(Info(
            "D47E020B-FFB2-46e5-97D5-11F23C3014A2", "Sample commission plugin",
            "Sample commission plugin - for the moment just an Interactive "
            "Brokers commission model")) {}
};

////////////////////////////////////////////////////////////////
// slippage plugin
class SampleSlippagePlugin : public SimplePlugin<Slippage> {
 public:
  virtual void init() { insert<SampleSlippage>(); }

  SampleSlippagePlugin()
      : SimplePlugin<Slippage>(Info("D5DD3FA7-52D2-45fc-9AAB-8F661CBF1FF1",
                                    "Sample slippage plugin",
                                    "Sample slippage plugin - for the moment "
                                    "just one very simple configuration")) {}
};

SampleCommissionPlugin cplugin;
SampleSlippagePlugin splugin;
RunnablePluginImpl rplugin;

PLUGIN_API tradery::SlippagePlugin* getSlippagePlugin() { return &splugin; }

PLUGIN_API tradery::CommissionPlugin* getCommissionPlugin() { return &cplugin; }

PLUGIN_API tradery::RunnablePlugin* getRunnablePlugin() { return &rplugin; }

//////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
      rplugin.init();
      cplugin.init();
      splugin.init();
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
