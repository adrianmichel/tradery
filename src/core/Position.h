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

#include <memory>
#include <optional>
#pragma warning(disable : 4800)
#pragma warning(default : 4800)

using namespace tradery;

// TODO: should I use exception here?
const size_t NO_POSITION = -1;

using std::pair;

/**
 * Template class for Settable - values that have a "not set" state
 * in addition to their values
 */


/**
 * One position leg, open or close.
 */
class PositionLeg {
 private:
  std::string _name;
  double _price;
  // TODO: should slippage be in the position?
  double _slippage;
  double _commission;
  // TODO: bar depends on the price series, so if positions are saved, bar value
  // may become invalid
  size_t _barIndex;
  DateTime _time;
  OrderType _orderType;

 public:
  PositionLeg(OrderType orderType, double price, double slippage, double commission, DateTime time, size_t barIndex, const std::string& name)
      : _orderType(orderType),
        _price(price),
        _slippage(slippage),
        _commission(commission),
        _time(time),
        _barIndex(barIndex),
        _name(name) {
    if (price == 0) {
      //			throw PositionZeroPriceException();
    }
  }
  virtual ~PositionLeg() {}

 public:
  OrderType getType() { return _orderType; }
  size_t getBarIndex() const { return _barIndex; }
  double getPrice() const { return _price; }
  const std::string& getName() const { return _name; }
  const DateTime& getTime() const { return _time; }
  double getCommission() const { return _commission; }
  double getSlippage() const { return _slippage; }
};

using PositionLegPtr = std::shared_ptr<PositionLeg>;

/**
 * PositionExtraInfo - has extra info per position, used for
 * handling that requires holding a staus info in time, for
 * example when doing trailingStop or breakEvenStop processing
 */
class PositionExtraInfo  // : ObjCount
{
 private:
  bool _breakEvenActive;
  bool _breakEvenLongActive;
  bool _breakEvenShortActive;
  bool _reverseBreakEvenActive;
  bool _reverseBreakEvenLongActive;
  bool _reverseBreakEvenShortActive;

  std::optional< double > _TTrailingStop;

 public:
  PositionExtraInfo()
      : _breakEvenActive(false),
        _reverseBreakEvenActive(false),
        _breakEvenLongActive(false),
        _breakEvenShortActive(false),
        _reverseBreakEvenLongActive(false),
        _reverseBreakEvenShortActive(false) {}

  bool isBreakEvenStopActive() const { return _breakEvenActive; }
  bool isBreakEvenStopLongActive() const { return _breakEvenLongActive; }
  bool isBreakEvenStopShortActive() const { return _breakEvenShortActive; }
  bool isReverseBreakEvenStopActive() const { return _reverseBreakEvenActive; }

  bool isReverseBreakEvenStopLongActive() const {
    return _reverseBreakEvenLongActive;
  }
  bool isReverseBreakEvenStopShortActive() const {
    return _reverseBreakEvenShortActive;
  }

  void activateBreakEvenStop() { _breakEvenActive = true; }
  void activateReverseBreakEvenStop() { _reverseBreakEvenActive = true; }
  void activateTrailingStop(double level) { _TTrailingStop = level; }
  bool isTrailingStopActive() const { return _TTrailingStop.has_value(); }
  double getTrailingStopLevel() const {
    // TODO: handle this case?
    // can't request the trailing value if it was not set
    assert(_TTrailingStop.has_value());
    return *_TTrailingStop;
  }
};

/**
 * An abstract Position. Concrete positions are short and long
 */

class PositionImpl : public PositionAbstr {
 private:
  // this is the id for all positions, will get incremented after a position is
  // created
  static std::atomic< PositionId > _uniqueId;
  const PositionUserData* _data;
  std::string _symbol;
  // number of shares before position sizing
  size_t _initialShares;
  // final number of shares, after position sizing
  size_t _shares;

  const std::string _userString;

  PositionLegPtr _openLeg;
  PositionLegPtr _closeLeg;

  PositionExtraInfo _extraInfo;
  // each position has an unique id
  const PositionId _id;
  const bool _applyPositionSizing;

 public:
  bool applyPositionSizing() const override { return _applyPositionSizing; }
  const std::string& getUserString() const override {
    return _userString;
  }
  PositionId getId() const override {
    assert(_id > 0);
    return _id;
  }

  // this sets the actual number of shares to 0
  void disable() { _shares = 0; }
  // sets the actual number of shares to a value different than that set
  // initially used during position sizing
  void setShares(size_t shares) { _shares = shares; }
  // if the actual number of shares is 0, the position is disabled
  bool isDisabled() const { return _shares == 0; }
  bool isEnabled() const { return _shares != 0; }
  bool isTrailingStopActive() const {
    return _extraInfo.isTrailingStopActive();
  }
  bool isBreakEvenStopActive() const {
    return _extraInfo.isBreakEvenStopActive();
  }
  bool isBreakEvenStopLongActive() const {
    return _extraInfo.isBreakEvenStopLongActive();
  }
  bool isBreakEvenStopShortActive() const {
    return _extraInfo.isBreakEvenStopShortActive();
  }
  bool isReverseBreakEvenStopActive() const {
    return _extraInfo.isReverseBreakEvenStopActive();
  }
  bool isReverseBreakEvenStopLongActive() const {
    return _extraInfo.isReverseBreakEvenStopLongActive();
  }
  bool isReverseBreakEvenStopShortActive() const {
    return _extraInfo.isReverseBreakEvenStopShortActive();
  }
  double getTrailingStopLevel() const {
    return _extraInfo.getTrailingStopLevel();
  }
  void activateTrailingStop(double level) {
    _extraInfo.activateTrailingStop(level);
  }
  void activateBreakEvenStop() { _extraInfo.activateBreakEvenStop(); }
  void activateReverseBreakEvenStop() {
    _extraInfo.activateReverseBreakEvenStop();
  }
  size_t getShares() const { return _shares; }
  void setPositionUserData(const PositionUserData* data) { _data = data; }
  const PositionUserData* getPositionUserData() const { return _data; }
  virtual bool isLong() const = 0;
  virtual bool isShort() const = 0;
  virtual void closeShort(OrderType orderType, double price, double slippage, double commission, DateTime time, size_t bar, const std::string& name) = 0;
  virtual void closeLong(OrderType orderType, double price, double slippage, double commission, DateTime time, size_t bar, const std::string& name) = 0;

  const std::string& getSymbol() const { return _symbol; }
  const bool isOpen() const { return _closeLeg.get() == 0; }

  const bool isClosed() const { return !isOpen(); }

  virtual OrderType getEntryOrderType() const {
    assert(_openLeg.get() != 0);
    return _openLeg->getType();
  }
  virtual OrderType getExitOrderType() const {
    if (!isClosed()) {
      throw PositionCloseOperationOnOpenPositionException("getExitType");
    }
    assert(_closeLeg.get() != 0);
    return _closeLeg->getType();
  }

  const DateTime getEntryTime() const { return _openLeg->getTime(); }

  const DateTime getCloseTime() const {
    if (!isClosed()) {
      throw PositionCloseOperationOnOpenPositionException("getCloseTime");
    }
    return _closeLeg->getTime();
  }

  size_t getEntryBar() const {
    assert(_openLeg.get() != 0);
    return _openLeg->getBarIndex();
  }

  size_t getCloseBar() const {
    if (!isClosed()) {
      throw PositionCloseOperationOnOpenPositionException("getCloseBar");
    }
    return _closeLeg->getBarIndex();
  }

  double getClosePrice() const override {
    if (!isClosed()) {
      throw PositionCloseOperationOnOpenPositionException("getClosePrice");
    }
    return _closeLeg->getPrice();
  }

  double getEntryPrice() const override {
    assert(_openLeg.get() != 0);
    return _openLeg->getPrice();
  }
  double getEntrySlippage() const override {
    assert(_openLeg.get() != 0);
    return _openLeg->getSlippage();
  }

  double getEntryCommission() const override {
    assert(_openLeg.get() != 0);
    return _openLeg->getCommission();
  }

  double getCloseSlippage() const override {
    if (!isClosed()) {
      throw PositionCloseOperationOnOpenPositionException("getCloseSlippage");
    }
    return _closeLeg->getSlippage();
  }
  double getCloseCommission() const override {
    if (!isClosed())
      throw PositionCloseOperationOnOpenPositionException("getCloseCommission");
    return _closeLeg->getCommission();
  }

  const std::string& getEntryName() const override {
    assert(_openLeg.get() != 0);
    return _openLeg->getName();
  }

  const std::string& getCloseName() const override {
    if (!isClosed())
      throw PositionCloseOperationOnOpenPositionException("getCloseName");
    return _closeLeg->getName();
  }

  double getPctGain() const override {
    if (!isClosed())
      throw PositionCloseOperationOnOpenPositionException("getPctGain");

    double entryCost = getEntryCost();
    entryCost = entryCost == 0 ? 0.01 : entryCost;

    return getGain() / entryCost * 100;
  }

  double getPctGain(double value) const override {
    return getGain(value) / getEntryCost() * 100;
  }
 protected:
  PositionImpl(OrderType orderType, const std::string& symbol, size_t shares, double price, double slippage, double commission, DateTime time,
               size_t bar, const std::string& name, const std::string& userString, bool applyPositionSizing, PositionId id)
      : _symbol(symbol),
        _shares(shares),
        _initialShares(shares),
        _openLeg(std::make_shared< PositionLeg >(orderType, price, slippage, commission, time, bar, name)),
        _id(id > 0 ? id : _uniqueId++),
        _userString(userString),
        _applyPositionSizing(applyPositionSizing) {
    assert(_id != 0);
  }

  void close( OrderType orderType, double price, double slippage, double commission, DateTime time, size_t bar, const std::string& name) {
    // TODO:  assert time > open time
    // TODO: warning if exit bar/date is lower or equal to entry bar/date (
    // exception?) - maybe an option that will allow exception to
    // be thrown or not
    // make sure we are closing an open position
    if (!isOpen()) {
      throw ClosingAlreadyClosedPositionException();
    }
    _closeLeg = std::make_shared< PositionLeg >(orderType, price, slippage, commission, time, bar, name);
  }

 public:
  virtual ~PositionImpl() {}
};

/**
 * A concrete short position
 */
class ShortPosition : public PositionImpl {
 public:
  ShortPosition(OrderType orderType, const std::string& symbol, size_t shares, double price, double slippage, double commission, DateTime time,
                size_t bar, const std::string& name, const std::string& userString, bool applyPositionSizing, PositionId id = 0)
      : PositionImpl(orderType, symbol, shares, price, slippage, commission, time, bar, name, userString, applyPositionSizing, id) {}

 public:
  bool isLong() const override { return false; }
  bool isShort() const override { return true; }
  void closeShort(OrderType orderType, double price, double slippage, double commission,DateTime time, size_t bar,const std::string& name) override {
    PositionImpl::close(orderType, price, slippage, commission, time, bar, name);
  }

  void closeLong (OrderType, double, double, double, DateTime, size_t, const std::string&) override {
    // covering instead of selling
    throw SellingShortPositionException();
  }

  double getGain() const override {
    return getEntryCost() - getCloseIncome();
  }

  double getGain(double price) const override {
    return getEntryCost() - price * getShares();
  }

  // for a short position, the entry cost is in fact an income, as the short
  // position is sold, so commission should be subtracted
  double getEntryCost() const override {
    return getEntryPrice() * getShares() - getEntryCommission();
  }

  double getEntryCost(size_t shares) const override {
    return getEntryPrice() * shares - getEntryCommission();
  }

  double getCloseIncome() const override {
    if (!isClosed()) {
      throw PositionCloseOperationOnOpenPositionException("getCloseCost");
    }

    return getClosePrice() * getShares() + getCloseCommission();
  }
};

/**
 * A concrete long position
 */
class LongPosition : public PositionImpl {
 public:
  LongPosition(OrderType orderType, const std::string& symbol, unsigned long shares, double price, double slippage, double commission,
    DateTime time, size_t bar, const std::string& name, const std::string& userString,bool applyPositionsSizing, PositionId id = 0)
      : PositionImpl(orderType, symbol, shares, price, slippage, commission, time, bar, name, userString, applyPositionsSizing, id) {}

 public:
  bool isLong() const override { return true; }
  bool isShort() const override { return false; }

  void closeLong(OrderType orderType, double price, double slippage, double commission, DateTime time, size_t bar, const std::string& name) override {
    PositionImpl::close(orderType, price, slippage, commission, time, bar, name);
  }

  void closeShort(OrderType, double, double, double, DateTime, size_t, const std::string&) override {
    // selling instead of covering
    throw CoveringLongPositionException();
  }

  double getGain() const override {
    return getCloseIncome() - getEntryCost();
  }

  double getGain(double price) const override {
    return getShares() * price - getEntryCost();
  }

  double getEntryCost(size_t shares) const override {
    return getEntryPrice() * shares + getEntryCommission();
  }

  double getEntryCost() const override {
    return getEntryPrice() * getShares() + getEntryCommission();
  }

  double getCloseIncome() const override {
    if (!isClosed()) {
      throw PositionCloseOperationOnOpenPositionException("getCloseCost");
    }

    return getClosePrice() * getShares() - getCloseCommission();
  }
};

// defines less entry time predicate
class LessEntryTimePredicate {
 public:
  bool operator()(const PositionAbstrPtr pos1, const PositionAbstrPtr pos2) const {
    assert(pos1.get() != 0 && pos2.get() != 0);
    if (pos1->getEntryTime() == pos2->getEntryTime()) {
      if (pos1->getEntryOrderType() != pos2->getEntryOrderType()) {
        return PositionAbstr::orderTypeLower(pos1->getEntryOrderType(), pos2->getEntryOrderType());
      }
      /*		else if( pos1->getExitOrderType() !=
         pos2->getExitOrderType() ) return PositionAbstr::orderTypeLower(
         pos1->getExitOrderType(), pos2->getExitOrderType() );
      */
      else if (pos1->getSymbol() != pos2->getSymbol()) {
        return pos1->getSymbol() < pos2->getSymbol();
      }
      else {
        return pos1.get() < pos2.get();
      }
    }
    else {
      return pos1->getEntryTime() < pos2->getEntryTime();
    }
  }
};

// defines less close time predicate
class LessCloseTimePredicate {
 public:
  bool operator()(const PositionAbstrPtr pos1, const PositionAbstrPtr pos2) const {
    assert(pos1.get() != 0 && pos2.get() != 0);
    if (pos1->isOpen()) {
      return true;
    }
    else if (pos2->isOpen()) {
      return false;
    }
    else if (pos1->getCloseTime() == pos2->getCloseTime()) {
      return PositionAbstr::orderTypeLower(pos1->getExitOrderType(), pos2->getExitOrderType());
    }
    else {
      return pos1->getCloseTime() < pos2->getCloseTime();
    }
  }
};

// defines less gain predicate
class LessGainPredicate {
 public:
  bool operator()(const PositionAbstrPtr pos1, const PositionAbstrPtr pos2) const {
    assert(pos1.get() != 0 && pos2.get() != 0);
    if (pos1->isOpen()) {
      return true;
    }
    else if (pos2->isOpen()) {
      return false;
    }
    else {
      return pos1->getGain() < pos2->getGain();
    }
  }
};

// adapter for externally defined arbirary predicate
class LessPredicate {
 private:
  PositionLessPredicate& _predicate;

 public:
  LessPredicate(PositionLessPredicate& predicate) : _predicate(predicate) {}

 public:
  bool operator()(const PositionAbstrPtr pos1, const PositionAbstrPtr pos2) const {
    return _predicate.less(pos1, pos2);
  }
};
