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
#include "miscwin.h"
#include <string.h>

class MISCWIN_API ResourceWrapper {
 private:
  HGLOBAL hRes = NULL;
  void* data = NULL;
  unsigned int size = 0;

 public:
  ResourceWrapper(const std::string& type, int id);
  virtual ~ResourceWrapper();
  void* getData() const { return this->data; }

  unsigned int getSize() { return size; }
};

#define TEXT_TYPE "TEXT"

class MISCWIN_API TextResource : public ResourceWrapper {
  boost::scoped_array<char> data;

 public:
  TextResource(int id)
      : ResourceWrapper(TEXT_TYPE, id), data(new char[__super::getSize() + 1]) {
    strncpy_s(data.get(), getSize() + 1, reinterpret_cast<const char*>(getData()), getSize());
  }

  operator const char*() const { return data.get(); }

  const char* get() const { return data.get(); }

  std::string to_string() const { return data.get(); }
};
