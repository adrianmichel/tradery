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

#include <queue>
#include <boost/thread.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <de_constraints.hpp>
#include <multithread.hpp>

#include "traderyrunobjectivefunction.h"
#include "Variables.h"

class Task;

typedef boost::shared_ptr<Task> TaskPtr;

typedef Settable<double> TaskBase;

class TaskListener {
 public:
  virtual ~TaskListener() {}

  virtual void betterResult(TaskPtr result) = 0;
  virtual void worseResult(TaskPtr result) = 0;
};

typedef boost::shared_ptr<TaskListener> TaskListenerPtr;

class Task : public TaskBase, public boost::enable_shared_from_this<Task> {
 private:
  de::DVectorPtr m_vars;
  TaskListenerPtr m_listener;
  bool m_minimize;
  double m_ref;

 public:
  Task(de::DVectorPtr vars, bool minimize, double ref, TaskListenerPtr listener)
      : m_vars(vars), m_minimize(minimize), m_ref(ref), m_listener(listener) {}

  /*
  Task()
  : m_vars( boost::make_shared< de::DVector >() )
  {
  }
*/
  void setResult(double result) {
    TaskBase::set(result);
    if (m_minimize && result < m_ref || !m_minimize && result > m_ref)
      m_listener->betterResult(shared_from_this());
    else
      m_listener->worseResult(shared_from_this());
  }

  de::DVectorPtr vars() { return m_vars; }
  double result() const { return TaskBase::getValue(); }
};

typedef boost::shared_ptr<Task> TaskPtr;

typedef std::queue<TaskPtr> TaskQueueBase;

typedef std::vector<TaskPtr> TasksBase;

class Tasks : public TasksBase {
 private:
  de::mutex m_mx;

 public:
  Tasks() {}

  void add(TaskPtr task) {
    de::lock lock(m_mx);

    TasksBase::push_back(task);
  }
};

typedef boost::shared_ptr<Tasks> TasksPtr;

class TaskQueue : public TaskQueueBase {
 private:
  mutable de::mutex m_mx;

 public:
  void push(TaskPtr task) {
    de::lock lock(m_mx);

    TaskQueueBase::push(task);
  }

  TaskPtr pop() {
    de::lock lock(m_mx);

    if (!TaskQueueBase::empty()) {
      TaskPtr task(TaskQueueBase::front());

      TaskQueueBase::pop();

      return task;
    } else
      return TaskPtr();
  }

  bool empty() const {
    de::lock lock(m_mx);

    return TaskQueueBase::empty();
  }

  void push_back(TaskPtr task) { push(task); }

  void push(TasksPtr tasks) {
    // no need for lock here, the thread sync is done inside push
    std::copy(tasks->begin(), tasks->end(), std::back_inserter(*this));
  }

  void clear() {
    de::lock lock(m_mx);

    while (!empty()) TaskQueueBase::pop();
  }
};

typedef boost::shared_ptr<TaskQueue> TaskQueuePtr;

class Processor {
 private:
  TaskQueuePtr m_queue;
  TasksPtr m_results;

  TraderyRunDEObjectiveFunctionPtr m_of;

  mutable de::mutex m_mx;
  bool m_stop;
  const size_t m_index;
  bool m_result;

 public:
  Processor(size_t index, TraderyRunDEObjectiveFunctionFactoryPtr of,
            TaskQueuePtr queue, TasksPtr results)
      : m_stop(false),
        m_index(index),
        m_of(of->make()),
        m_results(results),
        m_queue(queue),
        m_result(false) {}

  void stop() {
    de::lock lock(m_mx);
    m_stop = true;
  }

  bool isStop() const {
    de::lock lock(m_mx);
    return m_stop;
  }

  void operator()() {
    m_result = false;
    try {
      while (!isStop()) {
        if (!m_queue->empty()) {
          TaskPtr task(m_queue->pop());

          // a task must not have been processed already
          assert(!task->isSet());

          // run task;
          double result((*m_of)(task->vars()));
          task->setResult(result);

          m_results->add(task);
        } else
          Sleep(1);
      }
    } catch (const de::objective_function_exception& e) {
      std::cout << "Objective function failed: " << e.what() << std::endl;
      m_result = false;
    }
  }
};

typedef boost::shared_ptr<Processor> ProcessorPtr;
typedef std::vector<ProcessorPtr> ProcessorVector;

typedef boost::shared_ptr<boost::thread_group> thread_group_ptr;

class Processors {
 private:
  ProcessorVector m_processors;
  thread_group_ptr m_threads;
  TaskQueuePtr m_taskQueue;
  TasksPtr m_results;

 public:
  Processors(size_t count, TraderyRunDEObjectiveFunctionFactoryPtr off)
      : m_taskQueue(boost::make_shared<TaskQueue>()),
        m_results(boost::make_shared<Tasks>()) {
    assert(count > 0);

    for (size_t n = 0; n < count; ++n)
      m_processors.push_back(ProcessorPtr(
          boost::make_shared<Processor>(n, off, m_taskQueue, m_results)));
  }

  void start() {
    m_threads = boost::make_shared<boost::thread_group>();

    for (ProcessorVector::size_type n = 0; n < m_processors.size(); ++n)
      m_threads->create_thread(boost::ref(*m_processors[n]));
  }

  void stopSync() {
    for (ProcessorVector::size_type n = 0; n < m_processors.size(); ++n) {
      m_processors[n]->stop();
    }

    m_threads->join_all();
  }

  TasksPtr results() { return m_results; }
  TaskQueuePtr taskQueue() { return m_taskQueue; }

  void push_task(TaskPtr task) { m_taskQueue->push(task); }

  void push_tasks(TasksPtr tasks) { m_taskQueue->push(tasks); }

  void wait() { m_threads->join_all(); }

  void clearQueue() { m_taskQueue->clear(); }

  size_t count() const { return m_processors.size(); }
};

typedef boost::shared_ptr<Processors> ProcessorsPtr;
