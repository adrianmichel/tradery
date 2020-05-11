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

#include "tree.h"
#include "plugintree.h"

// T is the node type (Node or DocumentNode )
template <class T>
class Event {
 public:
  enum Type { add, remove, change };

 private:
  const Type _type;
  const T* _node;
  const UniqueId* _id;
  const UniqueId* _parentId;
  const bool _ensureVisible;

 public:
  Event(Type type, const T* node, const UniqueId* id, const UniqueId* parentId,
        bool ensureVisible)
      : _type(type),
        _node(node),
        _parentId(parentId),
        _ensureVisible(ensureVisible),
        _id(id) {}
  Type type() const { return _type; }
  const T* node() const { return _node; }
  const UniqueId* parentId() const { return _parentId; }
  bool ensureVisible() const { return _ensureVisible; }
  const UniqueId* id() const { return _id; }
};

template <class T>
class View {
 public:
  virtual ~View() {}

  virtual void update(const Event<T>& event) = 0;
  virtual bool allowRemove(const UniqueIdPtrVector& ids) const = 0;
  virtual bool allowChange(const UniqueIdPtrVector& ids) const = 0;
};

class Command {};

class Node : public Info {
 public:
  enum NodeType {
    MIN_NODE_TYPES,
    DATASOURCE,
    SYMBOLSSOURCE,
    RUNNABLE,
    SLIPPAGE,
    COMMISSION,
    SIGNALHANDLER,
    MAX_NODE_TYPES
  };

  enum NodeSubtype {
    MIN_NODE_SUBTYPES,
    CONFIG,
    PLUGIN,
    ROOT,
    MAX_NODE_SUBTYPES
  };

 public:
  Node(const Info& info) : Info(info) {}

  virtual ~Node() {}
  virtual NodeType type() const = 0;
  virtual NodeSubtype subtype() const = 0;

  static const std::string typeToString(NodeType type) {
    switch (type) {
      case DATASOURCE:
        return "Data Source";
      case SYMBOLSSOURCE:
        return "Symbols Source";
      case RUNNABLE:
        return "Runnable";
      case SLIPPAGE:
        return "Slippage";
      case COMMISSION:
        return "Commission";
      case SIGNALHANDLER:
        return "Signal";
      default:
        assert(false);
        return "";
    }
  }
};

template <Node::NodeType t>
class NodeBase : public Node {
 public:
  NodeBase(const Info& info) : Node(info) {}
  NodeType type() const { return t; }
};

// config info
template <Node::NodeType t>
class ConfigNode : public NodeBase<t> {
 public:
  ConfigNode(const Info& info) : NodeBase<t>(info) {}

  virtual Node::NodeSubtype subtype() const { return Node::NodeSubtype::CONFIG; }
};

class Pathable {
 public:
  virtual ~Pathable() {}

  virtual const std::string& path() const = 0;
};

template <Node::NodeType t>
class PluginNode : public NodeBase<t>, public Pathable {
 private:
  const std::string _path;

 public:
  PluginNode(const Info& info, const std::string& path)
      : NodeBase<t>(info), _path(path) {}
  Node::NodeSubtype subtype() const override { return Node::NodeSubtype::PLUGIN; }
  const std::string& path() const { return _path; }
};

template <Node::NodeType t>
class PluginsRootNode : public NodeBase<t> {
 public:
  PluginsRootNode() : NodeBase<t>(Info(makeName(), makeDescription())) {}

 private:
  static const std::string makeName() {
    switch (t) {
    case Node::NodeType::DATASOURCE:
        return "Data Source Plugins";
      case Node::NodeType::SYMBOLSSOURCE:
        return "Symbols Source Plugins";
      case Node::NodeType::RUNNABLE:
        return "Runnable Plugins";
      case Node::NodeType::SLIPPAGE:
        return "Slippage Plugins";
      case Node::NodeType::COMMISSION:
        return "Commission Plugins";
      case Node::NodeType::SIGNALHANDLER:
        return "Signal Handler Plugins";
      default:
        assert(false);
        return "";
    }
  }
  static const std::string makeDescription() {
    switch (t) {
      case Node::NodeType::DATASOURCE:
        return "Data Source Plugins";
      case Node::NodeType::SYMBOLSSOURCE:
        return "Symbols Source Plugins";
      case Node::NodeType::RUNNABLE:
        return "Runnable Plugins";
      case Node::NodeType::SLIPPAGE:
        return "Slippage Plugins";
      case Node::NodeType::COMMISSION:
        return "Commission Plugins";
      case Node::NodeType::SIGNALHANDLER:
        return "Signal Handler Plugins";
      default:
        assert(false);
        return "";
    }
  }

 public:
  virtual Node::NodeSubtype subtype() const { return Node::NodeSubtype::ROOT; }
};

using NodePtr = std::shared_ptr<Node>;

inline bool operator==(const NodePtr& node, const UniqueId& id) {
  return node->id() == id;
}

using DataSourceConfigNode = ConfigNode<Node::NodeType::DATASOURCE>;
using SymbolsSourceConfigNode = ConfigNode<Node::NodeType::SYMBOLSSOURCE>;
using RunnableConfigNode = ConfigNode<Node::NodeType::RUNNABLE>;
using CommissionConfigNode = ConfigNode<Node::NodeType::COMMISSION>;
using SlippageConfigNode = ConfigNode<Node::NodeType::SLIPPAGE>;
using SignalHandlerConfigNode = ConfigNode<Node::NodeType::SIGNALHANDLER>;

using DataSourcePluginNode = PluginNode<Node::NodeType::DATASOURCE>;
using SymbolsSourcePluginNode = PluginNode<Node::NodeType::SYMBOLSSOURCE>;
using RunnablePluginNode = PluginNode<Node::NodeType::RUNNABLE>;
using CommissionPluginNode = PluginNode<Node::NodeType::COMMISSION>;
using SlippagePluginNode = PluginNode<Node::NodeType::SLIPPAGE>;
using SignalHandlerPluginNode = PluginNode<Node::NodeType::SIGNALHANDLER>;

using DataSourcePluginRootNode = PluginsRootNode<Node::NodeType::DATASOURCE>;
using SymbolsSourcePluginRootNode = PluginsRootNode<Node::NodeType::SYMBOLSSOURCE>;
using RunnablePluginRootNode = PluginsRootNode<Node::NodeType::RUNNABLE>;
using CommissionPluginRootNode = PluginsRootNode<Node::NodeType::COMMISSION>;
using SlippagePluginRootNode = PluginsRootNode<Node::NodeType::SLIPPAGE>;
using SignalHandlerPluginRootNode = PluginsRootNode<Node::NodeType::SIGNALHANDLER>;

class Traverser {
 public:
  virtual ~Traverser() {}
  virtual void node(const Node* node) = 0;
};

class WrongPluginTypeException {
 private:
  const std::string _message;

 public:
  WrongPluginTypeException(const std::string& message) : _message(message) {}

  const std::string& message() const { return _message; }
};

class IdNotFoundException {
 private:
  const UniqueId _id;
  std::string _message;

 public:
  IdNotFoundException(const UniqueId& id) : _id(id) {
    _message = tradery::format("Id ", id.str(), " not found");
  }

  const std::string& message() const { return _message; }

  const UniqueId& id() const { return _id; }
};

class PluginTree : public tree<NodePtr>, public PluginExplorer {
 private:
  const iterator _dataSourcePluginRoot;
  const iterator _symbolsSourcePluginRoot;
  const iterator _runnablePluginRoot;
  const iterator _slippagePluginRoot;
  const iterator _commissionPluginRoot;
  const iterator _signalHandlerPluginRoot;

 public:
  PluginTree()
      : _dataSourcePluginRoot(insert(begin(), std::make_shared< DataSourcePluginRootNode >())),
        _symbolsSourcePluginRoot(insert(begin(), std::make_shared< SymbolsSourcePluginRootNode >())),
        _runnablePluginRoot(insert(begin(), std::make_shared< RunnablePluginRootNode >())),
        _commissionPluginRoot(insert(begin(), std::make_shared< CommissionPluginRootNode >())),
        _slippagePluginRoot(insert(begin(), std::make_shared< SlippagePluginRootNode >())),
        _signalHandlerPluginRoot(insert(begin(), std::make_shared< SignalHandlerPluginRootNode >()))

  {}

  void clear() {
    erase_children(_dataSourcePluginRoot);
    erase_children(_symbolsSourcePluginRoot);
    erase_children(_runnablePluginRoot);
    erase_children(_commissionPluginRoot);
    erase_children(_slippagePluginRoot);
    erase_children(_signalHandlerPluginRoot);
  }

  template <class T, class U, class V>
  void f(const std::string& filePath, iterator root) {
    try {
      PluginInstance<T> pi(filePath);
      addPlugin<Plugin<T>, U, V>(*pi, filePath, root);
    }
    catch (const PluginInstanceException& e) {
      LOG(log_debug, "PluginInstanceException: ", filePath, ", message: ", e.message());
    }
  }

  virtual void process(const std::string& filePath, PluginLoadingStatusHandler* loadingStatusHandler, std::vector<std::shared_ptr<Info> >& duplicates) {
    try {
      if (loadingStatusHandler != 0) {
        loadingStatusHandler->event(filePath);
      }

      try {
        LOG( log_debug, "trying data source plugin: ", filePath );
        f<DataSource, DataSourcePluginNode, DataSourceConfigNode>(filePath, _dataSourcePluginRoot);
      }
      catch (const PluginTreeException& e) {
        duplicates.insert(duplicates.end(), e.info().begin(), e.info().end());
      }

      try {
        f<SymbolsSource, SymbolsSourcePluginNode, SymbolsSourceConfigNode>(filePath, _symbolsSourcePluginRoot);
      }
      catch (const PluginTreeException& e) {
        duplicates.insert(duplicates.end(), e.info().begin(), e.info().end());
      }

      try {
        f<Runnable, RunnablePluginNode, RunnableConfigNode>(filePath, _runnablePluginRoot);
      }
      catch (const PluginTreeException& e) {
        duplicates.insert(duplicates.end(), e.info().begin(), e.info().end());
      }

      try {
        f<Commission, CommissionPluginNode, CommissionConfigNode>(filePath, _commissionPluginRoot);
      }
      catch (const PluginTreeException& e) {
        duplicates.insert(duplicates.end(), e.info().begin(), e.info().end());
      }

      try {
        f<Slippage, SlippagePluginNode, SlippageConfigNode>(filePath, _slippagePluginRoot);
      }
      catch (const PluginTreeException& e) {
        duplicates.insert(duplicates.end(), e.info().begin(), e.info().end());
      }

      try {
        f<SignalHandler, SignalHandlerPluginNode, SignalHandlerConfigNode>(filePath, _signalHandlerPluginRoot);
      }
      catch (const PluginTreeException& e) {
        duplicates.insert(duplicates.end(), e.info().begin(), e.info().end());
      }
    }
    catch (const PluginMethodException&) {
      // the plugin doesn't implement at least one method
      //			LOG( log_debug, "PluginMethodException" );
    }
    catch (const PluginException&) {
      //			LOG( log_debug, "PluginException" );
    }
  }

  void init() {}

  virtual void command(Command cmd) {}

  // TODO: implement and check assumptions: level of various nodes, no duplicate
  // ids etc
  bool assertValid() { return true; }

  template <class T, Node::NodeType U>
  std::shared_ptr<PluginInstance<T> > getPlugin(const UniqueId& id) const {
    iterator i = std::find(begin(), end(), id);
    // todo: replace this assert with an exception - this is an error
    if (i != end()) {
      const Node* node = i->get();
      // if the node is of the wrong type, throw exception
      if (node->type() != U) {
        throw WrongPluginTypeException(tradery::format("Plugin ", id.str(), " is of type " , Node::typeToString(node->type()), ", expected " , Node::typeToString(U)));
      }
      assert(node->subtype() != Node::NodeSubtype::ROOT);
      // go down the tree until we find a parent that is of plugin type
      while (node->subtype() != Node::NodeSubtype::PLUGIN) {
        // call the base class parent
        i = tree<NodePtr>::parent(i);
        node = i->get();
      }

      const Pathable* pluginNode = dynamic_cast<const Pathable*>(node);

      return std::make_shared< PluginInstance<T> >(pluginNode->path());
    }
    else {
      // couldn't find the id
      throw IdNotFoundException(id);
    }
  }

  bool traverseChildren(const UniqueId& id, Traverser& traverser) const {
    for (iterator i = begin(); i != end(); i++) {
      if (*i == id) {
        for (sibling_iterator si = i; si != end(i); si++) {
          traverser.node(si->get());
        }

        return true;
      }
    }
    return false;
  }

 public:
  template <Node::NodeType T, Node::NodeSubtype U, class V>
  std::shared_ptr< V > findNode(const UniqueId& id) const {
    for (iterator i = begin(); i != end(); i++) {
      if (*i == id) {
        ASSERT((*i)->type() == T);
        ASSERT((*i)->subtype() == U);
        return dynamic_cast<std::shared_ptr< V >>(*i);
      }
    }
    return 0;
  }

  NodePtr findNode(const UniqueId& id) const {
    iterator i = std::find(begin(), end(), id);
    return i != end() ? *i : NodePtr{};
  }

  bool hasId(const UniqueId& id) const { return findNode(id) != 0; }

  template <Node::NodeType T>
  const PluginNode<T>* findPluginNode(const UniqueId& id) const {
    return findNode<T, Node::NodeSubtype::PLUGIN, PluginNode<T> >(id);
  }

  template <Node::NodeType T>
  const ConfigNode<T>* findConfigNode(const UniqueId& id) const {
    return findNode<T, Node::NodeSubtype::CONFIG, ConfigNode<T> >(id);
  }

  template <Node::NodeType T>
  const PluginsRootNode<T>* findPluginRootNode(const UniqueId& id) const {
    return findNode<T, Node::NodeSubtype::ROOT, PluginsRootNode<T> >(id);
  }

  template <class V> void addConfig(const UniqueId& parent, InfoPtr info){
    if (std::find(begin(), end(), info->id()) != end()) {
      // duplicate ids - throw exception
      throw PluginTreeException(info);
    }
    iterator i = std::find(begin(), end(), parent);
    // parent must be in there already - assert if not
    assert(i != end());
    iterator k = append_child(i, std::make_shared< V >( *info ) );
  }

  template <class V> void removeConfig(const UniqueId& id) {
    iterator i = std::find(begin(), end(), id);
    assert(i != end());
    __super::erase(i);
  }

  template <class V> void replaceConfig(const Info& info) {
    iterator i = std::find(begin(), end(), info.id());
    assert(i != end());
    __super::replace(i, NodePtr(new V(info)));
  }

  bool checkIds(const UniqueId& id) {
    static UniqueId unauthorizedIds[] = {UniqueId(), UniqueId("02284BC4-CE53-4dd5-8925-B6B874799360") };

    for (int n = 0; n < sizeof(unauthorizedIds) / sizeof(UniqueId); n++) {
      if (id == unauthorizedIds[n]) {
        return false;
      }
    }
    return true;
  }

  template <class T, class U, class V> void addPlugin(const T& plugin, const std::string& path, const iterator& root) {
    LOG_PLUGIN(log_debug, "trying to add plugin type: \"", boost::typeindex::type_id<T>().pretty_name(), "\", path", path, ", id: ", plugin.id().str());

    PluginTreeException e;
    // check that the plug-in is not on the black  list
    // if it is, just ignore it
    if (!checkIds(plugin.id())) return;
    // the node should not be there when adding
    NodePtr node = findNode(plugin.id());

    // if the node is found, add it to the exception, the id should not be
    // duplicated
    if (node) {
      LOG(log_debug, "duplicate plugin found: ", path, ", id: ", plugin.id().str());
      e.add(node);
    }
    else {
      iterator i = append_child(root, std::make_shared< U >(plugin, path));

      for (InfoPtr info = plugin.first(); info.get() != 0; info = plugin.next()) {
        try {
          LOG_PLUGIN(log_debug, "adding plugin type: \"", boost::typeindex::type_id<T>().pretty_name(), "\", path: \"", path, "\", info: ", info->toString());

          addConfig<V>((*i)->id(), info);
        }
        catch (const PluginTreeException & ex) {
          LOG(log_error, "PluginTreeException while adding plugin id: ", info->toString(), ", message: ", ex.message());
          e.add(ex.info()[0]);
        }
      }
    }
    if (e.info().size() > 0) {
      throw e;
    }

  	LOG( log_debug, _T( "plugin added: "), path );
}

  // returns the id of the plugin whose parameter id is a configuration
  // if no parent, return 0
  const UniqueId* parent(const UniqueId& id) const {
    for (iterator i = begin(); i != end(); i++) {
      if (*i == id) {
        iterator p = tree<NodePtr>::parent(i);
        if (p != end()) {
          return &(*p)->id();
        }
        else {
          return 0;
        }
      }
    }
    // the id wasn't found - throw and exception
    LOG(log_error, "id not found: ", id.str());
    throw IdNotFoundException(id);
  }
};
