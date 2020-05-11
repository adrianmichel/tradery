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
#include <hinstance.h>

HInstance::HInstance(const std::string& fileName)
    : _path(fileName), _hInstance(init()) {}

HInstance::~HInstance() {
  LOG(log_debug, "freeing library \"", _path, "\"");
  if (_hInstance != 0) {
    AfxFreeLibrary(_hInstance);
  }
}

HINSTANCE HInstance::init() {
  std::wstring wpath(s2ws(_path));
  HINSTANCE h = AfxLoadLibrary(wpath.c_str());
  if (0 != h) {
    LOG(log_debug, "loaded library \"", _path, "\"");
    return h;
  }
  else {
    LOG(log_error, "error loading library \"", _path, "\", last error: ", GetLastError());
    throw HInstanceException(_path);
  }
}

FARPROC HInstance::getProcAddress(const std::string& procName) const {
  assert(_hInstance != 0);
  FARPROC proc = GetProcAddress(_hInstance, procName.c_str());

  if (proc != 0) {
    LOG(log_debug, "got proc \"", procName, "\" address from \"", _path, "\"");
    return proc;
  }
  else {
    LOG(log_error, "error getting proc \"", procName, "\" address from \"", _path, "\", last error: ", GetLastError());
    throw HInstanceMethodException(path(), procName);
  }
}
