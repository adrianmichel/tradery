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

class ArrayIndexNotFoundException : public CoreException {
 public:
  ArrayIndexNotFoundException(size_t index)
      : CoreException(ARRAY_INDEX_NOT_FOUND_EXCEPTION, "Index not found in array: "s + std::to_string( index) ) {}
};

template <class Value>
class ARRAY {
 private:
  std::vector<Value> _v;

 public:
  size_t count() const { return _v.size(); }
  size_t size() const { return _v.size(); }
  void add(const Value& v) { _v.push_back(v); }
  Value& operator[](size_t index) {
    if (index < _v.size()) {
      return _v[index];
    }
    else {
      throw ArrayIndexNotFoundException(index);
    }
  }

  bool empty() const { return _v.empty(); }
  void clear() { _v.clear(); }
};

class DictionaryKeyNotFoundException : public CoreException {
 public:
  DictionaryKeyNotFoundException()
      : CoreException(DICTIONARY_KEY_NOT_FOUND_EXCEPTION, "Key not found in dictionary collection") {}
};

#define Array(Value) ARRAY<Value>

template <class Key, class Value>
class DICTIONARY {
 private:
  using Map = std::map<Key, Value>;

  mutable Array(Key) _keys;
  Map _m;
  mutable bool _clean;

 public:
  DICTIONARY() : _clean(true) {}

  bool add(const Key& key, const Value& value) {
    std::pair<Map::iterator, bool> i = _m.insert(Map::value_type(key, value));
    if (i.second) {
      _clean = false;
    }
    return i.second;
  }

  Value& operator[](const Key& key) {
    if (has(key)) {
      return _m[key];
    }
    else {
      throw DictionaryKeyNotFoundException();
    }
  }

  bool has(const Key& key) { return _m.find(key) != _m.end(); }

  Array(Key) getKeys() const {
    if (!_clean) {
      _keys.clear();
      for (auto v : _m ){
        _keys.add(v.first);
      }
      _clean = true;
    }

    return _keys;
  }
};

#define Dictionary(Key, Value) DICTIONARY<Key, Value>
