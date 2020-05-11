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

#include "plugin.h"
#include "datasource.h"
#include "core.h"

namespace tradery {

/**
 * \defgroup PluginTypes Plugin types
 *
 * The plugin types currently supported
 * @{
 */

using DataSourcePlugin = Plugin<DataSource>;
using SymbolsSourcePlugin = Plugin<SymbolsSource>;
using RunnablePlugin = Plugin<Runnable>;
using SlippagePlugin = Plugin<Slippage>;
using CommissionPlugin = Plugin<Commission>;
using SignalHandlerPlugin = Plugin<SignalHandler>;

// end plugin types
/*@}*/

}  // namespace tradery

extern "C" {

/**
 * \defgroup PluginNonMember Non member plugin APIs
 * '
 * For a plugin to be discovered at runtime, it must implement one of the get...
 * methods
 *
 * The release... methods are optional, and can be used for resource cleanup
 * when the plugin is to be released
 */
/*@{*/
PLUGIN_API tradery::DataSourcePlugin* getDataSourcePlugin();
PLUGIN_API tradery::SymbolsSourcePlugin* getSymbolsSourcePlugin();
PLUGIN_API tradery::RunnablePlugin* getRunnablePlugin();
PLUGIN_API tradery::SlippagePlugin* getSlippagePlugin();
PLUGIN_API tradery::CommissionPlugin* getCommissionPlugin();
PLUGIN_API tradery::SignalHandlerPlugin* getSignalHandlerPlugin();

PLUGIN_API void releaseDataSourcePlugin();
PLUGIN_API void releaseSymbolsSourcePlugin();
PLUGIN_API void releaseRunnablePlugin();
PLUGIN_API void releaseSlippagePlugin();
PLUGIN_API void releaseCommissionPlugin();
PLUGIN_API void releaseSignalHandlerPlugin();
/*@}*/
}
