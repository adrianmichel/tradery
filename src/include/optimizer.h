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

/** @file
 *  \brief optimization related declarations
 *
 *  The current optimizer implementation can be run the same way as a regular
 * system (it's derived from Runnable), but can be used to optimize a system, in
 * back-testing mode (so static optimization) or in real time (dynamic
 * optimization)
 */

#include <core.h>

class CORE_API VariableException {};

// simple form of variable
class CORE_API Variable {
 private:
  double _start;
  double _end;
  double _current;
  double _step;

 protected:
  Variable(double start, double end, double step) throw(VariableException)
      : _start(start), _end(end), _current(start), _step(step) {
    if (_start > _end) throw VariableException();
  }

  double& getCurrent() { return _current; }

 public:
  virtual ~Variable() {}

  double getCurrent() const { return _current; }

  double getStart() const { return _start; }

  double getEnd() const { return _end; }

  double getStep() const { return _step; }
  // returns the current value
  operator double() const { return _current; }

  virtual void reset() { _current = _start; }

  virtual bool operator++() = 0;
  virtual bool operator++(int) = 0;
  virtual bool hasNext() const = 0;
};

// implements the most basic type of variable with linear increment
// other possible types could be: proportional step ( a percentage of the
// current value), or other non-linear scheme
class CORE_API ConstStepVariable : public Variable {
 public:
  ConstStepVariable(double start, double end,
                    double step) throw(VariableException)
      : Variable(start, end, step) {}

  // preincrement operator
  // increments and returns false if new value past the end, or false if not
  // past the end
  bool operator++() {
    getCurrent() += getStep();
    return getCurrent() <= getEnd();
  }

  // postincrement operator
  bool operator++(int) {
    bool b = getCurrent() <= getEnd();
    if (b) getCurrent() += getStep();
    return b;
  }

  bool hasNext() const { return getCurrent() <= getEnd() - getStep(); }
};

//****************************************************************

// params passed to a runnable
// params used for optimization - next calculates the next set of params, until
// all have been exhausted, when it returns 0 can be implemented using
// exhaustive optimization, MC
class CORE_API OptimizerParams {
 public:
  virtual ~OptimizerParams() {}
  // preincrement operator
  virtual bool operator++() = 0;
  // postincrmemt operator
  virtual bool operator++(int) = 0;
  // returns true if there are more params
  virtual bool hasNext() const = 0;
  // returns current set of params
  virtual std::auto_ptr<Parameters> getCurrent() = 0;
  // returns "best" set of params
  virtual std::auto_ptr<Parameters> best() = 0;
  virtual void reset() = 0;
};

typedef PtrVector<Variable> VV;
typedef std::auto_ptr<Parameters> PParameters;

class CORE_API ExhaustiveOptimizerParams : VV, public OptimizerParams {
 private:
  // the result of the last increment
  bool _overflow;

 public:
  ExhaustiveOptimizerParams() : _overflow(false) {}

  void addVariable(Variable* variable) {
    // maybe throw an exception here, although this is programmer error
    assert(variable != 0);
    push_back(variable);
  }

 private:
  // returns true on overflow
  // if overflow, all the variables will be set to start (the same as reset)
  bool increment() {
    // for all variables
    for (unsigned int n = 0; n < size(); n++) {
      Variable& v = *at(n);
      // if current variable does not overflow on increment, exit the loop
      if (v++) {
        _overflow = false;
        // if no overflow for the current variable, then no overflow and return
        // true
        return true;
      } else
        // if it overflows, reset it, and continue to the next variable in the
        // loop
        v.reset();
    }
    // we are here because overflow
    _overflow = true;
    return false;
  }

 public:
  // preincrement operator
  virtual bool operator++() {
    // increment and then return  0 if overflow, or return the resulting set of
    // variables
    return increment();
  }
  // postincrmemt operator
  virtual bool operator++(int) {
    // if last increment was an overflow, return 0
    if (_overflow)
      return false;
    else {
      bool b = hasNext();
      increment();
      return b;
    }
  }
  // returns true if increment possible without overflow
  // does not change any of the variables
  virtual bool hasNext() const {
    // for all variables
    for (unsigned int n = 0; n <= size(); n++) {
      const Variable* v = at(n);
      // if current variable does not overflow on increment, exit the loop
      if (v->hasNext()) return true;
    }
    // we are here because overflow
    return false;
  }
  // returns current set of params
  virtual PParameters getCurrent() {
    Parameters* p = new Parameters(size());

    for (unsigned int n = 0; n < size(); n++) p->setValue(n, *at(n));

    return PParameters(p);
  }
  // returns "best" set of params
  virtual PParameters best() { return PParameters(0); }
  virtual void reset() {
    for (unsigned int n = 0; n < size(); n++) at(n)->reset();
    _overflow = false;
  }
};

/**
 * An optimizer class.
 *
 * Two modes: optimize and get the optimal set of params, or optimize and then
 * run on the optimized params (adaptive mode)
 *
 * Note: this and all other related classes are preliminary code and are likely
 * to change in the future releases
 */
class CORE_API Optimizer : public Runnable {
 private:
  OptimizerParams& _params;
  Runnable& _runnable;
  // in optimization mode, replace the current positions with a temporary
  // object, on which will do the optimization
  // TODO: need to add a "clear" method
  // TODO: need to get slippage and commission for the temp positions
  std::auto_ptr<PositionsContainer> _posPtrList;
  std::auto_ptr<PositionsManager> _positions;
  // true: optimization mode
  // false: adaptive mode
  bool _mode;
  bool _last;

  unsigned int _state;

 public:
  // mode: true - optimize only, false, optimize and run with the optimized set
  // of params
  Optimizer::Optimizer(const std::wstring& name,
                       const std::wstring& description, Runnable& runnable,
                       OptimizerParams& params, bool mode)
      : Runnable(name, description),
        _params(params),
        _runnable(runnable),
        _mode(mode),
        _posPtrList(PositionsContainer::create()),
        _positions(PositionsManager::create(_posPtrList.get())),
        _state(0) {}

  virtual void run() {
    PParameters p;
    p = _state == 1 ? _params.best() : _params.getCurrent();
    _runnable.run();
    // clear temporary positions after run, so we won't print them if dump
    // chosen
    //    _positions -> reset();
  }

  virtual void init(const DataCollection* data, PositionsManager* positions) {
    // during optimization, pass the temporary
    if (_state == 1)
      // this is adaptvie mode and it's the last run with the optimal params
      _runnable.init(data, positions);
    else {
      // init temp positions with the same slippage, commission and filter as
      // the ones in the system
      _positions->init(*positions);
      // clear the list of trades in the temp positions
      _positions->reset();
      // init the runnable
      _runnable.init(data, &*_positions);
    }
  }

  // called before the next set of runs, so get the next params
  void begin() {
    //    _params.incrment();
  }

  bool again() {
    bool retValue;
    switch (_state) {
      case 0:
        if (++_params)
          retValue = true;
        else if (_mode)
          retValue = false;
        else {
          _state = 1;
          retValue = true;
        }
        break;
      case 1:
        retValue = false;
        break;
      default:
        // can only have 2 states
        assert(false);
        retValue = false;
        break;
    }
    return retValue;
  }

  /**
   * Pure virtual function that has to be implemented by the user in Runnable
   * derived classes.
   *
   * Contain initialization code.
   *
   * This method is called by the scheduler after all the default parameters
   * have been set and before calling the run method, thus giving a chance to
   * the user to do its own init work.
   *
   * It also gives a chance to stop the system from running if the return is
   * false
   *
   * @param symbol The symbol on which the system is about to be run
   * @return true - continue with run, false - stop the system
   */
  virtual bool init(const std::wstring& symbol) {
    _state = 0;
    return true;
  }
  /**
   * Pure virtual function that needs to be implemented by the user in Runnable
   * derived classes.
   *
   * Gives the user a chance to cleanup after the run.
   */
  virtual void cleanup() {}
};
