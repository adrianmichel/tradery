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

#include <process.h>
#include <misc.h>
namespace tradery

#ifdef THREAD_EXPORTS
#define THREAD_API __declspec(dllexport)
#else
#define THREAD_API __declspec(dllimport)
#endif

{
/**\brief Encapsulation of a thread
 *
 */

class THREAD_API ThreadBase {
 private:
  HANDLE _hThread;
  DWORD _threadId;
  const std::string _diag_string;
  bool _priority;
  bool _run;
  bool _running;

 public:
  /**
   * thread context base class - to be derived if necessary
   *
   * A pointer to a thread context object or derived is passed to the run method
   */
  class ThreadContext {
   public:
    virtual ~ThreadContext() {}
  };

  /**
   * starts the thread
   */
  virtual void start(ThreadContext* context = 0);
  virtual void startSync(ThreadContext* context = 0, unsigned long timeout = 0);

  /**
   * stops the thread synchronously
   */
  virtual void stopSync();
  /**
   * stops the thread asynchronously
   */
  virtual void stopAsync();

  /**
   * restarts the thread (stops and starts it)
   */
  void restart(ThreadContext* context = 0);
  bool isRunning() const;
  bool stopping();
  const std::string& name() const { return _diag_string; }

  bool isThreadActive() {
    return NULL != _hThread && WAIT_OBJECT_0 != ::WaitForSingleObject(_hThread, 0);
  }

  // returns false if timeout
  // true if the objet has become signaled
  bool waitForThread(DWORD timeout = INFINITE) {
    return ::WaitForSingleObject(_hThread, timeout) != WAIT_TIMEOUT;
  }

  HANDLE handle() { return _hThread; }

 protected:
  ThreadBase(const std::string& diag_string, bool priority = false)
      : _hThread(0), _run(false), _running(false), _diag_string(diag_string), _priority(priority), _threadId(0) {}

 public:
  virtual ~ThreadBase() {
    if (_hThread != 0) CloseHandle(_hThread);
  }

 protected:
  class Context {
   private:
    ThreadBase* _thread;
    std::shared_ptr<ThreadBase::ThreadContext> _context;

   public:
    Context(ThreadBase* thread, ThreadBase::ThreadContext* context = 0)
        : _thread(thread), _context(context) {}

    virtual ~Context() {}

    ThreadBase* thread() { return _thread; }

    ThreadBase::ThreadContext* threadContext() { return _context.get(); }

    const std::string& diagString() const {
      assert(_thread != 0);
      return _thread->_diag_string;
    }
  };

  /**
   * the thread method
   *
   * receives a pointer to a generic ThreadContext - must cast it to the
   * specific derived context defined by the caller
   */
  virtual void run(ThreadContext* context) = 0;
  HANDLE create(ThreadContext* context, unsigned int (*proc)(void*));

 private:
  void startx(ThreadContext* context = 0, bool sync = false,
              unsigned long timeout = 0);
  void stopx();
  void stopy();
  void runx(ThreadContext* context);
  static unsigned int threadProc(void* p);
};

class THREAD_API Thread : public ThreadBase {
 public:
  Thread(const std::string& diag_string, bool priority = false)
      : ThreadBase(diag_string, priority) {}
};
}