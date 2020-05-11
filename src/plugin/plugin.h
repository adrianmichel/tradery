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

#include <hinstance.h>
#include <stringformat.h>

//#define ENABLE_LOG_PLUGIN

#if defined(ENABLE_LOG_PLUGIN)
#define LOG_PLUGIN(x, ...) LOG(x, __VA_ARGS__)
#else
#define LOG_PLUGIN(x, ...)
#endif

class PluginMethodException : public PluginException {
 private:
  const std::string _methodName;

 public:
  PluginMethodException(const std::string& PluginName, const std::string& methodName)
      : _methodName(methodName), PluginException(PluginName, "Method exception") {}

  const std::string& getMethodName() const { return _methodName; }
};

class PluginInstanceException : public PluginException {
 public:
  PluginInstanceException(const std::string& pluginName)
      : PluginException(pluginName, "Plugin instance exception") {}
};

template <class T>
class PluginInstanceBase : protected HInstance {
 protected:
  using  GET_PLUGIN = Plugin<T>*(*)();
  Plugin<T>* _plugin;

 public:
  PluginInstanceBase(const std::string& path) : HInstance(path), _plugin(0) {}

  Plugin<T>* operator->() {
    assert(_plugin != 0);
    return _plugin;
  }

  Plugin<T>& operator*() {
    if (_plugin != 0) {
      return *_plugin;
    }
    else {
      throw PluginInstanceException(path());
    }
  }

  void setPlugin( const std::string& procName ) {
    GET_PLUGIN getPlugin = reinterpret_cast<GET_PLUGIN>(HInstance::getProcAddress(procName));
    LOG_PLUGIN(log_debug, "Calling getPlugin");
   _plugin = (*getPlugin)();
  }
};

/**
 * A generic Plugin instance
 * loads a specific plugin based on it's path (including file name)
 * calls the getPlugin method which each plugin must implement
 * if this method is not implemented, or the path doesn't point to a valid
 * module an exception is thrown
 */
template <class T>
class PluginInstance : public PluginInstanceBase<T> {
 public:
  // the complete path, including name and extension
  PluginInstance(const std::string& path)
    try
      : PluginInstanceBase <T>(path) {
        try {
            LOG_PLUGIN(log_debug, "Candidate plugin: ", path);
          __super::setPlugin( procName );
        }
        catch (const HInstanceMethodException& e) {
          LOG_PLUGIN(log_debug, "Likely not a plugin: ", e.path(), ", method \"", e.name(), "\" was not found, last error: ", e.getLastError());
        }
        catch (const HInstanceException& e) {
          LOG_PLUGIN(log_debug, "HInstanceException, path: ", e.path(), ", last error: ", e.getLastError());
          throw PluginException("", __super::path());
        }
    }
    catch (const HInstanceException& e) {
      throw PluginInstanceException( tradery::format( path, "last error: ", e.getLastError()));
    }

  using RELEASE_PLUGIN = void (*)();
  ~PluginInstance() {
    try {
      RELEASE_PLUGIN releasePlugin = reinterpret_cast<RELEASE_PLUGIN>(PluginInstanceBase< T >::getProcAddress(releaseProcName));
      (*releasePlugin)();
    }
    catch (const HInstanceMethodException&) {
    }
  }

 private:
  static const std::string procName;
  static const std::string releaseProcName;
};

template <> const std::string PluginInstance<SymbolsSource>::procName = "getSymbolsSourcePlugin";
template <> const std::string PluginInstance<Runnable>::procName = "getRunnablePlugin";
template <> const std::string PluginInstance<Slippage>::procName = "getSlippagePlugin";
template <> const std::string PluginInstance<Commission>::procName = "getCommissionPlugin";
template <> const std::string PluginInstance<DataSource>::procName = "getDataSourcePlugin";
template <> const std::string PluginInstance<SignalHandler>::procName = "getSignalHandlerPlugin";

template <> const std::string PluginInstance<SymbolsSource>::releaseProcName = "releaseSymbolsSourcePlugin";
template <> const std::string PluginInstance<Runnable>::releaseProcName = "releaseRunnablePlugin";
template <> const std::string PluginInstance<Slippage>::releaseProcName = "releaseSlippagePlugin";
template <> const std::string PluginInstance<Commission>::releaseProcName = "releaseCommissionPlugin";
template <> const std::string PluginInstance<DataSource>::releaseProcName = "releaseDataSourcePlugin";
template <> const std::string PluginInstance<SignalHandler>::releaseProcName = "releaseSignalHandlerPlugin";
