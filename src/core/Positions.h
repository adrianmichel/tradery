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

#include "position.h"
#include "bars.h"

using std::list;

// TODO: make this thread safe for the case where multiple threads are writing
// positioins in the same list.

using BaseContainer = std::list<PositionAbstrPtr>;
using OpenPosBaseContainer = std::list<PositionAbstrPtr>;

/**
 * Vector of open positions.  Contains indexes to the actual positions
 * Gets adjusted as new positions are open and old ones are closed
 */
class OpenPositions : public OpenPosBaseContainer {
 private:
  iterator find(const PositionAbstrPtr pos) {
    assert(pos);
    // take into account that there can be closed positions because we defer
    // deleting of them
    return std::find(begin(), end(), pos);
  }

  const_iterator find(const PositionAbstrPtr pos) const {
    assert(pos);
    // take into account that there can be closed positions because we defer
    // deleting of them
    return std::find(begin(), end(), pos);
  }

  bool hasPosition(const PositionAbstrPtr pos) const {
    assert(pos);
    return find(pos) != end() && pos->isOpen();
  }

 public:
  OpenPositions() {}
  void add(PositionAbstrPtr pos) {
    assert(pos);
    // can only add an open position
    assert(pos->isOpen());
    // cannot add the same position twice
    assert(find(pos) == end());
    push_back(pos);
  }

  void append(OpenPositions& openPos) {
    splice(end(), openPos);
    openPos.clear();
  }

  void remove(const PositionAbstrPtr pos) {
    assert(pos);
    // only remove a position after it has been closed
    assert(pos->isClosed());
    // to remove a position, it has to be there in the first place
    assert(find(pos) != end());
    // don't do this now, defer until we need to read the positions, in
    // forEach...
    //    erase( find( pos ) );
  }

  PositionAbstrPtr getLast() {
    // we need to do this as the last position may be closed (we defer removing
    // the closed positions until it's necessary, like now)
    while (!empty()) {
      if (back()->isClosed()) {
        // if last position has been closed, remove it and repeat the process
        erase(--end());
      }
      else {
        // else, return the last open positions
        return back();
      }
    }
    return 0;
  }

  size_t getCount() const {
    OpenPositions* p = const_cast<OpenPositions*>(this);
    // get rid of closed positions before returning the size
    for (iterator i = p->begin(); i != p->end();) {
      if ((*i)->isClosed()) {
        i = p->erase(i);
      }
      else {
        ++i;
      }
    }
    return p->size();
  }

  // calls handler for each open position, and passes bar too.
  // the trick is that it also removes positions that are no longer open, which
  // improves performance tremendously for lists with lots of positions and open
  // positions (instead of doing it every time a position is closed, we defer
  // removing them until it's necessary, and we do it in block)
  void forEachOpenPosition(OpenPositionHandler& openPositionHandler, Bars bars, size_t bar) {
    forEachOpenPosition(openPositionHandler, bars, bar, PositionEqualAllPredicate());
  }

  void forEachOpenPosition(OpenPositionHandler1& openPositionHandler) {
    forEachOpenPosition(openPositionHandler, PositionEqualAllPredicate());
  }

  virtual void forEachOpenPosition(OpenPositionHandler& openPositionHandler, Bars bars, size_t bar, const PositionEqualPredicate& pred) {
    for (iterator i = begin(); i != end();) {
      // we do this for positions that are closed - they need to be removed
      // (left from calls to remove, which defers the remove to this method)
      tradery::Position pos(*i);

      assert(pos);

      if (pos.isClosed()) {
        i = erase(i);
      }
      else {
        if (pred == pos && !pos.isDisabled()) {
          if (!openPositionHandler.onOpenPosition(pos, bars, bar)) break;
        }
        ++i;
      }
    }
  }

  virtual void forEachOpenPosition(OpenPositionHandler1& openPositionHandler, const PositionEqualPredicate& pred) {
    for (iterator i = begin(); i != end();) {
      // we do this for positions that are closed - they need to be removed
      // (left from calls to remove, which defers the remove to this method)
      tradery::Position pos(*i);
      assert(pos);

      if (pos.isClosed()) {
        i = erase(i);
      }
      else {
        if (pred == pos && !pos.isDisabled()) {
          if (!openPositionHandler.onOpenPosition(pos)) break;
        }
        ++i;
      }
    }
  }

  void clear() {}
};

class OpenPositionsIteratorImpl : public OpenPositionsIteratorAbstr {
 private:
  OpenPositions& _op;
  OpenPositions::iterator _i;

 public:
  OpenPositionsIteratorImpl(OpenPositions& op) : _op(op), _i(op.begin()) {}

  Position getFirst() {
    _i = _op.begin();
    return getNext();
  }

  Position getNext() {
    if (_i != _op.end()) {
      tradery::Position pos(*_i);
      if (pos.isClosed()) {
        _op.erase(_i++);
        return getNext();
      }
      else {
        ++_i;
        return pos;
      }
    }
    else {
      return tradery::Position();
    }
  }
};

class PositionsIteratorImpl;

using PositionIdToPositionMap = std::map<PositionId, PositionAbstrPtr>;

// class PositionsPtrList : private PosPtrList, public PositionsContainer
class PositionsContainerImpl : private BaseContainer, public PositionsContainer {
  friend PositionsIteratorImpl;

 private:
  mutable OpenPositions _openPositions;

  // todo: make this an on demand map
  PositionIdToPositionMap _idsToPositions;

 public:
  ~PositionsContainerImpl() override {}
  PositionsContainerImpl() {}

 public:
  virtual OpenPositionsIterator getOpenPositionsIterator() {
    return OpenPositionsIterator(std::make_shared< OpenPositionsIteratorImpl >(_openPositions));
  }

  virtual tradery::Position getPosition(PositionId id) {
    PositionIdToPositionMap::iterator i = _idsToPositions.find(id);

    return i != _idsToPositions.end() ? (*i).second : nullptr;
  }

  /**
   * Appends the contents of the argument list to the current list. It also
   * empties the argument list in the process, but without deleting anything. It
   * is equivalent to moving the contents of the argument list over to the end
   * of the current list
   *
   * @param posList Source list
   */
  void add(PositionAbstrPtr pos) {
    assert(pos);
    BaseContainer::push_back(pos);
    if (pos->isOpen()) {
      _openPositions.add(pos);
    }
    // make sure the id is unique
    // todo: assert
    //
    //    std::cout << _T( "adding position with id: " ) << pos->getId() <<
    //    std::endl;
    bool b = _idsToPositions.insert(PositionIdToPositionMap::value_type(pos->getId(), pos)).second;

    assert(b);
  }

  void append(PositionsContainer* posContainer) override {
    // TODO: throw an exception if posList is 0
    assert(posContainer != 0);
    try {
      PositionsContainerImpl* p = dynamic_cast<PositionsContainerImpl*>(posContainer);
      // attach all the elements in the map
      // and erase the original map
      // todo: make this more efficient

      _idsToPositions.insert(p->_idsToPositions.begin(), p->_idsToPositions.end());
      p->_idsToPositions.erase(p->_idsToPositions.begin(), p->_idsToPositions.end());

      _openPositions.append(p->_openPositions);
      BaseContainer::splice(end(), *p);

    }
    catch (const std::bad_cast&) {
      // TODO: throw an exception for the user to tell him he cannot derive from
      // Position
      assert(false);
    }
  }

  void nonDestructiveAppend(PositionsContainer* posContainer) override {
    assert(posContainer != 0);

    try {
      PositionsContainerImpl* p = dynamic_cast<PositionsContainerImpl*>(posContainer);
      // todo: does this just make a copy of the elements, or does it remove
      // them from the source sequance?

      PositionsContainerImpl newPC = *p;
      append(&newPC);
    }
    catch (const std::bad_cast&) {
      // TODO: throw an exception for the user to tell him he cannot derive from
      // Position
      assert(false);
    }
  }

  size_t count() const override { return BaseContainer::size(); }

  size_t enabledCount() const override {
    size_t n = 0;
    for (BaseContainer::const_iterator i = __super::begin(); i != __super::end(); i++) {
      if ((*i)->isEnabled()) {
        n++;
      }
    }

    return n;
  }

  tradery::Position getLastPosition() override {
    return empty() ? Position() : BaseContainer::back();
  }

  size_t openPositionsCount() const override {
    return _openPositions.getCount();
  }

  void close(const PositionAbstrPtr pos) { _openPositions.remove(pos); }

  tradery::Position getLastOpenPosition() override {
    return _openPositions.getLast();
  }

  const tradery::Position getLastOpenPosition() const override {
    return _openPositions.getLast();
  }

  void clear() override {
    BaseContainer::clear();
    _openPositions.clear();
  }

  /**
   * Predefined sort, sorting by position entry time
   *
   * @param ascending true for ascending, false for descending, default
   * ascending
   * @see sort
   */
  void sortByEntryTime(bool ascending = true) override {
    LOG(log_info, "Sorting positions by entry time");
    if (ascending) {
      BaseContainer::sort(LessEntryTimePredicate());
    }
    else {
      BaseContainer::sort(
        [](PositionAbstrPtr a, PositionAbstrPtr b)->bool {
          return !LessEntryTimePredicate()(a, b);
        }
      );
    }
  }
  /**
   * Predefined sort, sorting by position exit time
   *
   * @param ascending true for ascending, false for descending, default
   * ascending
   * @see sort
   */
  void sortByExitTime(bool ascending = true) override {
    if (ascending) {
      BaseContainer::sort(LessCloseTimePredicate());
    }
    else {
      BaseContainer::sort(
        [](PositionAbstrPtr a, PositionAbstrPtr b)->bool {
          return !LessCloseTimePredicate()(a, b);
        }
      );
    }
  }
  /**
   * Predefined sort, sorting by position gain (exit price - entry price)
   * Not closed position will be last (or first for descending).
   *
   * @param ascending true for ascending, false for descending, default
   * ascending
   * @see sort
   */
  void sortByGain(bool ascending = true) override {
    if (ascending) {
      BaseContainer::sort(LessGainPredicate());
    }
    else {
      BaseContainer::sort(
        [](PositionAbstrPtr a, PositionAbstrPtr b)->bool {
          return !LessGainPredicate()(a, b);
        }
      );
    }
  }
  /**
   * Reverses the order of all positions in the list.
   */
  void reverse() override { std::reverse(begin(), end()); }
  /**
   * General sort method, that takes a user defined comparison predicate as
   * parameter.
   *
   * In order to use it, a user will have to wite a concrete class derived from
   * PositionLess and to implement the less method.
   *
   * Thus, sorting can be done in any possible order, given the right predicate,
   * including sorting by multiple fields.
   *
   * @param predicate On object implementing the comparison method for the sort.
   * @param ascending true for ascending sorting, false for descending sorting,
   * true default value
   * @see PositionLess
   */
  void sort(PositionLessPredicate& predicate, bool ascending = true) override {
    if (ascending) {
      BaseContainer::sort(LessPredicate(predicate));
    }
    else {
      BaseContainer::sort(
        [&predicate](PositionAbstrPtr a, PositionAbstrPtr b)->bool {
          return !LessPredicate(predicate)(a, b);
        }
      );
    }
  }

  void forEach(PositionHandler& op) override {
    for (auto pos : *this) {
      if (!pos->isDisabled()) {
        op.onPosition(pos);
      }
    }
  }

  void forEach(PositionHandler& op, const PositionEqualPredicate& pred) override {
    for (auto pos : *this) {
      if (pred == pos && !pos->isDisabled()) {
        op.onPosition(pos);
      }
    }
  }

  void forEachNot(PositionHandler& op, const PositionEqualPredicate& pred) override {
    for (auto pos : *this) {
      if (pred != pos && !pos->isDisabled()) {
        op.onPosition(pos);
      }
    }
  }

  void forEach(PositionEqualPredHandler& predHandler) override {
    for (auto pos : *this) {
      if (predHandler == pos && !pos->isDisabled()) {
        predHandler.onPosition(pos);
      }
    }
  }

  void forEachNot(PositionEqualPredHandler& predHandler) override {
    for (auto pos : *this) {
      if (predHandler != pos && !pos->isDisabled()) {
        predHandler.onPosition(pos);
      }
    }
  }

  void forEachOr(PositionHandler& positionHandler, std::vector<PositionEqualPredicate*> predicates) override {
    for (auto pos : *this) {
      if (!pos->isDisabled()) {
        for (auto predicate : predicates ) {
          if (*predicate == pos) {
            positionHandler.onPosition(pos);
            break;
          }
        }
      }
    }
  }

  void forEachAnd(PositionHandler& positionHandler, std::vector<PositionEqualPredicate*> predicates) override {
    for (auto pos : *this) {
      if (!pos->isDisabled()) {
        bool b = true;
        for (auto predicate : predicates ) {
          if (*predicate != pos) {
            b = false;
            break;
          }
        }
        if (b) positionHandler.onPosition(pos);
      }
    }
  }

  void forEachConst(PositionHandler& op) const override {
    for (auto pos : *this ) {
      if (!pos->isDisabled()) {
        op.onPosition(pos);
      }
    }
  }

  void forEachConst(PositionHandler& op, PositionEqualPredicate& pred) const override {
    for (auto pos : *this) {
      if (pred == pos && !pos->isDisabled()) op.onPosition(pos);
    }
  }

  // this goes through all open pos and closes the ones for which "pr" returns
  // true. this should be used as it guarantees the integrity of the list after
  // removing elements
  void forEachOpenPosition(OpenPositionHandler& openPositionHandler, Bars bars, size_t bar) override {
    _openPositions.forEachOpenPosition(openPositionHandler, bars, bar);
  }

  void forEachOpenPosition(OpenPositionHandler1& openPositionHandler) override {
    _openPositions.forEachOpenPosition(openPositionHandler);
  }

  void forEachOpenPosition(OpenPositionHandler& openPositionHandler, Bars bars, size_t bar, const PositionEqualPredicate& pred) {
    _openPositions.forEachOpenPosition(openPositionHandler, bars, bar, pred);
  }

  void forEachOpenPosition(OpenPositionHandler1& openPositionHandler, const PositionEqualPredicate& pred) override {
    _openPositions.forEachOpenPosition(openPositionHandler, pred);
  }

  void forEachClosed(PositionHandler& op) override {
    for (auto pos : *this) {
      if (pos->isClosed() && !pos->isDisabled()) op.onPosition(pos);
    }
  }
    
  void forEachClosedConst(PositionHandler& op) const override {
    for (auto pos : *this) {
      if (pos->isClosed() && !pos->isDisabled()) op.onPosition(pos);
    }
  }
};

using PositionsContainerImplPtr = std::shared_ptr< PositionsContainerImpl >;

/**
 * Position trailing stop data
 */
class TrailingStopData{
private:
  using DoublePair = pair<double, double>;
  std::optional< DoublePair > m_trailingStopData;

 public:
  void set(double trigger, double level) {
    m_trailingStopData = DoublePair( trigger, level);
  }
  double getTrigger() const { return m_trailingStopData->first; }
  double getLevel() const { return m_trailingStopData->second; }

  operator bool() const { return m_trailingStopData.has_value(); }
};

class SignalHandlerCollection : public SignalHandler, public std::vector<SignalHandler*> {
 public:
  SignalHandlerCollection()
      : SignalHandler(Info("45ED02AB-C2A7-4c25-9E66-24DB06E239A2",
                           "Signal handler collection",
                           "Signal handler collection")) {}

  void add(SignalHandler* signalHandler) {
    //  only add non-null signal handlers
    if (signalHandler != 0) push_back(signalHandler);
  }

  virtual void signal(SignalPtr _signal) {
    for (auto handler : *this ) {
      assert(handler);
      handler->signal(_signal);
    }
  }
};

/**
 * Manages positions associated with a system
 *
 * It is passed a pointer to a positionPtrList object, which
 * will contain the actual positions.
 */
class PositionsManagerImpl : public PositionsManagerAbstr  //, ObjCount
{
  friend class CX;

 private:
  Slippage* _slippage;
  Commission* _commission;
  const DateTime _startTrades;  // start time (inclusive) after which generate trades
  const DateTime _endTrades;  // end time (exclusive) until which generate trades
  std::optional< double > _breakEvenStop;
  std::optional< double > _breakEvenStopLong;
  std::optional< double > _breakEvenStopShort;
  std::optional< double > _reverseBreakEvenStop;
  std::optional< double > _reverseBreakEvenStopLong;
  std::optional< double > _reverseBreakEvenStopShort;
  std::optional< double > _profitTarget;
  std::optional< double > _profitTargetShort;
  std::optional< double > _profitTargetLong;
  std::optional< double > _stopLoss;
  std::optional< double > _stopLossLong;
  std::optional< double > _stopLossShort;
  std::optional< int > _timeBasedExitAtMarket;
  std::optional< int > _timeBasedExitAtClose;
  TrailingStopData _TTrailingStop;
  SignalHandlerCollection _signalHandlers;
  PositionsContainerImplPtr _posContainer;
  std::string _systemName;
  std::string _systemId;

  bool _acceptVolume0;

  OrderFilter* _orderFilter;

 private:
  void validateSymbol(Bars bars, Position pos) const {
    if (bars.getSymbol() != pos.getSymbol()) {
      throw ClosingPostionOnDifferentSymbolException(pos.getSymbol(), bars.getSymbol());
    }
  }

  void validateLimitPrice(size_t barIndex, double price) {
    if (price <= 0) {
      throw InvalidLimitPriceException(barIndex, price);
    }
  }

  void validateStopPrice(size_t barIndex, double price) {
    if (price <= 0) {
      throw InvalidStopPriceException(barIndex, price);
    }
  }

 public:
  void setSystemName(const std::string& str) override {
    _systemName = str;
  }

  const std::string& systemName() const override { return _systemName; }
  void setSystemId(const std::string& str) override {
    _systemId = str;
  }
  const std::string& systemId() const override { return _systemId; }

  tradery::Position getPosition(PositionId id) override {
    assert(_posContainer != 0);
    return _posContainer->getPosition(id);
  }

  PositionsManagerImpl(PositionsContainer::PositionsContainerPtr posContainer, DateTime startTrades, DateTime endTrades, Slippage* slippage = 0, Commission* commission = 0, bool acceptVolume0 = true)
      : _slippage(slippage),
        _commission(commission),
        _orderFilter(0),
        _posContainer(std::dynamic_pointer_cast<PositionsContainerImpl>(posContainer)),
        _startTrades(startTrades),
        _endTrades(endTrades),
        _acceptVolume0(acceptVolume0) {
    // TODO: throw exception if pointer is 0
    assert(_posContainer != 0);
  }

  ~PositionsManagerImpl() override {}

  void forEachOpenPosition(OpenPositionHandler& openPositionHandler, Bars bars, size_t bar) override {
    _posContainer->forEachOpenPosition(openPositionHandler, bars, bar);
  }

  // TODO: add parameters to all the filters
  double calculateSlippage(unsigned long shares, unsigned long volume, double price) const {
    return _slippage == 0 ? 0 : _slippage->getValue(shares, volume, price);
  }

  double calculateCommission(unsigned long shares, double price) const {
    return _commission == 0 ? 0 : _commission->getValue(shares, price);
  }

 public:
  OrderFilter* registerOrderFilter(OrderFilter* orderFilter) override {
    std::swap(orderFilter, _orderFilter);
    return orderFilter;
  }

  // can be 0
  void registerSignalHandler(SignalHandler* al) override {
    _signalHandlers.add(al);
  }

  void registerSignalHandlers(std::vector<SignalHandler*> ah) override {
    for (auto handler : ah) {
      if (handler != nullptr) {
        _signalHandlers.push_back(handler);
      }
    }
  }

 public:
  void installTimeBasedExitAtMarket(Index bars) {
    _timeBasedExitAtMarket = bars;
  }

  void installTimeBasedExitAtClose(Index bars) {
    _timeBasedExitAtClose = bars;
  }

  void installTimeBasedExit(Index bars) { _timeBasedExitAtMarket = bars; }

  void installReverseBreakEvenStop(double level) {
    _reverseBreakEvenStop = level;
  }

  void installReverseBreakEvenStopLong(double level) {
    _reverseBreakEvenStopLong = level;
  }

  void installReverseBreakEvenStopShort(double level) {
    _reverseBreakEvenStopShort = level;
  }

  void installStopLoss(double level) { _stopLoss = level; }

  void installStopLossLong(double level) { _stopLossLong = level; }

  void installStopLossShort(double level) { _stopLossShort = level; }

  void installProfitTarget(double level) { _profitTarget = level; }

  void installProfitTargetLong(double level) { _profitTargetLong = level; }

  void installProfitTargetShort(double level) { _profitTargetShort = level; }

  void installTrailingStop(double trigger, double level) {
    _TTrailingStop.set(trigger, level);
  }

  void installBreakEvenStop(double level) { _breakEvenStop = level; }

  void installBreakEvenStopLong(double level) { _breakEvenStopLong = level; }

  void installBreakEvenStopShort(double level) {
    _breakEvenStopShort = level;
  }


 public:
 private:
  std::shared_ptr< tradery::PositionAbstr > openShort(tradery::OrderType orderType, const std::string& symbol, size_t shares,
      double price, double slippage, double commission, DateTime time, size_t bar, const std::string& name,
      const std::string& userString, bool applyPositionsSizing, PositionId id = 0) {
    assert(_posContainer != 0);
    // TODO: calculate slippage using volume

    std::shared_ptr< tradery::PositionAbstr > pos = std::make_shared< ShortPosition > (orderType, symbol, shares, price, slippage, commission, time, bar, name,
        userString, applyPositionsSizing, id);
    _posContainer->add(pos);
    return pos;
  }

  void closeShort( tradery::OrderType orderType, tradery::Position p, double price, double slippage, double commission, DateTime time, size_t bar, const std::string& name) {
    assert(_posContainer != 0);

    try {
      assert(p);
      PositionImpl& pos = dynamic_cast<PositionImpl&>(*p.getPos());
      if (pos.isClosed()) {
        throw ClosingAlreadyClosedPositionException();
      }
      else {
        // TODO: make sure we are closing a position of the same type that was
        // opened throw exception if not close the position
        // TODO: calculate slippage using volume
        pos.closeShort(orderType, price, slippage, commission, time, bar, name);

        // this removes the position from open positions
        _posContainer->close(p.getPos());
      }
    }
    catch (const std::bad_cast&) {
      // TODO: throw an exception for the user to tell him he cannot derive from
      // Position
      assert(false);
    }
  }

  PositionAbstrPtr openLong(tradery::OrderType orderType, const std::string& symbol, unsigned long shares, double price, double slippage, double commission,
                         DateTime time, size_t bar, const std::string& name, const std::string& userString, bool applyPositionsSizing, PositionId id = 0) {
    // TODO: calculate slippage using volume
    assert(_posContainer != 0);
    auto pos = std::make_shared< LongPosition >(orderType, symbol, shares, price, slippage, commission, time, bar, name, userString, applyPositionsSizing, id);
    _posContainer->add(pos);
    return pos;
  }

  void closeLong(tradery::OrderType orderType, tradery::Position p, double price, double slippage, double commission, DateTime time, size_t bar, const std::string& name) {
    assert(_posContainer != 0);

    try {
      assert(p);
      PositionImpl& pos = dynamic_cast<PositionImpl&>(*p.getPos());
      if (pos.isClosed()) {
        throw ClosingAlreadyClosedPositionException();
      }
      else {
        // TODO: make sure we are closing a position of the same type that was
        // opened throw exception if not close the position
        // TODO: calculate slippage using volume
        pos.closeLong(orderType, price, slippage, commission, time, bar, name);

        // remove from open positions
        _posContainer->close(p.getPos());
      }
    }
    catch (const std::bad_cast&) {
      // TODO: throw an exception for the user to tell him he cannot derive from
      // PositionsContainer
      assert(false);
    }
  }

  void setPositionUserData(tradery::PositionAbstr* pos, const PositionUserData* data) {
    pos->setPositionUserData(data);
  }

  const PositionUserData* getPositionUserData(tradery::PositionAbstr* pos) const {
    return pos->getPositionUserData();
  }

public:
  void applyTimeBased(Bars bars, Index barIndex, tradery::Position pos) override {
    applyTimeBasedAtMarket(bars, barIndex, pos);
  }
  void applyTimeBasedAtMarket( Bars bars, Index barIndex, tradery::Position pos) override;
  void applyTimeBasedAtClose( Bars bars, Index barIndex, tradery::Position pos) override;
  void applyStopLoss(Bars bars, Index barIndex, tradery::Position pos) override;
  void applyStopLossLong( Bars bars, Index barIndex, tradery::Position pos) override;
  void applyStopLossShort(Bars bars, Index barIndex, tradery::Position pos) override;
  void applyTrailing(Bars bars, Index barIndex, tradery::Position pos) override;
  void applyBreakEvenStop(Bars bars, Index barIndex, tradery::Position pos) override;
  void applyBreakEvenStopLong(Bars bars, Index barIndex, tradery::Position pos) override;
  void applyBreakEvenStopShort(Bars bars, Index barIndex, tradery::Position pos) override;
  void applyReverseBreakEvenStop(Bars bars, Index barIndex, tradery::Position pos) override;
  void applyReverseBreakEvenStopLong(Bars bars, Index barIndex, tradery::Position pos) override;
  void applyReverseBreakEvenStopShort(Bars bars, Index barIndex, tradery::Position pos) override;
  void applyProfitTarget(Bars bars, Index barIndex, tradery::Position pos) override;
  void applyProfitTargetLong(Bars bars, Index barIndex, tradery::Position pos) override;
  void applyProfitTargetShort(Bars bars, Index barIndex, tradery::Position pos) override;
  void applyAutoStops(Bars bars, Index barIndex) override;

private:
  void applyAutoStops(Bars bs, Index barIndex, tradery::Position pos);

public:
  PositionId buyAtMarket(Bars bars, Index barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) override;
  PositionId buyAtClose(Bars bars, Index barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) override;
  PositionId buyAtStop(Bars bars, Index barIndex, double stopPrice, size_t shares, const std::string& name, bool applyPositionSizing = true) override;
  PositionId buyAtPrice(Bars bars, size_t barIndex, double price, size_t shares, const std::string& name, bool applyPositionSizing, PositionId id) override;
  PositionId shortAtPrice(Bars bars, size_t barIndex, double price, size_t shares, const std::string& name, bool applyPositionSizing, PositionId id) override;
  PositionId buyAtLimit(Bars bars, Index barIndex, double limitPrice, size_t shares, const std::string& name, bool applyPositionSizing = true) override;
  bool sellAtMarket(Bars bars, Index barIndex, tradery::Position pos, const std::string& name) override;
  bool sellAtClose(Bars bars, Index barIndex, tradery::Position pos, const std::string& name) override;
  bool sellAtStop(Bars bars, Index barIndex, tradery::Position pos, double stopPrice, const std::string& name) override;
  bool sellAtLimit(Bars bars, Index barIndex, tradery::Position pos, double limitPrice, const std::string& name) override;
  PositionId shortAtMarket(Bars bars, Index barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) override;
  PositionId shortAtClose(Bars bars, Index barIndex, size_t shares, const std::string& name, bool applyPositionSizing = true) override;
  PositionId shortAtStop(Bars bars, Index barIndex, double stopPrice, size_t shares, const std::string& name, bool applyPositionSizing = true) override;
  PositionId shortAtLimit(Bars bars, Index barIndex, double limitPrice, size_t shares, const std::string& name, bool applyPositionSizing = true) override;
  bool coverAtMarket(Bars bars, Index barIndex, tradery::Position pos, const std::string& name) override;
  bool coverAtClose(Bars bars, Index barIndex, tradery::Position pos, const std::string& name) override;
  bool coverAtStop(Bars bars, Index barIndex, tradery::Position pos, double stopPrice, const std::string& name) override;
  bool coverAtLimit(Bars bars, Index barIndex, tradery::Position pos, double limitPrice, const std::string& name) override;

  bool sellAtMarket(Bars bars, size_t barIndex, PositionId pos, const std::string& name) override;
  bool sellAtClose(Bars bars, size_t barIndex, PositionId pos, const std::string& name) override;
  bool sellAtStop(Bars bars, size_t barIndex, PositionId pos, double stopPrice, const std::string& name) override;
  bool sellAtLimit(Bars bars, size_t barIndex, PositionId pos, double limitPrice, const std::string& name) override;
  bool coverAtMarket(Bars bars, size_t barIndex, PositionId pos, const std::string& name) override;
  bool coverAtClose(Bars bars, size_t barIndex, PositionId pos, const std::string& name) override;
  bool coverAtStop(Bars bars, size_t barIndex, PositionId pos, double stopPrice, const std::string& name) override;
  bool coverAtLimit(Bars bars, size_t barIndex, PositionId pos, double limitPrice, const std::string& name) override;

  virtual tradery::Position getLastPosition() {
    assert(_posContainer != 0);
    return _posContainer->getLastPosition();
  }

  /**
   * Returns 0 if there is no open position
   *
   * @return
   */
  const tradery::Position getLastOpenPosition() const {
    assert(_posContainer != 0);
    return _posContainer->getLastOpenPosition();
  }

  tradery::Position getLastOpenPosition() {
    assert(_posContainer != 0);
    return _posContainer->getLastOpenPosition();
  }

  size_t openPositionsCount() const {
    assert(_posContainer != 0);
    return _posContainer->openPositionsCount();
  }

  /**
   * returns NO_POSITION if there are no positions, open or closed.
   *
   * @return
   */
  tradery::Position getLastPosition() const {
    assert(_posContainer != 0);
    return _posContainer->getLastPosition();
  }

  bool isLastPositionOpen() const { return getLastPosition().isOpen(); }

  double positionBasisPrice() const {
    // TODO - positionBasisPrice
    // TODO - throw NotImplementedException until then
  }

  size_t positionCount() const {
    assert(_posContainer != 0);
    return _posContainer->count();
  }

  void reset() override {
    assert(_posContainer != 0);
    _posContainer->clear();
    if (_commission != 0) {
      _commission->reset();
    }

    if (_slippage != 0) {
      _slippage->reset();
    }
  }

  void init(PositionsManagerAbstr& positions) override {
    // TODO: not entirely ok - what happens with the existing slippage, etc
    // and what happens when multiple positions point to the same slippage etc
    PositionsManagerImpl& p = dynamic_cast<PositionsManagerImpl&>(positions);
    _slippage = p._slippage;
    _commission = p._commission;
    this->registerOrderFilter(p._orderFilter);
  }

  class CloseAtMarketHandler : public OpenPositionHandler {
    const std::string _name;
    PositionsManagerAbstr& _pm;
    bool _first;

   public:
    CloseAtMarketHandler(const std::string& name, PositionsManagerAbstr& pm, bool first = false)
        : _name(name), _pm(pm), _first(first) {}

    bool onOpenPosition(tradery::Position pos, Bars bars, size_t bar) override {
      if (pos.isLong()) {
        _pm.sellAtMarket(bars, bar, pos, _name);
      }
      else {
        _pm.coverAtMarket(bars, bar, pos, _name);
      }

      // if first, return false so we won't do any more iterations
      return !_first;
    }
  };

  class CloseAtCloseHandler : public OpenPositionHandler {
    const std::string _name;
    PositionsManagerAbstr& _pm;

   public:
    CloseAtCloseHandler(const std::string& name, PositionsManagerAbstr& pm)
        : _name(name), _pm(pm) {}

    bool onOpenPosition(tradery::Position pos, Bars bars, size_t bar) override {
      if (pos.isLong()) {
        _pm.sellAtClose(bars, bar, pos, _name);
      }
      else {
        _pm.coverAtClose(bars, bar, pos, _name);
      }

      return true;
    }
  };

  class CloseAtLimitHandler : public OpenPositionHandler {
    const std::string _name;
    PositionsManagerAbstr& _pm;
    const double _price;

   public:
    CloseAtLimitHandler(const std::string& name, PositionsManagerAbstr& pm, double price)
        : _name(name), _pm(pm), _price(price) {}

    bool onOpenPosition(tradery::Position pos, Bars bars, size_t bar) override {
      if (pos.isLong()) {
        _pm.sellAtLimit(bars, bar, pos, _price, _name);
      }
      else {
        _pm.coverAtLimit(bars, bar, pos, _price, _name);
      }

      return true;
    }
  };

  class CloseAtStopHandler : public OpenPositionHandler {
    const std::string _name;
    PositionsManagerAbstr& _pm;
    const double _price;

   public:
    CloseAtStopHandler(const std::string& name, PositionsManagerAbstr& pm, double price)
        : _name(name), _pm(pm), _price(price) {}

    bool onOpenPosition(tradery::Position pos, Bars bars, size_t bar) override {
      if (pos.isLong()) {
        _pm.sellAtStop(bars, bar, pos, _price, _name);
      }
      else {
        _pm.coverAtStop(bars, bar, pos, _price, _name);
      }

      return true;
    }
  };

  class PositionShortEqualSharesPredicate : public PositionEqualPredicate {
   private:
    unsigned long _shares;

   public:
    PositionShortEqualSharesPredicate(unsigned long shares) : _shares(shares) {}

    /**
     * Returns true if the position is short
     *
     * @param position The position to be tested
     *
     * @return true if the position is short, false otherwise
     */
    bool operator==(const tradery::Position position) const override {
      return position.getShares() == _shares && position.isShort();
    }
  };

  class PositionLongEqualSharesPredicate : public PositionEqualPredicate {
   private:
    unsigned long _shares;

   public:
    PositionLongEqualSharesPredicate(unsigned long shares) : _shares(shares) {}

    /**
     * Returns true if the position is short
     *
     * @param position The position to be tested
     *
     * @return true if the position is short, false otherwise
     */
    bool operator==(const tradery::Position position) const override {
      return position.getShares() == _shares && position.isLong();
    }
  };

  void closeAllAtMarket(Bars bars, size_t barIndex, const std::string& name) override {
    forEachOpenPosition(CloseAtMarketHandler(name, *this), bars, barIndex);
  }
  void closeAllShortAtMarket(Bars bars, size_t barIndex, const std::string& name) override {
    _posContainer->forEachOpenPosition(CloseAtMarketHandler(name, *this), bars, barIndex, PositionEqualShortPredicate());
  }
  void closeAllLongAtMarket(Bars bars, size_t barIndex, const std::string& name) override {
    _posContainer->forEachOpenPosition(CloseAtMarketHandler(name, *this), bars, barIndex, PositionEqualLongPredicate());
  }

  void closeAllAtClose(Bars bars, size_t barIndex, const std::string& name) override {
    forEachOpenPosition(CloseAtCloseHandler(name, *this), bars, barIndex);
  }
  void closeAllShortAtClose(Bars bars, size_t barIndex, const std::string& name) override {
    _posContainer->forEachOpenPosition(CloseAtCloseHandler(name, *this), bars, barIndex, PositionEqualShortPredicate());
  }
  void closeAllLongAtClose(Bars bars, size_t barIndex, const std::string& name) override {
    _posContainer->forEachOpenPosition(CloseAtCloseHandler(name, *this), bars, barIndex, PositionEqualLongPredicate());
  }

  void closeAllShortAtLimit(Bars bars, size_t barIndex, double price, const std::string& name) override {
    _posContainer->forEachOpenPosition(CloseAtLimitHandler(name, *this, price), bars, barIndex, PositionEqualShortPredicate());
  }
  void closeAllLongAtLimit(Bars bars, size_t barIndex, double price, const std::string& name) override {
    _posContainer->forEachOpenPosition(CloseAtLimitHandler(name, *this, price), bars, barIndex, PositionEqualLongPredicate());
  }

  void closeAllShortAtStop(Bars bars, size_t barIndex, double price, const std::string& name) override {
    _posContainer->forEachOpenPosition(CloseAtStopHandler(name, *this, price), bars, barIndex, PositionEqualShortPredicate());
  }
  void closeAllLongAtStop(Bars bars, size_t barIndex, double price,const std::string& name) override {
    _posContainer->forEachOpenPosition(CloseAtStopHandler(name, *this, price), bars, barIndex, PositionEqualLongPredicate());
  }

  class CloseFirstAtMarketHandler : public OpenPositionHandler {
    const std::string _name;
    PositionsManagerAbstr& _pm;

   public:
    CloseFirstAtMarketHandler(const std::string& name, PositionsManagerAbstr& pm)
        : _name(name), _pm(pm) {}

    bool onOpenPosition(tradery::Position pos, Bars bars, size_t bar) override {
      if (pos.isLong()) {
        _pm.sellAtMarket(bars, bar, pos, _name);
      }
      else {
        _pm.coverAtMarket(bars, bar, pos, _name);
      }

      return true;
    }
  };

  void closeFirstLongAtMarketByShares(size_t shares, Bars bars, size_t barIndex, const std::string& name) override {
    _posContainer->forEachOpenPosition(CloseAtMarketHandler(name, *this, true), bars, barIndex, PositionLongEqualSharesPredicate(shares));
  }

  void closeFirstShortAtMarketByShares(size_t shares, Bars bars, size_t barIndex, const std::string& name) override {
    _posContainer->forEachOpenPosition(CloseAtMarketHandler(name, *this, true), bars, barIndex, PositionShortEqualSharesPredicate(shares));
  }

  OpenPositionsIterator getOpenPositionsIterator() override {
    return _posContainer->getOpenPositionsIterator();
  }
};
