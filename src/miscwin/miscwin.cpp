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
#include <thread.h>
#include <shlobj.h>
#include <miscfile.h>
#include <io.h>
#include <sys/types.h>  // For stat().
#include <sys/stat.h>   // For stat().

#include <boost/shared_array.hpp>
#include <wininet.h>
#include <Processthreadsapi.h>
#include <WinBase.h>

#include <log.h>

using namespace tradery;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
    break;
  case DLL_THREAD_ATTACH:
    break;
  case DLL_THREAD_DETACH:
    break;
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}


MISCWIN_API const RunProcessResult tradery::runProcess(
    const std::string& processFileName, const std::string& cmdLine,
    bool waitForProcess, const std::string* startingDirectory,
    const Environment& env, unsigned long timeout) {
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  LPCSTR startingDir = startingDirectory != 0 ? startingDirectory->c_str() : 0;

  // Run the autoupdate program.
  if (CreateProcess(
          s2ws(processFileName.empty() ? 0 : processFileName).c_str(),
          const_cast<LPTSTR>(s2ws(cmdLine).c_str()),  // Command line.
          NULL,                       // Process handle not inheritable.
          NULL,                       // Thread handle not inheritable.
          TRUE,                       // Set handle inheritance to FALSE.
          0,                          // No creation flags.
          0,                          // Use parent's environment block.
          s2ws(startingDir).c_str(),  // Use parent's starting directory.
          &si,                        // Pointer to STARTUPINFO structure.
          &pi)  // Pointer to PROCESS_INFORMATION structure.
  ) {
    DWORD exitCode(0);

    bool timedout(false);
    // Wait until child process exits.
    if (waitForProcess) {
      if (timeout == 0) {
        WaitForSingleObject(pi.hProcess, INFINITE);
      }
      else {
        if (WaitForSingleObject(pi.hProcess, timeout) == WAIT_TIMEOUT) {
          // if timeout, forcefully therminate the process
          TerminateProcess(pi.hProcess, -1);
          timedout = true;
        }
      }
    }
    GetExitCodeProcess(pi.hProcess, &exitCode);

    // Close process and thread handles.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return RunProcessResult(timedout, exitCode);
  }
  else {
    throw RunProcessException(processFileName, cmdLine, GetLastError());
  }
}

MISCWIN_API void setCurrentThreadIdealProcessor(unsigned int processor) {
  /*	DWORD pa;
          DWORD sa;
          GetProcessAffinityMask( GetCurrentProcess(), &pa, &sa );
          COUT << pa << _T( ", " ) << sa << std::endl;
          */
  HANDLE hThread = GetCurrentThread();
  ::SetThreadIdealProcessor(hThread, processor);
  //	::SetThreadAffinityMask( GetCurrentThread(), 1 >> processor );
}

MISCWIN_API unsigned long getCurrentCPUNumber() {
  return GetCurrentProcessorNumber();
  // this api is not supported on XP
  //::GetCurrentProcessorNumber();
}

////////////////////////////////
// log related static variables
////////////////////////////////
tradery::Log tradery::Log::m_log;

LOG_API unsigned __int64 LogEntryExit::m_crtId(0);
LOG_API std::mutex LogEntryExit::m_mx;

//**************************
// Thread code
//**************************
void ThreadBase::start(ThreadContext* context) {
  if (!_running) startx(context);
}

void ThreadBase::startSync(ThreadContext* context, unsigned long timeout) {
  if (!_running) startx(context, true, timeout);
}

void ThreadBase::stopSync() {
  if (_running) stopx();
}
void ThreadBase::stopAsync() {
  if (_running) stopy();
}
void ThreadBase::restart(ThreadContext* context) {
  stopSync();
  start(context);
}
bool ThreadBase::isRunning() const { return _running; }
bool ThreadBase::stopping() { return !_run; }

unsigned int ThreadBase::threadProc(void* p) {
  std::shared_ptr<Context> context(static_cast<Context*>(p));

  try {
    LOG(log_info, "Starting thread: \"", context->diagString(), "\"");
    context->thread()->run(context->threadContext());
    context->thread()->_run = false;
    context->thread()->_running = false;
    LOG(log_info, "Exiting thread: \"", context->diagString(), "\"");
  }
  catch (std::exception& e) {
    LOG(log_error, "Unhandled exception in thread: \"", context->diagString(), "\"");
    LOG(log_error, "error: \"", e.what(), "\"");
    context->thread()->_run = false;
    context->thread()->_running = false;
  }
  catch (...) {
    LOG(log_error, "Unhandled unknown exception in thread: \"", context->diagString(), "\"");

    context->thread()->_run = false;
    context->thread()->_running = false;
  }
  return 0;
}

HANDLE ThreadBase::create(ThreadContext* context, unsigned int (*proc)(void*)) {
  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = NULL;

  return (HANDLE)_beginthreadex( (void*)&sa, 0, (unsigned(__stdcall*)(void*))proc, new Context(this, context), 0, (unsigned int*)&_threadId);
}

void ThreadBase::startx(ThreadContext* context, bool sync,
                        unsigned long timeout) {
  if (!_running) {
    _run = true;
    _running = true;

    _hThread = create(context, threadProc);

    if (_hThread) {
      if (sync) {
        WaitForSingleObject(_hThread, (timeout > 0 ? timeout : INFINITE));
      }

      if (_priority) {
        BOOL b = ::SetThreadPriority(_hThread, THREAD_PRIORITY_ABOVE_NORMAL);
      }
    } else {
      //couldn't create thread, handle error
    }
  }
}

void ThreadBase::stopx() {
  if (_hThread != 0) {
    _run = false;
    WaitForSingleObject(_hThread, INFINITE);
    _hThread = 0;
  }
}

void ThreadBase::stopy() {
  if (_hThread != 0) {
    _run = false;
    _hThread = 0;
  }
}

MISCWIN_API tradery::VersionPtr tradery::getFileVersion( const std::string& fileName) {
  DWORD handle;
  DWORD size(::GetFileVersionInfoSize(s2ws(fileName).c_str(), &handle));

  if (size > 0) {
    boost::shared_array<char> data(new char[size]);

    if (::GetFileVersionInfo(s2ws(fileName).c_str(), 0, size, data.get())) {
      VS_FIXEDFILEINFO* pVS = NULL;
      unsigned int len;

      if (VerQueryValue(data.get(), _T("\\"), (void**)&pVS, &len) && NULL != pVS ) {
        unsigned int n1, n2, n3, n4;
        n1 = pVS->dwProductVersionMS >> 16;
        n2 = pVS->dwProductVersionMS & 0xFFFF;
        n3 = pVS->dwProductVersionLS >> 16;
        n4 = pVS->dwProductVersionLS & 0xFFFF;

        return VersionPtr(std::make_shared< tradery::Version >(n1, n2, n3, n4));
      }
      else {
        return VersionPtr();
      }
    }
    else {
      return VersionPtr();
    }
  }
  else {
    return VersionPtr();
  }
}

MISCWIN_API std::string tradery::getFileLanguage(const std::string& fileName) {
  return "en";
}

extern "C" {
_CRTIMP int __cdecl _open_osfhandle(intptr_t, int);
}

// reserved windows file names
boost::regex expression( "(?:\\`|\\\\|/)(?:CON|PRN|AUX|NUL|(?:COM[1-9])|(?:LPT[1-9]))(?:\\'|\\\\|\\.|/)", boost::regex::perl | boost::regex::icase);

boost::match_flag_type flags = boost::match_default;

bool hasReservedFileName(const std::string& fileName) {
  // a reserved sring at the begining of string, or preceded by
  boost::match_results<const char*> what;
  return boost::regex_search(fileName.c_str(), what, expression, flags);
}

#define FILE_PREFIX std::string("\\\\?\\")

SpecialFileRead::SpecialFileRead(const std::string& fileName) : Base(fileName) {
  if (hasReservedFileName(fileName)) {
    createReadFile(FILE_PREFIX + fileName);
  }
  else {
    __super::setFStream(std::make_shared< std::ifstream >(fileName.c_str()));
  }
}

SpecialFileWrite::SpecialFileWrite(const std::string& fileName, bool append, bool binary)
    : Base(fileName) {
  if (hasReservedFileName(fileName)) {
    createWriteFile(FILE_PREFIX + fileName, append, binary);
  }
  else {
    file_type_ptr f(std::make_shared< std::ofstream >());

    int mode = (append ? std::ios_base::app : 0) | (binary ? std::ios_base::binary : 0);

    if (mode != 0) {
      f->open(fileName.c_str(), mode);
    }
    else {
      f->open(fileName.c_str());
    }

    f->precision(12);
    __super::setFStream(f);
  }
}

MISCWIN_API void tradery::SpecialFileRead::createReadFile( const std::string& fileName) {
  HANDLE file_handle = CreateFile(s2ws(fileName).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  __super::setHandle(file_handle);
  int file_descriptor = _open_osfhandle((intptr_t)file_handle, 0);

  setFileDescriptor(file_descriptor);
  FILE* file = _fdopen(file_descriptor, "r");

  __super::setFILE(file);

  __super::setFStream(std::make_shared< std::ifstream >(file));
}

MISCWIN_API void tradery::SpecialFileWrite::createWriteFile( const std::string& fileName, bool append, bool binary) {
  HANDLE file_handle = CreateFile( s2ws(fileName).c_str(), append ? FILE_APPEND_DATA : GENERIC_WRITE, 0, NULL, append ? OPEN_ALWAYS : CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  __super::setHandle(file_handle);

  int file_descriptor = _open_osfhandle((intptr_t)file_handle, 0);

  setFileDescriptor(file_descriptor);
  FILE* file = _fdopen(file_descriptor, "w");

  __super::setFILE(file);

  file_type_ptr fs(std::make_shared< std::ofstream >(file));

  __super::setFStream(fs);
}

