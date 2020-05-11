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

using namespace tradery;

//#define STRING std::wstring
using String = std::string;
using Index = size_t;

/**
 * default positions
 *
 * each system generates positions (trades) using...
 */
#define DEF_POSITIONS __super::positions()
/**
 * default bar data
 *
 * each system receives bar data for the default symbol ...
 */
#define DEF_BARS __super::bars()
#define DEF_SYMBOL __super::defSymbol()
//#define BARS const Bars*

#define CLOSE_SERIES DEF_BARS.closeSeries()
#define OPEN_SERIES DEF_BARS.openSeries()
#define HIGH_SERIES DEF_BARS.highSeries()
#define LOW_SERIES DEF_BARS.lowSeries()
#define VOLUME_SERIES DEF_BARS.volumeSeries()
#define OPEN_INTEREST_SERIES DEF_BARS.openInterestSeries()
#define TIME_SERIES DEF_BARS.timeSeries();

#define FOR_EACH_OPEN_POSITION(bar) DEF_POSITIONS.forEachOpenPosition(*this, DEF_BARS, bar)
#define FOR_EACH_BAR(val) forEachBar(*this, val)

#define BUY_AT_MARKET(bar, shares, name) DEF_POSITIONS.buyAtMarket(DEF_BARS, bar, shares, name)
#define BUY_AT_CLOSE(bar, shares, name) DEF_POSITIONS.buyAtClose(DEF_BARS, bar, shares, name)
#define BUY_AT_LIMIT(bar, price, shares, name) DEF_POSITIONS.buyAtLimit(DEF_BARS, bar, price, shares, name)
#define BUY_AT_STOP(bar, price, shares, name) DEF_POSITIONS.buyAtStop(DEF_BARS, bar, price, shares, name)

#define SELL_AT_MARKET(bar, pos, name) DEF_POSITIONS.sellAtMarket(DEF_BARS, bar, pos, name)
#define SELL_AT_CLOSE(bar, pos, name) DEF_POSITIONS.sellAtClose(DEF_BARS, bar, pos, name)
#define SELL_AT_LIMIT(bar, pos, price, name) DEF_POSITIONS.sellAtLimit(DEF_BARS, bar, pos, price, name)
#define SELL_AT_STOP(bar, pos, price, name) DEF_POSITIONS.sellAtStop(DEF_BARS, bar, pos, price, name)

#define SHORT_AT_MARKET(bar, shares, name) DEF_POSITIONS.shortAtMarket(DEF_BARS, bar, shares, name)
#define SHORT_AT_CLOSE(bar, shares, name) DEF_POSITIONS.shortAtClose(DEF_BARS, bar, shares, name)
#define SHORT_AT_LIMIT(bar, price, shares, name) DEF_POSITIONS.shortAtLimit(DEF_BARS, bar, price, shares, name)
#define SHORT_AT_STOP(bar, price, shares, name) DEF_POSITIONS.shortAtStop(DEF_BARS, bar, price, shares, name)

#define COVER_AT_MARKET(bar, pos, name) DEF_POSITIONS.coverAtMarket(DEF_BARS, bar, pos, name)
#define COVER_AT_CLOSE(bar, pos, name) DEF_POSITIONS.coverAtClose(DEF_BARS, bar, pos, name)
#define COVER_AT_LIMIT(bar, pos, price, name) DEF_POSITIONS.coverAtLimit(DEF_BARS, bar, pos, price, name)
#define COVER_AT_STOP(bar, pos, price, name) DEF_POSITIONS.coverAtStop(DEF_BARS, bar, pos, price, name)

#define CLOSE_ALL_AT_MARKET(bar, name) DEF_POSITIONS.closeAllAtMarket(DEF_BARS, bar, name)
#define CLOSE_ALL_SHORT_AT_MARKET(bar, name) DEF_POSITIONS.closeAllShortAtMarket(DEF_BARS, bar, name)
#define CLOSE_ALL_LONG_AT_MARKET(bar, name) DEF_POSITIONS.closeAllLongAtMarket(DEF_BARS, bar, name)

#define CLOSE_ALL_AT_CLOSE(bar, name) DEF_POSITIONS.closeAllAtClose(DEF_BARS, bar, name)
#define CLOSE_ALL_SHORT_AT_CLOSE(bar, name) DEF_POSITIONS.closeAllShortAtClose(DEF_BARS, bar, name)
#define CLOSE_ALL_LONG_AT_CLOSE(bar, name) DEF_POSITIONS.closeAllLongAtClose(DEF_BARS, bar, name)

#define CLOSE_ALL_SHORT_AT_LIMIT(bar, price, name) DEF_POSITIONS.closeAllShortAtLimit(DEF_BARS, bar, price, name)
#define CLOSE_ALL_LONG_AT_LIMIT(bar, price, name) DEF_POSITIONS.closeAllLongAtLimit(DEF_BARS, bar, price, name)

#define CLOSE_ALL_SHORT_AT_STOP(bar, price, name) DEF_POSITIONS.closeAllShortAtStop(DEF_BARS, bar, price, name)
#define CLOSE_ALL_LONG_AT_STOP(bar, price, name) DEF_POSITIONS.closeAllLongAtStop(DEF_BARS, bar, price, name)

#define POSITION_ID PositionId

#define INSTALL_PROFIT_TARGET(target) DEF_POSITIONS.installProfitTarget(target)
#define INSTALL_TIME_BASED_EXIT(bars) DEF_POSITIONS.installTimeBasedExit(bars)
#define INSTALL_BREAK_EVEN_STOP(level) DEF_POSITIONS.installBreakEvenStop(level)
#define INSTALL_REVERSE_BREAK_EVEN_STOP(level) DEF_POSITIONS.installReverseBreakEvenStop(level)
#define INSTALL_STOP_LOSS(level) DEF_POSITIONS.installStopLoss(level)
#define INSTALL_TRAILING_STOP(trigger, level) DEF_POSITIONS.installTrailingStop(trigger, level)

#define APPLY_PROFIT_TARGET(bar, pos) DEF_POSITIONS.applyProfitTarget(DEF_BARS, bar, pos)
#define APPLY_TIME_BASED_EXIT(bar, pos) DEF_POSITIONS.applyTimeBased(DEF_BARS, bar, pos)
#define APPLY_BREAK_EVEN_STOP(bar, pos) DEF_POSITIONS.installBreakEvenStop(DEF_BARS, bar, pos)
#define APPLY_REVERSE_BREAK_EVEN_STOP(bar, pos) DEF_POSITIONS.installReverseBreakEvenStop(DEF_BARS, bar, pos)
#define APPLY_STOP_LOSS(bar, pos) DEF_POSITIONS.installStopLoss(DEF_BARS, bar, pos)
#define APPLY_TRAILIING_STOP(bar, pos) DEF_POSITIONS.installTrailingStop(DEF_BARS, bar, pos)

#define APPLY_AUTO_STOPS(bar) DEF_POSITIONS.applyAutoStops(DEF_BARS, bar)
#define APPLY_AUTO_STOPS_POS_DEF(bar, pos) DEF_POSITIONS, applyAutoStops(DEF_BARS, bar, pos)

// using the BarSystem::_os static variable which is visible in derived system
// classes and their inner classes
#define PRINT_LINE(line)          \
  {                               \
    OutputBuffer ob;              \
    assert(_os != 0);             \
    (*_os).printLine(ob << line); \
  }
//#define PrintLine( line ) { OutputBuffer ob; assert( _os != 0 );
//(*_os).printLine( ob <<line ); }
#define PrintLine(line) printLine(_os, OutputBuffer() << line)
#define PRINT(line)                 \
  {                                 \
    OutputBuffer ob;                \
    outputSink().print(ob << line); \
  }

/*#define SYMBOLS_ITERATOR (*symbolsIterator())
#define SYMBOL std::wstring
#define RESET_SYMBOLS SYMBOLS_ITERATOR.reset()
#define GET_NEXT_SYMBOL ((SYMBOL)*(SYMBOLS_ITERATOR.getNext() ) )
#define HAS_MORE_SYMBOLS ( SYMBOLS_ITERATOR.hasMore() )
#define GET_FIRST_SYMBOL ( (SYMBOL)*(SYMBOLS_ITERATOR.getFirst() ) )
*/

#define OPEN_POSITIONS_COUNT DEF_POSITIONS.openPositionsCount()
#define HAS_OPEN_POSITIONS (OPEN_POSITIONS_COUNT > 0)

#define AND &&
#define OR ||
#define EQUALS ==
#define DIFFERENT !=
#define NOT !

#define TRIGGERS const tradery::Triggers&

#define ARRAY(value) std::vector<value>
#define MAP(key, value) std::map<key, value>

// obsolete declarations:
using STRING = std::string;
using INDEX = size_t;
