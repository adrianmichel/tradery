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
#include <fstream>
#include <explicittrades.h>
#include <tokenizer.h>
#include <log.h>

class StringExplicitTrade : public ExplicitTrade {
 public:
  StringExplicitTrade(const std::vector<std::string>& tokens) try
      : ExplicitTrade(trim(tokens[0]), Date(trim(tokens[1])),toAction(boost::trim_copy(tokens[2])), toType(trim(tokens[3])),
            tokens.size() >= 5 ? atol(boost::trim_copy(tokens[4]).c_str()) : 0, tokens.size() == 6 ? atof(boost::trim_copy(tokens[5]).c_str()) : 0) {
    assert(tokens.size() <= 6 && tokens.size() >= 4);
  }
  catch (const DateException&) {
    throw ExplicitTradeException("Wrong date format: "s + tokens[1]);
  }
};

class JsonExplicitTrade : public ExplicitTrade {
 public:
  JsonExplicitTrade(const nlohmann::json json) try
      : ExplicitTrade(json.at("symbol").get<std::string>(), Date(json["time"].get<std::string>()), toAction(json["action"].get<std::string>()),
                      toType(json["type"].get<std::string>()), json["shares"].get<unsigned int>(), json.find("price ") == json.end() ? 0 : json["price"].get<double>(),
                      json.find("id") == json.end() ? 0 : json["id"].get<unsigned int>()) {
  } 
  catch (...) {
    assert(false);
  }
};

void ExplicitTrade::processExit(Index barIndex, Positions pos, Bars bars) const {
  switch (action()) {
    case BUY:
    case SELL_SHORT:
      break;

    case SELL: {
      switch (type()) {
        case CLOSE:
          break;
        case MARKET:
          pos.closeFirstLongAtMarketByShares(shares(), bars, barIndex, "Explicit trade close first long position by shares at market");
          break;
        case LIMIT:
          break;
        case STOP:
          break;
        case PRICE:
          // handle
          break;
        default:
          break;
      }
    } break;
    case COVER: {
      switch (type()) {
        case CLOSE:
          break;
        case MARKET:
          pos.closeFirstShortAtMarketByShares(shares(), bars, barIndex, "Explicit trade close first short position by shares at market");
          break;
        case LIMIT:
          break;
        case STOP:
          break;
        case PRICE:
          // handle
          break;
        default:
          break;
      }
    } break;
    case SELL_ALL: {
      switch (type()) {
        case CLOSE:
          pos.closeAllLongAtClose(bars, barIndex, "Explicit trade close all long at close");
          break;
        case MARKET:
          pos.closeAllLongAtMarket(bars, barIndex, "Explicit trade close all long at market");
          break;
        case LIMIT:
          pos.closeAllLongAtLimit(bars, barIndex, price(), "Explicit trade close all long at limit");
          break;
        case STOP:
          pos.closeAllLongAtStop(bars, barIndex, price(), "Explicit trade close all long at stop");
          break;
        case PRICE:
          // handle
          break;
        default:
          break;
      }
    } break;
    case COVER_ALL: {
      switch (type()) {
        case CLOSE:
          pos.closeAllShortAtClose(bars, barIndex, "Explicit trade close all short at close");
          break;
        case MARKET:
          pos.closeAllShortAtMarket(bars, barIndex, "Explicit trade close all short at market");
          break;
        case LIMIT:
          pos.closeAllShortAtLimit(bars, barIndex, price(), "Explicit trade close all short at limit");
          break;
        case STOP:
          pos.closeAllShortAtStop(bars, barIndex, price(), "Explicit trade close all short at stop");
          break;
        case PRICE:
          // handle
          break;
        default:
          break;
      }
    } break;
    case EXIT_ALL: {
      switch (type()) {
        case CLOSE:
          pos.closeAllAtClose(bars, barIndex, "Explicit trade close all at close");
          break;
        case MARKET:
          pos.closeAllAtMarket(bars, barIndex, "Explicit trade close all at market");
          break;
        case LIMIT:
          // error handle
          break;
        case STOP:
          // handle error
          break;
        case PRICE:
          // handle error
        default:
          break;
      }
    } break;
    default:
      assert(false);
      break;
  }
}

void ExplicitTrade::processEntry(Index barIndex, Positions pos, Bars bars) const {
  switch (action()) {
    case SELL:
    case COVER:
    case SELL_ALL:
    case COVER_ALL:
    case EXIT_ALL:
      break;

    case BUY:
      switch (type()) {
        case CLOSE:
          pos.buyAtClose(bars, barIndex, shares(), "Explicit trade buy at close", false);
          break;
        case MARKET:
          pos.buyAtMarket(bars, barIndex, shares(), "Explicit trade buy at market", false);
          break;
        case LIMIT:
          pos.buyAtLimit(bars, barIndex, price(), shares(), "Explicit trade buy at limit", false);
          break;
        case STOP:
          pos.buyAtStop(bars, barIndex, price(), shares(), "Explicit trade buy at stop", false);
          break;
        case PRICE:
          pos.buyAtPrice(bars, barIndex, price(), shares(), "Explicit trade buy at stop", false, id());
          break;
        default:
          assert(false);
          break;
      }

      break;
    case SELL_SHORT:
      switch (type()) {
        case CLOSE:
          pos.shortAtClose(bars, barIndex, shares(), "Explicit trade buy at close", false);
          break;
        case MARKET:
          pos.shortAtMarket(bars, barIndex, shares(), "Explicit trade buy at market", false);
          break;
        case LIMIT:
          pos.shortAtLimit(bars, barIndex, price(), shares(), "Explicit trade buy at limit", false);
          break;
        case STOP:
          pos.shortAtStop(bars, barIndex, price(), shares(), "Explicit trade buy at stop", false);
          break;
        case PRICE:
          pos.shortAtPrice(bars, barIndex, price(), shares(), "Explicit trade buy at stop", false, id());
          break;
        default:
          assert(false);
          break;
      }
      break;
    default:
      assert(false);
      break;
  }
}

void FileExplicitTrades::processCSVFormat(const std::string& line,
                                          unsigned int lineCt) {
  Tokenizer tokens(line, ",");
  LOG(log_info, "FileExplicitTrades::FileExplicitTrades line: ", line);

  if (tokens.size() > 0) {
    if (tokens.size() < 4 || tokens.size() > 6) {
      // wrong number of elements
      throw ExplicitTradesException( tradery::format( "Explicit Trade format error on line ", lineCt, ": \"", line, "\""));
    }

    try {
      LOG(log_debug, "creating StringExplicitTrade");
      ExplicitTradeConstPtr p(std::make_shared< StringExplicitTrade>(tokens));
      LOG(log_debug, "after creating StringExplicitTrade");
      __super::add(p);
    }
    catch (const ExplicitTradeException& e) {
      throw ExplicitTradesException( tradery::format( "Explicit trade error on line ", lineCt, ": ", e.message() ));
    }
  }
}

void FileExplicitTrades::processJSONFormat(const std::string& line, unsigned int lineCt) {}

CORE_API FileExplicitTrades::FileExplicitTrades(const std::string& fileName)
    : _format(csv)  // by default use the csv format
{
  LOG(log_debug, "FileExplicitTrades::FileExplicitTrades: ", fileName);
  if (!fileName.empty()) {
    std::ifstream extFile(fileName.c_str());

    unsigned int lineCt = 0;
    while (extFile) {
      std::string line;

      std::getline(extFile, line);
      boost::trim(line);
      lineCt++;

      preprocess(line);

      if (ignore(line)) continue;

      switch (_format) {
        case csv:
          processCSVFormat(line, lineCt);
          break;
        case json:
          processJSONFormat(line, lineCt);
          break;
        default:
          break;
      }
    }
    LOG(log_debug, "exting constructor");
  }
}

Action ExplicitTrade::toAction(const std::string& action) {
  const std::string t(to_lower_case(action));
  if (t == "buy") {
    return BUY;
  }
  else if (t == "sell") {
    return SELL;
  }
  else if (t == "short" || t == "sellshort" || t == "sell_short") {
    return SELL_SHORT;
  }
  else if (t == "cover") {
    return COVER;
  }
  else if (t == "sell_all" || t == "sellall" || t == "exitalllong" ||
    t == "exit_all_long" || t == "closealllong" || t == "close_all_long") {
    return SELL_ALL;
  }
  else if (t == "cover_all" || t == "coverall" || t == "exitallshort" ||
    t == "exit_all_short" || t == "closeallshort" || t == "close_all_short") {
    return COVER_ALL;
  }
  else if (t == "exit_all" || t == "exitall" || t == "close_all" || t == "closeall") {
    return EXIT_ALL;
  }
  else {
    throw ExplicitTradeException("Invalid trigger direction: "s + action);
  }
}

Type ExplicitTrade::toType(const std::string& type) {
  const std::string t(to_lower_case(type));
  if (t == "market")
    return MARKET;
  else if (t == "close")
    return CLOSE;
  else if (t == "limit")
    return LIMIT;
  else if (t == "stop")
    return STOP;
  else if (t == "price")
    return PRICE;
  else {
    throw ExplicitTradeException("Invalid trigger type: "s + type);
  }
}

std::string typeToString(Type type) {
  switch (type) {
    case MARKET:
      return "market";
    case CLOSE:
      return "close";
    case LIMIT:
      return "limit";
    case STOP:
      return "stop";
    case PRICE:
      return "price";
    default:
      assert(false);
      return "unknown";
  }
}

std::string actionToString(Action action) {
  switch (action) {
    case BUY:
      return "buy";
    case SELL:
      return "sell";
    case SELL_SHORT:
      return "sell short";
    case COVER:
      return "cover";
    case SELL_ALL:
      return "sell all";
    case COVER_ALL:
      return "cover all";
    case EXIT_ALL:
      return "exit all";
    default:
      assert(false);
      return "unknown";
  }
}

std::string ExplicitTrade::toString() const {
  std::string str;

  return "symbol: "s + symbol() + ", time: " + time().to_simple_string()
      + ", type: " + typeToString(type()) + ", shares: " + std::to_string( shares() )
      + ", price: " + std::to_string( price() ) + ", action:" + actionToString(action());

  return str;
}

#define COMMENT_STYLE_1(str) \
  (str.length() > 1 && str[0] == '/' && str[2] == '/')
#define COMMENT_STYLE_2(str) (str.length() > 0 && str[0] == '#')

bool FileExplicitTrades::isComment(const std::string& str) {
  if (str.empty()) {
    return false;
  }
  else if (COMMENT_STYLE_1(str) || COMMENT_STYLE_2(str)) {
    return true;
  }
  else {
    return false;
  }
}

bool FileExplicitTrades::ignore(const std::string& str) {
  return str.empty() || isComment(str);
}

std::string FileExplicitTrades::getComment(const std::string& str) {
  if (!isComment(str)) {
    return "";
  }
  else if (COMMENT_STYLE_2(str)) {
    return str.length() > 1 ? str.substr(1, str.length() - 1) : "";
  }
  else if (COMMENT_STYLE_1(str)) {
    return str.length() > 2 ? str.substr(2, str.length() - 2) : "";
  }
  else {
    return "";
  }
}

// processes directives
// format: !name=value
void FileExplicitTrades::preprocess(const std::string& str) {
  std::string comment(trim(getComment(str)));

  // a directive comment must be at least 4 chars long: !x=y
  if (comment.length() >= 4 && comment[0] == '!') {
    // this is a directive

    Tokenizer tokens(comment.substr(1, comment.length() - 1), "=");

    if (tokens.size() == 2) {
      const std::string name(to_lower_case(trim(tokens[0])));
      const std::string value(trim(tokens[1]));

      if (name == "csv") {
        _format = csv;
      }
      else if (name == "json") {
        _format = json;
      }
    }
  }
}
