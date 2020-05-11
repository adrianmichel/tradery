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

/* @cond */
namespace tradery {
/* @endcond */
/**
 * SimplePlugin is a base class for plug-ins that do not require require
 * configuration persistance and generate configuration objects on the fly when
 * requested.
 *
 * An example of such a plug-in is a Runnable plug-in that containsa number of
 * fixed Runnable derived classes, which have all their startup parameters
 * hardcoded inside the default constructor.
 *
 * The template parameter is the configuration type, such as Runnable
 */
template <class ConfigBase>
class SimplePlugin : public Plugin<ConfigBase> {
 protected:
  static const Info info;

  using XMap = PtrMap<UniqueId, ConfigBase>;
  using PluginType = Plugin<ConfigBase>;
  XMap _x;
  mutable typename XMap::const_iterator _i;

 public:
  /**
   * Pure virtual method - must be implemented in derived classes.
   *
   * Does the plug-in startup initialization by inserting all the available
   * configuration in an internal collection. The method insert is called to do
   * the actual initialization
   *
   * \code
   * virtual void RunnablePluginImpl::init()
   * {
   *     insert< System1 >();
   *     insert< System2 >();
   *     insert< System3 >();
   *     insert< System4 >();
   * }
   * \endcode
   *
   * @see SimplePlugin::insert
   */
  virtual void init() = 0;

 public:
  /**
   * Constructor - takes the plugin Info as parameter
   *
   * @param info
   */
  SimplePlugin(const Info& info) : PluginType(info) {}

  virtual ~SimplePlugin() {}
  /**
   * The info for the first available configuration, if any
   *
   * Together with SimplePlugin::next, used to traverse the list of all
   * available configurations
   *
   * @return pointer to the configuration info, or 0 if no configuration
   * available
   * @see SimplePlugin::next
   */
  virtual InfoPtr first() const {
    _i = _x.begin();
    return next();
  }
  /**
   * The info for the next available configuration, if any.
   *
   * Together with SimplePlugin::first, used to traverse the list of all
   * available configurations
   *
   * @return pointer to the configuration info, or 0 if no more configurations
   * available
   * @see SimplePlugin::first
   */
  virtual InfoPtr next() const {
    return _i != _x.end() ? std::make_shared< Info >(*(_i++->second)) : InfoPtr{};
  }

  /**
   * Gets a std::shared_ptr of the configuration with the specified id
   *
   * @param id     The configuration id
   *
   * @return std::shared_ptr to configuration, or 0 if id not found
   */
  virtual std::shared_ptr<ConfigBase> get(
      const UniqueId& id, const std::vector<std::string>* createStrings = 0) {
    XMap::iterator i = _x.find(id);
    std::shared_ptr< ConfigBase > cb = i->second;
    return i != _x.end() ? dynamic_cast< Clonable<ConfigBase>* >( cb.get() )->clone( createStrings ) : 0;
  }

  /**
   * By default, plug-ins derived from SimplePlugin can't create new
   * configurations
   *
   * @return false
   */
  virtual bool canCreate() const { return false; }
  /**
   *  By default, plug-ins derived from SimplePlugin can't create new
   * configurations
   *
   * @return 0
   */
  virtual std::shared_ptr<ConfigBase> create(
      const std::vector<std::string>* createStrings = 0) {
    return 0;
  }
  /**
   * By default, plug-ins derived from SimplePlugin can't edit configurations
   *
   * @param id     The id of the configuration to be edited
   *
   * @return false
   */
  virtual bool canEdit(const UniqueId& id) const { return false; }
  /**
   * By default, plug-ins derived from SimplePlugin can't edit configurations
   *
   * @param id     the id of the configuration to be edited
   *
   * @return 0
   */
  virtual std::shared_ptr<ConfigBase> edit(const UniqueId& id) { return 0; }
  /**
   * By default, plug-ins derived from SimplePlugin can't remove configurations
   *
   * @param id     The id of the configuration to be removed
   *
   * @return false
   */
  virtual bool canRemove(const UniqueId& id) const { return false; }
  /**
   * By default, plug-ins derived from SimplePlugin can't remove configurations
   *
   * @param id     The id of the configuration to be removed
   */
  virtual void remove(const UniqueId& id) {
    throw PluginException(name(), "This plug-in can not remove configurations");
  }
  /**
   * Called to add a new configuration to the list of available configurations
   *
   * The Config template parameter represents the actual type of the
   * configuration, so for a class TradingSystem1:
   *
   * \code
   * insert< TradingSystem1 >();
   * \endcode
   *
   * @param Config The configuration type
   */
  template <class Config>
  void insert() {
    ConfigBase* p = new Config();
    _x.insert(XMap::value_type(p->id(), p));
  }
};

}  // namespace tradery