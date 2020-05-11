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

class HInstanceException {
 private:
  DWORD _lastError;
  const std::string _path;

 public:
  HInstanceException(const std::string& path)
      : _lastError(::GetLastError()), _path(path) {}

  DWORD getLastError() const { return _lastError; }
  const std::string& path() const { return _path; }
};

class HInstanceMethodException : public HInstanceException {
 private:
  const std::string _name;

 public:
  HInstanceMethodException(const std::string& path, const std::string& name)
      : _name(name), HInstanceException(path) {}

  const std::string& name() const { return _name; }
};

// a library instance class that deals correctly with loading/freeing the
// library
class HInstance {
 private:
  const std::string _path;
  HINSTANCE _hInstance;

 protected:
  HInstance(const std::string& fileName);
  ~HInstance();

  bool valid() const { return _hInstance != 0; }

  FARPROC getProcAddress(const std::string& procName) const;

 private:
  HINSTANCE init();

 public:
  const std::string& path() const { return _path; }
};