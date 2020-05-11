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

#include "StdAfx.h"
#include "misc.h"

using namespace ::boost::uuids;

uuid generate() {
  boost::uuids::random_generator gen;
  return gen();
}

namespace tradery {
UniqueId::UniqueId() { id = to_string(::generate()); }

UniqueId::UniqueId(const std::string& i) : id(to_upper_case(i)) {}

UniqueId::UniqueId(const char* i) {
  uuid u;
  std::stringstream ss;

  ss << i;
  ss >> u;
  id = to_upper_case(to_string(u));
}

UniqueId::UniqueId(const TCHAR* id) {
  (*this) = ws2s(id).c_str();
}

UniqueId::UniqueId(const UniqueId& id) : id(to_upper_case(id.id)) {}

UniqueId::operator const std::string() const { return id; }

UniqueId::operator const std::wstring() const { return s2ws(id); }

const std::string& UniqueId::str() const { return id; }

bool UniqueId::operator<(const UniqueId& other) const { return id < other.id; }

bool UniqueId::operator==(const UniqueId& other) const {
  return id == other.id;
}

const UniqueId& UniqueId::operator=(const UniqueId& other) {
  if (this != &other) {
    id = other.id;
  }
  return *this;
}

UniqueId UniqueId::generate() { return UniqueId(); }
}  // namespace tradery