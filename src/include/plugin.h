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

#ifdef PLUGIN_EXPORTS
#define PLUGIN_API __declspec(dllexport)
#else
#define PLUGIN_API __declspec(dllimport)
#endif

#include "exceptions.h"
#include <set>

/**
 * \defgroup Plugin Plugin classes and APIs
 * '
 * For several complete examples of plugins, using persistence in the registry
 * or not, see the sample projects
 */
/*@{*/
namespace tradery {

/**
 * plugin info: info (id, name, description) + version
 */
class PluginInfo : public Info {
 private:
  const Version _version;

 public:
  /**
   * Copy constructor
   *
   * @param pluginInfo The source PluginInfo
   *
   * @see Info
   * @see Version
   */
  PluginInfo(const PluginInfo& pluginInfo)
      : _version(pluginInfo.version()), Info(pluginInfo) {}
  /**
   * Constructor - takes a reference to a Info object
   *
   * The version is set by default to the current version of the product
   *
   * @param info   The source info
   *
   * @see Info
   * @see Version
   */
  PluginInfo(const Info& info) : Info(info) {}

  /**
   * Returns the plugin version
   *
   * @return The plugin version
   * @see Version
   */
  const Version& version() const { return _version; }
};

/**
 * Abstract base class for all plugin types and instances.
 *
 * Implemented as a template class to allow different type of plugins.
 * Also, implemented as an abstract class to allow for specific implementations
 * of various plugins
 *
 * A plugin must derive from this class and implement its various virtual
 * methods that will define its behavior. The plugin type is determined by the
 * type passed as template argument.
 *
 * The template argument is the type of the plugin. For example, a data source
 * plugin is defined as:
 *
 * \code
 * typedef Plugin< DataSource > DataSourcePlugin;
 * \endcode
 *
 * To be useful, a plugin must be able to instantiate at least one
 * configuration, which is an object of the type of the plugin. Most plugins
 * will be able to create, edit and delete multiple configurations as well as
 * persist them.
 *
 * For example a DataSource plugin will be able to create instances of
 * DataSource derived classes and pass them as pinters when requested.
 *
 * To traverse the list of available configuration, use first/next.
 */
template <class Type>
class Plugin : public PluginInfo {
 public:
  /**
   * Constructor - takes a reference to an Info object
   *
   * @param info   The info for the current plugin
   */
  Plugin(const Info& info) : PluginInfo(info) {}

  /**
   * Returns a pointer to a Info about the first available configuration
   *
   * @return A smart pointer to an Info object, or 0 if no configurations are
   * available
   * @exception PluginException
   *                   Thrown in case of an error
   * @see Info
   */
  virtual InfoPtr first() const = 0;
  /**
   * Returns a pointer to an Info object for the next available configuration
   *
   * @return A smart pointer to an Info object, or 0 if no more configurations
   * are available
   * @exception PluginException
   *                   Thrown in case of an error
   * @see Info
   */
  virtual InfoPtr next() const = 0;
  /**
   * Gets an instance of a configuration, usually as a pointer or smart pointer.
   *
   * @param id     The id of the configuration
   *
   * @return The configuration pointer, or 0 if could not get
   * @exception PluginException
   *                   Thrown in case of an error
   * @see UniqueId
   * @see Info
   */
  virtual std::shared_ptr<Type> get(const UniqueId& id, const std::vector<std::string>* createStrings = 0) = 0;
  /**
   * Indicates the configuration capabilities of the plugin
   *
   * If the plugin is capable of creating new configurations, it will return
   * true
   *
   * @return true if can create new configurations, false otherwise
   */
  virtual bool canCreate() const = 0;
  /**
   * creates a new configuration
   *
   * For this method to be called, the canCreate method must return true,
   * indicating that plug-in creation is supported
   *
   * Internally, a plugin can call a dialog box to set parameters, persist the
   * configuration in the registry or do anything that it needs to, the only
   * requirement being that at the end it returns a pointer to a new
   * configuration or 0 if it didn't or couldn't create it.
   *
   * A plug-in configuration can be passed a string that can contain runtime
   * information such as a list of symbols for a symbolssource or other values
   *
   * @param createString
   *               A configuration defined string passed at creation. The
   * default value is an empty string This string can be any value that the
   * plug-in configuration will know how to interpret and handle.
   *
   * @return The pointer to the new configuration, or 0 if no configuration has
   * been created
   * @exception PluginException
   *                   Thrown in case of an error
   * @see UniqueId
   */
  virtual std::shared_ptr<Type> create(const std::vector<std::string>* createStrings = 0) = 0;
  /**
   * Determines if a specific configuration can be edited
   *
   * @param id     The id of the configuration that is tested
   *
   * @return true if the configuration can be edited, false otherwise
   * @see UniqueId
   */
  virtual bool canEdit(const UniqueId& id) const = 0;
  /**
   * Edits a configuration
   *
   * For this method to be called, the canEdit method must return true
   *
   * Internally, a plugin can call a dialog box to change the configuration's
   * parameters, persist them in the registry or do anything that it needs to,
   * the only requirement being that at the end it returns a pointer to the
   * edited configuration
   *
   * @param id     The id of the configuration to be edited
   *
   * @return the edited configuration
   * @exception PluginException
   *                   Thrown in case of an internal error
   * @see UniqueId
   */
  virtual std::shared_ptr<Type> edit(const UniqueId& id) = 0;
  /**
   * Determines if a specific configuration can be removed
   *
   * @param id     The id of the configuration to be removed
   *
   * @return true if it can be removed, false if not
   * @see UniqueId
   */
  virtual bool canRemove(const UniqueId& id) const = 0;
  /**
   * Removes a configuration
   *
   * For this method to be called, the canRemove method must return true
   *
   * If the configuration can't be removed, an exception should be thrown
   * indicating the reason
   *
   * @param id     The id of the configuration to be removed
   *
   * @exception PluginException
   *                   Thrown in case of an error, for example of the
   * configuration can't be removed
   * @see UniqueId
   */
  virtual void remove(const UniqueId& id) = 0;

  /**
   * Indicates whether a plug-in configuration corresponding to a id is UI
   * enabled
   *
   * The default implementation returns false. If the plug-in configuration id
   * can create its own UI, this method should be overriden and it should return
   * true for that id. In that case, the plug-in configuration will be requested
   * to provide more information about its UI - see the PluginConfiguration
   * class.
   *
   * @param id     The id of the configuration to be checked for UI
   *
   * @see UniqueId
   * @see PluginConfiguration
   */
  virtual bool hasWindow(const UniqueId& id) const { return false; }
};

////////////////////////////////////
// Plugin APIs
//

class PluginTreeException {
 public:
  std::vector<InfoPtr > _info;

 public:
  PluginTreeException() {}
  PluginTreeException(InfoPtr info) { add(info); }
  PluginTreeException(const std::vector<InfoPtr >& info) {
    _info.insert(_info.begin(), info.begin(), info.end());
  }

  void add(InfoPtr infoPtr) { _info.push_back(infoPtr); }

  const std::vector<InfoPtr >& info() const { return _info; }
  bool empty() const { return _info.empty(); }
  const std::string message() const {
    std::ostringstream o;
    o << "Duplicate ids, the components with these Ids will be ignored: " << std::endl;

    for (auto v : _info) {
      o << v->id().str() << std::endl;
    }

    return o.str();
  }
};

class PluginLoadingStatusHandler {
 public:
  virtual ~PluginLoadingStatusHandler() {}

  virtual void event(const std::string& event) = 0;
  virtual void done() = 0;
};

class NullPluginLoadingStatusHandler : public PluginLoadingStatusHandler {
  virtual void event(const std::string& event) {}
  virtual void done() {}
};

class PluginExplorer {
 private:
  virtual void process(const std::string& path, PluginLoadingStatusHandler* loadingStatusHandler, std::vector<InfoPtr>& duplicates) = 0;

  static void initIgnoreModulesSet(std::set<std::string>& ignoreModulesSet);

  static bool ignoreModule(const std::string& _fileName);

 public:
  // searches and loads all plug-ins in several paths
  void explore(const std::vector<std::string>& paths, const std::string& ext, bool recursive, PluginLoadingStatusHandler* loadingStatusHandler) {
    std::vector<std::shared_ptr<Info> > duplicates;
    for (auto path : paths) {
      explore(path, ext, recursive, loadingStatusHandler, duplicates);
    }

    if (!duplicates.empty()) throw PluginTreeException(duplicates);
  }

  void explore(const std::string& path, const std::string& ext, bool recursive, PluginLoadingStatusHandler* loadingStatusHandler) {
    std::vector<std::string> paths;
    paths.push_back(path);

    explore(paths, ext, recursive, loadingStatusHandler);
  }

 private:
  // search and loads all plug-ins in one path
  void explore(const std::string& p, const std::string& ext, bool recursive, PluginLoadingStatusHandler* loadingStatusHandler, std::vector<InfoPtr>& duplicates);
};

}  // namespace tradery

// end Plugin group
/*@}*/
