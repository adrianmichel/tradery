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
#include "optimizertraderysession.h"
#include "traderyrunobjectivefunction.h"
#include "cmdline.h"
#include "optimizer1.h"

#include <processors.hpp>

using namespace tradery;
using namespace de;
using namespace boost;

class OptimizerException {};

typedef de::objective_function_factory<TraderyRunDEObjectiveFunction>
    TraderyRunFactory;
typedef boost::shared_ptr<TraderyRunFactory> ObjectiveRunFactoryPtr;
typedef de::processors<ObjectiveRunFactoryPtr> ProcessorsX;

class OptimizerOptions : public Options {
 public:
  OptimizerOptions()
      : Options(false, true, false, false, false, false, warning) {}
};

individual_ptr de_optimization(TraderyCredentialsPtr credentials,
                               Date startDate, Date ed,
                               PositionSizingPtr positionSizing,
                               VariablesNamesPtr variablesNames,
                               SystemIdsPtr systemIds, StringPtr symbolsList,
                               OptionsPtr options,
                               const Configuration& cmdLine) {
  OptimizerTraderySessionEventListener m_el1;

  constraints_ptr constraints(
      make_shared<VariablesConstraints>(cmdLine.variables()));

  Date leadInDate = startDate - DateDuration(cmdLine.leadIn());
  Date endDate =
      min(startDate + DateDuration(cmdLine.optimizationPeriod() - 1), ed);

  std::cout << "Optimization range: " << leadInDate.toString(us) << ", "
            << startDate.toString(us) << ", " << endDate.toString(us)
            << std::endl;

  ParametersPtr parameters(new Parameters(
      leadInDate, endDate, cmdLine.commission(), cmdLine.slippage()));

  // can't use make_shared - too many arguments
  ObjectiveRunFactoryPtr tff(new TraderyRunDEObjectiveFunctionFactory(
      "Tradery run objective function", credentials, parameters, variablesNames,
      m_el1, symbolsList, cmdLine.systems(), options, positionSizing,
      cmdLine.statToOptimize(), cmdLine.statGroupToOptimize(), startDate));
  de::processor_listener_ptr deProcessorListener(
      boost::make_shared<de::null_processor_listener>());
  ProcessorsX::processors_ptr processors(make_shared<ProcessorsX>(
      cmdLine.processorsCount(), tff, deProcessorListener));
  mutation_strategy_arguments msa(cmdLine.weight(), cmdLine.crossover());

  termination_strategy_ptr terminationStrategy(
      boost::make_shared<max_gen_termination_strategy>(
          cmdLine.maxGenerations()));

  selection_strategy_ptr selectionStrategy(
      boost::make_shared<best_parent_child_selection_strategy>());
  mutation_strategy_ptr mutationStrategy(
      boost::make_shared<mutation_strategy_1>(VARS_COUNT, msa));
  de::listener_ptr deListener(boost::make_shared<de::null_listener>());

  de::differential_evolution<ObjectiveRunFactoryPtr> differentialEvolution(
      VARS_COUNT, cmdLine.populationSize(), processors, constraints,
      cmdLine.minimize(), terminationStrategy, selectionStrategy,
      mutationStrategy, deListener);

  differentialEvolution.run();
  // this is the best value and set of variables
  return differentialEvolution.best();
}

class WalkForwardException {};

// this is the date for the previous optimization periiod
bool walkForward(TraderyCredentialsPtr credentials, Date optStartDate, Date ed,
                 PositionSizingPtr positionSizing,
                 VariablesNamesPtr variablesNames, SystemIdsPtr systemIds,
                 StringPtr symbolsList, OptionsPtr options,
                 const Configuration& cmdLine, de::DVectorPtr vars) {
  tradery::StringPtr bestVars = ExternalVars(variablesNames, vars).toString();

  std::cout << std::endl << "running walk forward with:" << bestVars;

  OptimizerTraderySessionEventListener m_el1;

  Date startDate = optStartDate + DateDuration(cmdLine.optimizationPeriod());
  std::wstring x = startDate.toString(us);

  x = ed.toString(us);

  // test if we're exceeding the end date limit
  Date leadInDate = startDate - DateDuration(cmdLine.leadIn());
  x = leadInDate.toString(us);
  Date endDate =
      min(startDate + DateDuration(cmdLine.walkForwardPeriod() - 1), ed);
  x = endDate.toString(us);
  std::cout << "Walk forward range: " << leadInDate.toString(us) << ", "
            << startDate.toString(us) << ", " << endDate.toString(us)
            << std::endl;

  if (startDate > ed) {
    throw WalkForwardException();
  }

  ParametersPtr wfParameters(new Parameters(
      leadInDate, endDate, cmdLine.commission(), cmdLine.slippage()));

  TraderyRunDEObjectiveFunction trwf(
      "Walk forward run", credentials, wfParameters, variablesNames, m_el1,
      symbolsList, systemIds, options, positionSizing, cmdLine.statToOptimize(),
      cmdLine.statGroupToOptimize(), startDate);

  double wfresult = trwf(vars);

  std::cout << std::endl << "walk forward result: " << wfresult;

  TraderyRunDEObjectiveFunction trnr(
      "Normal run", credentials, wfParameters,
      VariablesNamesPtr(make_shared<VariablesNames>()), m_el1, symbolsList,
      systemIds, options, positionSizing, cmdLine.statToOptimize(),
      cmdLine.statGroupToOptimize(), startDate);

  double nrresult = trnr(make_shared<DVector>());

  std::cout << std::endl << "normal run result: " << nrresult;

  return true;
}

int _tmain(int argc, _TCHAR* argv[]) {
  try {
    //	Log::setLogToConsole();

    Configuration cmdLine;
    cmdLine.process();

    std::cout << "*** Parameters ***" << std::endl
              << *cmdLine.toString() << std::endl;

    setTraderyServer(cmdLine.server());

    PositionSizingPtr posSizing(new PositionSizing(
        cmdLine.initialCapital(), cmdLine.maxOpenPos(),
        (PositionSizingParams::PosSizeType)cmdLine.posSizeType(),
        cmdLine.posSizeValue(),
        (PositionSizingParams::PosSizeLimitType)cmdLine.posSizeLimitType(),
        cmdLine.posSizeLimitValue()));
    VariablesNamesPtr variablesNames(
        make_shared<VariablesNames>(cmdLine.variables()));
    SystemIdsPtr systemIds(cmdLine.systems());
    StringPtr symbolsList(cmdLine.symbolsList());
    OptionsPtr options(new OptimizerOptions());
    TraderyCredentialsPtr credentials(make_shared<TraderyCredentials>(
        cmdLine.userName(), cmdLine.password()));

    for (Date startDate(cmdLine.startDate()); startDate <= cmdLine.endDate();
         startDate += DateDuration(cmdLine.step())) {
      switch (cmdLine.runType()) {
        case RunType::de_optimization: {
          individual_ptr best(de_optimization(
              credentials, startDate, cmdLine.endDate(), posSizing,
              variablesNames, systemIds, symbolsList, options, cmdLine));

          tradery::StringPtr bestVars =
              ExternalVars(variablesNames, best->vars()).toString();

          std::cout << std::endl
                    << "best so far: " << best->cost()
                    << ", vars: " << bestVars;

        } break;
        case RunType::de_walk_forward: {
          individual_ptr best(de_optimization(
              credentials, startDate, cmdLine.endDate(), posSizing,
              variablesNames, systemIds, symbolsList, options, cmdLine));

          tradery::StringPtr bestVars =
              ExternalVars(variablesNames, best->vars()).toString();

          std::cout << std::endl
                    << "best so far: " << best->cost()
                    << ", vars: " << bestVars;

          walkForward(credentials, startDate, cmdLine.endDate(), posSizing,
                      variablesNames, systemIds, symbolsList, options, cmdLine,
                      best->vars());
        } break;
        case RunType::adrian_optimization: {
          TaskPtr best(new_optimization(
              credentials, startDate, cmdLine.endDate(), posSizing,
              variablesNames, systemIds, symbolsList, options, cmdLine));
          if (best) {
            tradery::StringPtr bestVars =
                ExternalVars(variablesNames, best->vars()).toString();

            std::cout << std::endl
                      << "best so far: " << best->result()
                      << ", vars: " << bestVars;

            walkForward(credentials, startDate, cmdLine.endDate(), posSizing,
                        variablesNames, systemIds, symbolsList, options,
                        cmdLine, best->vars());
          } else {
            std::cout << "no best was found, so no walk forward, moving to the "
                         "next range"
                      << std::endl;
          }
        } break;
        case RunType::adrian_walk_forward: {
          // adrianWalkForward( credentials, startDate, cmdLine.end)
        } break;
        default:
          throw OptimizerException();
      }
    }

    return 0;
  } catch (const WalkForwardException&) {
    std::cout
        << "Walk forward range exceeding the end date, optimization complete"
        << std::endl;
    return 0;
  } catch (const differential_evolution_exception&) {
    std::cout << "Optimization session failed" << std::endl;
    return 1;
  } catch (const ConfigurationException& e) {
    std::cout << "Command line error: " << e.what() << std::endl;
    return 1;
  } catch (const OptimizerException&) {
    std::cout << "Optimizer error: " << std::endl;
    return 1;
  }
}
