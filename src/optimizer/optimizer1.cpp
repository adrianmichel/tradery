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

#include <map>
#include <boost/scope_exit.hpp>
#include <traderyconnection.h>
#include <datetime.h>
#include "optimizertraderysession.h"
#include "task.h"
#include "cmdline.h"
#include "variables.h"
#include "traderyrunobjectivefunction.h"
#include "optimizer1.h"

using namespace de;
using namespace boost;
using namespace tradery;

class TaskRegionException : public std::exception {
 public:
  TaskRegionException(const std::string& message)
      : std::exception(message.c_str()) {}
};

// this is a region in the variable space that is tested for a good overall
// result (better than reference)
class TasksRegion : public Tasks,
                    public TaskListener,
                    public boost::enable_shared_from_this<TasksRegion> {
 public:
  typedef boost::shared_ptr<TasksRegion> TasksRegionPtr;

 private:
  typedef std::map<double, TaskPtr> TasksMap;

 private:
  de::DVectorPtr m_origin;
  double m_sidePct;
  de::constraints_ptr m_constraints;
  mutable de::mutex m_mx;

  std::map<double, TaskPtr> m_betterTasks;

  size_t m_countToProcess;

 public:
  TasksRegion(de::constraints_ptr constraints, de::DVectorPtr origin,
              double sidePct)
      : m_origin(origin),
        m_sidePct(sidePct),
        m_constraints(constraints),
        m_countToProcess(0) {}

  // uses the center as origin
  TasksRegion(de::constraints_ptr constraints, double sidePct)
      : m_origin(constraints->get_middle_point()),
        m_sidePct(sidePct),
        m_constraints(constraints),
        m_countToProcess(0) {}

  void generate_tasks(size_t count, double refResult, bool minimize) {
    de::lock lock(m_mx);

    for (size_t n = 0; n < count; ++n) {
      de::DVectorPtr vars(
          m_constraints->get_square_zone_rand_values(m_origin, m_sidePct));
      TaskPtr task(boost::make_shared<Task>(vars, minimize, refResult,
                                            shared_from_this()));
      __super::add(task);

      ++m_countToProcess;
    }
  }

  virtual void betterResult(TaskPtr task) {
    de::lock lock(m_mx);

    assert(task);
    assert(task->isSet());
    assert(m_countToProcess > 0);

    std::cout << "good result: " << task->result() << std::endl;
    --m_countToProcess;
    m_betterTasks.insert(TasksMap::value_type(task->getValue(), task));
  }

  virtual void worseResult(TaskPtr task) {
    de::lock lock(m_mx);
    assert(task);
    assert(task->isSet());
    assert(m_countToProcess > 0);

    std::cout << "bad result: " << task->result() << std::endl;
    --m_countToProcess;
  }

  bool hasBetterResult() const {
    de::lock lock(m_mx);

    return !m_betterTasks.empty();
  }

  TaskPtr getBestResult() const {
    de::lock lock(m_mx);

    return hasBetterResult() ? m_betterTasks.rbegin()->second : TaskPtr();
  }

  bool hasUnprocessedTasks() const {
    de::lock lock(m_mx);
    return m_countToProcess > 0;
  }

  void waitUntilProcessed() {
    while (hasUnprocessedTasks()) {
      Sleep(1);
    }
  }

  void waitUntilBetterResultOrProcessed() {
    while (hasUnprocessedTasks() && !hasBetterResult()) {
      Sleep(1);
    }
  }

  // margin  - number between 0-1, by how much has the value be better than ref
  // to quality. if margin 0.1, then it has to be 1.1 of ref for max, or 0.9 of
  // ref for min
  bool acceptable(double ref, bool minimize, double good_bad_ratio) {
    de::lock lock(m_mx);

    if (__super::size() > 0) {
      double sum = 0;

      size_t count(0);

      bool set(false);

      double good(0);
      double bad(0);

      for (size_t n = 0; n < __super::size(); ++n) {
        TaskPtr task((*this)[n]);

        if (task->isSet()) {
          set = true;
          ++count;
          double value = task->getValue();
          if (minimize && value < ref || !minimize && value > ref)
            good++;
          else
            bad++;

          sum += value;
        }
      }

      if (!set)
        throw TaskRegionException(
            "Region is not empty, but has no set values, can't calculate "
            "average");

      double average = sum / (double)count;

      double gbratio = good / bad;

      std::cout << "average: " << average << ", good/bad ratio: " << gbratio
                << std::endl;

      return (minimize && average < ref || !minimize && average > ref) &&
             gbratio > good_bad_ratio;

    } else
      throw TaskRegionException("Region empty, can't calculate average");
  }
};

typedef boost::shared_ptr<TasksRegion> TasksRegionPtr;

#define MAX_TASKS 100
#define MARGIN 0.2
#define GOOD_BAD_RATIO 4

class adrian_optimizer {
 private:
  ProcessorsPtr m_processors;
  const de::constraints_ptr m_constraints;
  // result of running the system with default parameters. The optimization
  // should yield at least the max of m_minResult and m_refResult. m_minResult
  // is used for example if the result is the tradery score and it's negative,
  // in that case it's not enough that optimization gets better result then
  // refResult, it should be at least better than 0 (which is what m_minResult
  // will be set at).
  const double m_refResult;
  const bool m_minimize;

 private:
 public:
  adrian_optimizer(ProcessorsPtr processors, de::constraints_ptr constraints,
                   double refResult, bool minimize)
      : m_constraints(constraints),
        m_refResult(refResult),
        m_processors(processors),
        m_minimize(minimize) {}

  TaskPtr run() {
    try {
      double ref = m_minimize ? m_refResult * (1.0 - MARGIN)
                              : m_refResult * (1.0 + MARGIN);

      m_processors->start();
      std::cout << "--- Starting optimization" << std::endl;

      TaskPtr result;

      for (size_t totalTasks = 0; totalTasks < MAX_TASKS; ++totalTasks) {
        std::cout << "--- Finding a candidate region " << std::endl;
        // put count sets of random variables in the queue

        // create a region with side 90% of the total side and centered in the
        // middle of the var space
        TasksRegionPtr tr(boost::make_shared<TasksRegion>(m_constraints, 90.0));

        tr->generate_tasks(m_processors->count(), ref, m_minimize);

        totalTasks += m_processors->count();
        m_processors->push_tasks(tr);

        tr->waitUntilBetterResultOrProcessed();

        m_processors->clearQueue();

        // if we found one
        if (tr->hasBetterResult()) {
          // first clear queue, no need to continue looking for a
          // better result while analyzing the current better
          // result.
          std::cout << tradery::format(
                           "--- Found a candidate region (%1%), analyzing",
                           tr->getBestResult()->result())
                    << std::endl;

          TasksRegionPtr tr1(boost::make_shared<TasksRegion>(
              m_constraints, tr->getBestResult()->vars(), 10.0));

          tr1->generate_tasks(10, ref, m_minimize);

          m_processors->push_tasks(tr1);

          // adding the best after pushing them into the queue, so it won't get
          // processed twice
          tr1->add(tr->getBestResult());

          tr1->waitUntilProcessed();

          std::cout << "--- Done analyzing the candidate region" << std::endl;

          // calculate if its a good region
          // if not, start the process again, either we use the next best
          // result, or generate a fresh set of results by going to the first
          // for

          if (tr1->acceptable(ref, m_minimize, GOOD_BAD_RATIO)) {
            // found a region that qualifies, getting the result and exiting the
            // loop
            std::cout << "--- Region qualifies" << std::endl;
            result = tr1->getBestResult();
            break;
          } else {
            // region is not good enough, trying again.
            std::cout << "--- Region doesn't qualify" << std::endl;
          }
        } else {
          // optimization failed for this region - couldn't find any result
          // bettern than default try again
          std::cout << "--- Couldn't find a candidate region, retrying"
                    << std::endl;
        }
      }

      if (result)
        std::cout << "--- Optimization succeeded" << std::endl;
      else
        std::cout << "--- Optimization failed, couldn't find a good region "
                     "within the allowed number of tries"
                  << std::endl;

      m_processors->stopSync();

      return result;
    } catch (...) {
      m_processors->stopSync();
      throw;
    }
  }
};

TaskPtr new_optimization(TraderyCredentialsPtr credentials,
                         tradery::Date startDate, tradery::Date ed,
                         PositionSizingPtr positionSizing,
                         VariablesNamesPtr variablesNames,
                         SystemIdsPtr systemIds, tradery::StringPtr symbolsList,
                         OptionsPtr options, const Configuration& cmdLine) {
  OptimizerTraderySessionEventListener m_el1;

  de::constraints_ptr constraints(
      boost::make_shared<VariablesConstraints>(cmdLine.variables()));

  Date leadInDate = startDate - DateDuration(cmdLine.leadIn());
  Date endDate =
      min(startDate + DateDuration(cmdLine.optimizationPeriod() - 1), ed);

  std::cout << "Optimization range: " << leadInDate.toString(us) << ", "
            << startDate.toString(us) << ", " << endDate.toString(us)
            << std::endl;

  ParametersPtr parameters(new Parameters(
      leadInDate, endDate, cmdLine.commission(), cmdLine.slippage()));

  // do a regular run using default values for all variables, to use as
  // reference during optimization

  VariablesNamesPtr emptyVarsNames(make_shared<VariablesNames>());
  TraderyRunDEObjectiveFunction tr(
      "reference run", credentials, parameters, emptyVarsNames, m_el1,
      symbolsList, systemIds, options, positionSizing, cmdLine.statToOptimize(),
      cmdLine.statGroupToOptimize(), startDate);
  double refRunResult(tr(make_shared<DVector>()));

  de::processor_listener_ptr deProcessorListener(
      boost::make_shared<de::null_processor_listener>());

  refRunResult = cmdLine.minimize()
                     ? std::min(refRunResult, cmdLine.worstAcceptableResult())
                     : std::max(refRunResult, cmdLine.worstAcceptableResult());

  TraderyRunDEObjectiveFunctionFactoryPtr tff(
      new TraderyRunDEObjectiveFunctionFactory(
          "tradery run objective function", credentials, parameters,
          variablesNames, m_el1, symbolsList, cmdLine.systems(), options,
          positionSizing, cmdLine.statToOptimize(),
          cmdLine.statGroupToOptimize(), startDate));

  ProcessorsPtr processors(boost::make_shared<Processors>(
      cmdLine.processorsCount(), boost::ref(tff)));

  adrian_optimizer ao(processors, constraints, refRunResult,
                      cmdLine.minimize());

  return ao.run();
}
