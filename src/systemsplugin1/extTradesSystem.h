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

/*
IMPORTANT DISCLAIMER

These sample trading systems are included for illustrative purpose only.

Using them with real money can lead to substantial financial losses.

DO NOT USE FOR REAL MONEY TRADING OR INVESTMENT DECISIONS.
*/

#pragma once
#pragma warning(disable : 4482)

#include <fstream>
#include <tokenizer.h>
// format: SYMBOL, LONG/SHORT, DATE, SHARES, PRICE [, DATE, PRICE]

class Trade {
 public:
  enum TradeType { price, market };

 private:
  TradeType _entryType;
  TradeType _exitType;
  std::string _symbol;
  bool _long;
  Date _entryDate;
  unsigned long _shares;

  double _entryPrice;
  Date _exitDate;
  double _exitPrice;
  bool _closed;

 public:
  Trade(const std::string& trade) : _closed(false) {
    //      std::cout << "parsing trade: " << trade << std::endl;
    Tokenizer tokens(trade, ", \t\n");

    assert(tokens.size() == 5 || tokens.size() == 7);

    unsigned int n = 0;
    for (Tokenizer::iterator i = tokens.begin(); i != tokens.end(); i++, n++) {
      std::string str = (*i);
      switch (n) {
        case 0:
          _symbol = to_lower_case(str);
          break;
        case 1:
          _long = to_lower_case(str) == "long";
          break;
        case 2:
          _entryDate = Date(str, us);
          break;
        case 3:
          _shares = atol(str.c_str());
          break;
        case 4:
          if (to_lower_case(str) == "market")
            _entryType = TradeType::market;
          else {
            _entryType = price;
            std::istringstream is(str);
            is >> _entryPrice;
          }
          break;
        case 5:
          _closed = true;
          _exitDate = Date(str, us);
          break;
        case 6:
          if (to_lower_case(str) == "market")
            _exitType = TradeType::market;
          else {
            _exitType = price;
            std::istringstream is(str);
            is >> _exitPrice;
          }
          break;
        default:
          assert(false);
          break;
      }
    }
  }

  const Date& entryDate() const {
    assert(!_entryDate.is_not_a_date());
    return _entryDate;
  }
  const Date& exitDate() const { return _exitDate; }
  bool closed() const {
    //      assert( _closed ^^ !_exitDate.is_not_a_date() );
    return _closed;
  }

  bool isLong() const { return _long; }
  double entryPrice() const { return _entryPrice; }
  unsigned long shares() const { return _shares; }
  double exitPrice() const { return _exitPrice; }
  const std::string& getSymbol() const { return _symbol; }
  TradeType entryType() const { return _entryType; }
  TradeType exitType() const { return _exitType; }
};

class TradeAndPosition : public Trade {
 private:
  PositionId _id;

 public:
  TradeAndPosition(const std::string& trade) : Trade(trade), _id(0) {}

  PositionId getPositionId() const { return _id; }
  void setPositionId(PositionId id) { _id = id; }
};

using TradeAndPositionPtr = std::shared_ptr<TradeAndPosition>;
using DatesToTradesMap = std::multimap<Date, TradeAndPositionPtr>;

class Trades : public DatesToTradesMap {
 public:
  bool hasDate(const Date& date) const {
    DatesToTradesMap::const_iterator i = __super::find(date);
    return i != __super::end();
  }

  void add(const Date& date, TradeAndPositionPtr trade) {
    //      std::cout << "add 70" << std::endl;
    //      std::cout << date.to_iso_string() << std::endl;
    //      std::cout << trade->entryPrice() << std::endl;
    __super::insert(value_type(date, trade));
    //      std::cout << "add 80" << std::endl;
  }
};

using StrToTradesMap = std::map<std::string, Trades>;

class SymbolTradesException {};

using TradesRange = std::pair<Trades::iterator, Trades::iterator>;

class SymbolTrades {
 private:
  StrToTradesMap _entries;
  StrToTradesMap _exits;

 public:
  void add(TradeAndPositionPtr tp) {
    //      std::cout << "add 1" << std::endl;
    const std::string symbol = tp->getSymbol();
    //      std::cout << "add 10" << std::endl;

    StrToTradesMap::iterator i = _entries.find(symbol);
    //      std::cout << "add 20" << std::endl;
    if (i == _entries.end()) {
      i = _entries.insert(StrToTradesMap::value_type(symbol, Trades())).first;
    }
    //      std::cout << "add 30" << std::endl;
    (*i).second.add(tp->entryDate(), tp);
    //      std::cout << "add 40" << std::endl;

    //      std::cout << "add 50" << std::endl;
    if (tp->closed()) {
      //        std::cout << "add 55" << std::endl;
      i = _exits.find(symbol);
      if (i == _exits.end()) {
        i = _exits.insert(StrToTradesMap::value_type(symbol, Trades())).first;
      }
      (*i).second.add(tp->exitDate(), tp);
      //        std::cout << "add 57" << std::endl;
    }
    //      std::cout << "add 60" << std::endl;
  }

  TradesRange getEntries(const std::string& symbol, const Date& date) {
    //      std::cout << "get entries 1: " << symbol << ", " <<
    //      date.to_iso_extended_string() << std::endl;
    StrToTradesMap::iterator i = _entries.find(to_lower_case(symbol));
    //      std::cout << "get entries 5" << std::endl;
    if (i != _entries.end()) {
      //        std::cout << "get entries 10" << std::endl;
      Trades& trades((*i).second);
      Trades::iterator i = trades.find(date);
      //        std::cout << "get entries 15" << std::endl;
      if (i != trades.end())
        return TradesRange(trades.find(date), trades.upper_bound(date));
      else
        return TradesRange(i, i);
    } else {
      // error - can't find trades for this symbol
      //        std::cout << "get entries 30" << std::endl;
      throw SymbolTradesException();
    }
    //      std::cout << "get entries 40" << std::endl;
  }

  std::shared_ptr<TradesRange> getExits(const std::string& symbol,
                                      const Date& date) {
    //      std::cout << "get exits 1: " << symbol << ", " <<
    //      date.to_iso_extended_string() << std::endl;
    StrToTradesMap::iterator i = _exits.find(to_lower_case(symbol));
    //      std::cout << "get exits 2" << std::endl;
    if (i != _exits.end()) {
      //        std::cout << "get exits 3" << std::endl;
      Trades& trades((*i).second);
      //        std::cout << "get exits 4" << std::endl;
      Trades::iterator i = trades.find(date);
      //        std::cout << "get exits 5" << std::endl;
      if (i != trades.end())
        return std::make_shared<TradesRange>(
           trades.find(date), trades.upper_bound(date));
      else
        return std::make_shared<TradesRange>(i, i);
    } else {
      // the position is still opened, continue
      return std::shared_ptr<TradesRange>();
    }
    //      std::cout << "get exits 7" << std::endl;
  }
};

class ExternalTrades {
 private:
  SymbolTrades _st;

 public:
  ExternalTrades(const std::string& fileName) {
    std::cout << "ExternalTrdes constructor: " << fileName << std::endl;
    std::ifstream extFile(fileName.c_str());

    if (extFile) {
      do {
        std::string str;
        std::getline(extFile, str);

        if (str.length() > 0 && str[0] == TCHAR('#')) continue;

        //          LOG( log_debug, "line: " << str << ", length: " <<
        //          str.length() << ", [0]: " << str[ 0 ] << ", [1]: " << str[ 1
        //          ] << std::endl;
        if (str.length() > 1 && str[0] == TCHAR('/') && str[1] == TCHAR('/')) {
          std::cout << "comment line" << std::endl;
          continue;
        }

        Tokenizer tokens(str, ", \t\n");
        if (tokens.size() == 0) continue;
        TradeAndPositionPtr trade(std::make_shared< TradeAndPosition >(str) );
        _st.add(trade);
      } while (!extFile.eof());
    }
  }

  TradesRange getEntries(const std::string& symbol, const Date& date) {
    //      std::cout << "get entries: " << symbol << ", " <<
    //      date.to_iso_extended_string() << std::endl;
    return _st.getEntries(symbol, date);
  }

  std::shared_ptr<TradesRange> getExits(const std::string& symbol,
                                      const Date& date) {
    //      std::cout << "get exits: " << symbol << ", " <<
    //      date.to_iso_extended_string() << std::endl;
    return _st.getExits(symbol, date);
  }
};

class ExtTradesSystem : public BarSystem<ExtTradesSystem> {
  const std::vector<std::string>* _params;
  std::shared_ptr<ExternalTrades> _trades;

 public:
  ExtTradesSystem(const std::vector<std::string>* params = 0)
      // set the system name and the holding period to 2 days, profit target to
      // 5%
      : BarSystem<ExtTradesSystem>(Info("EB5823F3-B38E-4a05-A4C0-425E9517BBCF",
                                        "Backtesting external trades", "")),
        _params(params) {
    // params can be 0 the first time the plugin is instatiated by the plugin
    // loader, when it looks for all available plugins to build the tree
    std::cout << "ExtTradesSystem constructor, params: " << params << std::endl;
    if (params != 0) _trades = std::make_shared< ExternalTrades >((*params)[0]);
  }

 private:
  bool init(const String& symbol) { return true; }

  void run() { FOR_EACH_BAR(0); }

  void onBar(Index bar) {
    //    PRINT_LINE( "1" );
    // get the current time stamp
    Date date(DEF_BARS.time(bar).date());

    //    PRINT_LINE( "2" );
    TradesRange entries(_trades->getEntries(__super::getSymbol(), date));
    //    PRINT_LINE( "count entries: " << entries.second - entries.first );
    //    PRINT_LINE( "25" );

    for (Trades::iterator i = entries.first; i != entries.second; i++) {
      //      PRINT_LINE( "3" );

      TradeAndPosition* p = (*i).second.get();
      //      PRINT_LINE( "Pointer: " << (long)p << " long: " << p ->isLong() );

      TradeAndPosition& tp(*(*i).second);

      //      PRINT_LINE( "4" );
      if (tp.isLong()) {
        //        PRINT_LINE( "entry price: " << tp.entryPrice() );

        PositionId id;

        if (tp.entryType() == Trade::TradeType::price) {
          if (tp.entryPrice() <= open(bar))
            id = BUY_AT_LIMIT(bar, tp.entryPrice(), tp.shares(), "");
          else
            id = BUY_AT_STOP(bar, tp.entryPrice(), tp.shares(), "");
          //        PRINT_LINE( "11" );
        } else if (tp.entryType() == Trade::TradeType::market)
          id = BUY_AT_MARKET(bar, tp.shares(), "");

        if (id != 0)
          tp.setPositionId(id);
        else {
          systemError(tradery::format( "Cannot open long position - entry price ", tp.entryPrice() , " may be out of price range for bar ", date.to_iso_extended_string()));
        }
      } else {
        //        PRINT_LINE( "20" );
        //        std::cout << "short at limit: " << tp.entryPrice() <<
        //        std::endl;
        PositionId id;
        //        PRINT_LINE( "low: " << low( bar ) << ", high: " << high( bar )
        //        );

        if (tp.entryType() == Trade::TradeType::price) {
          if (tp.entryPrice() >= open(bar))
            id = SHORT_AT_LIMIT(bar, tp.entryPrice(), tp.shares(), "");
          else
            id = SHORT_AT_STOP(bar, tp.entryPrice(), tp.shares(), "");
        } else if (tp.entryType() == Trade::TradeType::market)
          id = SHORT_AT_MARKET(bar, tp.shares(), "");

        if (id != 0)
          tp.setPositionId(id);
        else {
          systemError(tradery::format( "Cannot open short position - entry price ", tp.entryPrice(), " may be out of price range for bar ", date.to_iso_extended_string()));
        }

        //        PRINT_LINE( "22" );
      }
      //      PRINT_LINE( "5" );
    }

    //    PRINT_LINE( "6" );

    std::shared_ptr<TradesRange> x(_trades->getExits(__super::getSymbol(), date));
    if (x.get() != 0) {
      TradesRange& exits(*x);
      for (Trades::iterator i = exits.first; i != exits.second; i++) {
        //        PRINT_LINE( "7" );
        TradeAndPosition& tp(*(*i).second);

        PositionId id = tp.getPositionId();

        tradery::Position pos = DEF_POSITIONS.getPosition(id);
        if (!pos) {
          // todo: handle this error
        }
        if (tp.isLong()) {
          bool b;
          if (tp.entryType() == Trade::TradeType::price) {
            if (tp.entryPrice() >= open(bar))
              b = SELL_AT_LIMIT(bar, pos, tp.exitPrice(), "");
            else
              b = SELL_AT_STOP(bar, pos, tp.exitPrice(), "");
          } else if (tp.entryType() == Trade::TradeType::market)
            b = SELL_AT_MARKET(bar, pos, "");

          if (!b) {
            systemError(tradery::format("Cannot close long position - exit price ", tp.exitPrice(), " may be out of price range for bar ", date.to_iso_extended_string()));
          }
        } else {
          bool b;
          if (tp.entryType() == Trade::TradeType::price) {
            if (tp.entryPrice() <= open(bar))
              b = COVER_AT_LIMIT(bar, pos, tp.exitPrice(), "");
            else
              b = COVER_AT_STOP(bar, pos, tp.exitPrice(), "");
          } else if (tp.entryType() == Trade::TradeType::market)
            b = COVER_AT_MARKET(bar, pos, "");

          if (!b) {
            systemError(tradery::format( "Cannot close short position - exit price ", tp.exitPrice(), " may be out of price range for bar ", date.to_iso_extended_string()));
          }
        }
      }
    }
  }

  bool onOpenPosition(tradery::Position pos, Index bar) { return true; }

  void cleanup() {}
};
