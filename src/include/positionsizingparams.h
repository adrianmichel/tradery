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

#include "datetime.h"
#include "misc.h"
#include "defaults.h"
#include "datetimerange.h"

class MaxOpenPositions {
 private:
  unsigned __int64 _maxOpenPos;

 public:
  MaxOpenPositions() : _maxOpenPos(0) {}
  MaxOpenPositions(__int64 mop) : _maxOpenPos(mop) {}

  bool unlimited() const { return _maxOpenPos == 0; }
  unsigned __int64 get() const {
    assert(!unlimited());
    return _maxOpenPos;
  }
  unsigned __int64 getValue() const { return _maxOpenPos; }
  void setUnlimited() { _maxOpenPos = 0; }
  void set(unsigned __int64 maxOpenPos) { _maxOpenPos = maxOpenPos; }

  std::string toString() const {
    return unlimited() ? "unlimited"s : std::to_string(_maxOpenPos);
  }
};

enum PosSizeType { system_defined, shares, size, pctEquity, pctCash };
enum PosSizeLimitType { none, pctVolume, limit };

class PositionSizingParams{
private:
  double _initialCapital;
  MaxOpenPositions _maxOpenPos;
  PosSizeType _posSizeType;
  double _posSize;
  PosSizeLimitType _posSizeLimitType;
  double _posSizeLimit;

protected:
  PositionSizingParams(double initialCapital,
    const MaxOpenPositions& maxOpenPos,
    PosSizeType posSizeType, double posSize,
    PosSizeLimitType posSizeLimitType,
    double posSizeLimit)
    : _initialCapital(initialCapital),
    _maxOpenPos(maxOpenPos),
    _posSizeType(posSizeType),
    _posSize(posSize),
    _posSizeLimitType(posSizeLimitType),
    _posSizeLimit(posSizeLimit) {}

public:

  PositionSizingParams()
    : _initialCapital(DEFAULT_INITIAL_CAPITAL),
    _maxOpenPos(DEFAULT_MAX_OPEN_POSITIONS),
    _posSizeType((PosSizeType)DEFAULT_POS_SIZE_TYPE),
    _posSize(DEFAULT_POSITION_SIZE),
    _posSizeLimitType(DEFAULT_POS_SIZE_LIMIT_TYPE),
    _posSizeLimit(DEFAULT_POSITION_SIZE_LIMIT) {}

  PositionSizingParams& operator=(
    const PositionSizingParams& psp) {
    setInitialCapital(psp.initialCapital());
    setMaxOpenPos(psp.maxOpenPos());
    setPosSizeType((PosSizeType)psp.posSizeType());
    setPosSize(psp.posSize());
    setPosSizeLimitType((PosSizeLimitType)psp.posSizeLimitType());
    setPosSizeLimit(psp.posSizeLimit());

    return *this;
  }

  double initialCapital() const { return _initialCapital; }
  const MaxOpenPositions maxOpenPos() const { return _maxOpenPos; }
  PosSizeType posSizeType() const { return _posSizeType; }
  double posSize() const { return _posSize; }
  PosSizeLimitType posSizeLimitType() const { return _posSizeLimitType; }
  double posSizeLimit() const { return _posSizeLimit; }

  void setInitialCapital(double capital) {
    _initialCapital = capital;
  }
  void setMaxOpenPos(const MaxOpenPositions& maxOpenPos) {
    _maxOpenPos = maxOpenPos;
  }
  void setPosSizeType(PosSizeType posSizeType) {
    _posSizeType = posSizeType;
  }
  void setPosSize(double posSize) {
    _posSize = posSize;
  }
  void setPosSizeLimitType(PosSizeLimitType posSizeLimitType) {
    _posSizeLimitType = posSizeLimitType;
  }
  void setPosSizeLimit(double posSizeLimit) {
    _posSizeLimit = posSizeLimit;
  }

  void set(const PositionSizingParams& psp) {
    operator=(psp);
  }
};

