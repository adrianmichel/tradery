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

#include <winbase.h>
#include "strings.h"
#include "sharedptr.h"
#include "misc.h"
#include "log.h"

namespace tradery {

class FileException {};

inline std::string getModuleFileName() {
  TCHAR fileName[MAX_PATH + 1] = {0};

  GetModuleFileName(0, fileName, MAX_PATH);

  return ws2s(fileName);
}

using VersionPtr = std::shared_ptr<Version>;
MISCWIN_API VersionPtr getFileVersion(const std::string& fileName);
MISCWIN_API std::string getFileLanguage(const std::string& fileName);

class SpecialFileException {};

template <typename file_type>
class SpecialFile {
 protected:
  using file_type_ptr = std::shared_ptr<file_type>;

 private:
  file_type_ptr _fs;
  FILE* _f;
  HANDLE _h;
  int _fd;
  const std::string _fileName;

 protected:
  SpecialFile(const std::string& fileName)
      : _f(0), _h(0), _fd(0), _fileName(fileName) {}

  void setHandle(HANDLE h) {
    if (h != INVALID_HANDLE_VALUE) {
      _h = h;
    }
    else {
      LOG(Level::log_error, "last error: ", GetLastError());
      throw SpecialFileException();
    }
  }
  void setFILE(FILE* f) {
    if (f != 0) {
      _f = f;
    }
    else {
      LOG(Level::log_error, "last error: ", GetLastError());
      throw SpecialFileException();
    }
  }

  void setFStream(file_type_ptr fs) {
    if (fs && *fs) {
      _fs = fs;
    }
    else {
      LOG(Level::log_error, "last error: ", GetLastError());
      throw SpecialFileException();
    }
  }

  void setFileDescriptor(int fd) {
    if (fd >= 0) {
      _fd = fd;
    }
    else {
      LOG(Level::log_error, "last error: ", GetLastError());
      throw SpecialFileException();
    }
  }

  virtual ~SpecialFile() {
    if (_fs && _fs->is_open()) {
      _fs->close();
    }

    if (_f) {
      fclose(_f);
    }
  }

 public:
  operator file_type&() {
    assert(this->operator bool());
    return *_fs;
  }

  operator bool() const { return _fs && *_fs; }
};

class MISCWIN_API SpecialFileRead : public SpecialFile<std::ifstream> {
 private:
  using Base = SpecialFile<std::ifstream>;

 private:
  void createReadFile(const std::string& fileName);

 public:
  SpecialFileRead(const std::string& fileName);
};

class MISCWIN_API SpecialFileWrite : public SpecialFile<std::ofstream> {
 private:
  using Base = SpecialFile<std::ofstream>;

 private:
  void createWriteFile(const std::string& fileName, bool append, bool binary);

 public:
  SpecialFileWrite(const std::string& fileName, bool append = false, bool binary = false);
};

}  // namespace tradery
