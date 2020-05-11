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

/**
 * SystemTest0 illustrates some of the techniques used when writing a trading
 * system
 *
 * It is not supposed to be a working trading system, meaning it will most
 * likely not be profitable in real trading
 *
 * It is derived from
 * - BarSystem< SystemTest >, which makes it a test system that will create
 * positions
 * - OpenPOsotionHandler, which makes it a position onBar - will be called when
 * looping methods such as forEachOpenPosition are called
 * - BarHandler, which makes it a bar onBar - will be called when looping
 * methods such as forEachBar are called
 * - OrderFilter, which makes it an order filter too - will receive calls before
 * orders are executed
 *
 * @see BarSystem< SystemTest >
 * @see OpenPositionsHandler
 * @see BarHandler
 * @see OrderFilter
 */
class SystemTest0 : public BarSystem<SystemTest0> {
 private:
  // time based holding period
  unsigned int _days;
  // profit target
  double _profitTarget;
  // this is a custom built series, so we need to take care of this (will delete
  // every time on cleanup, but it's still in auto_ptr, in case of an exception
  // thrown somewhere in the system
  Series series;
  // the Series manager will take care of this (don't need to delete)
  Series sma;

 private:
  // this is the OpenPositionHandle method, which would be called on each open
  // position it is not actually called in this case, as forEachOpenPosition is
  // not used
  virtual bool onOpenPosition(tradery::Position pos, INDEX bar) {
    // do something here with the position
    return true;
  }

 public:
  SystemTest0(const std::vector<std::string>* params = 0)
      // set the system name and the holding period to 2 days, profit target to
      // 5%
      : BarSystem<SystemTest0>(
            Info("9A780135-48AB-4255-95E2-91B32A2258FC", "Test 0", "Test 0")),
        _days(2),
        _profitTarget(5) {}

  ~SystemTest0() { int n = 0; }

  // this is called by the framework before running the system.
  // used to do one time initializations
  virtual bool init(const std::string& symbol) {
    // registers the system itself as an order filter with the default positions
    // an order filter could have been implemented as a separate class,
    // which would have allowed it to be used by other systems,
    // but it works this way too
    // onBuyAtLimit will be called for every buyAtLimit call
    positions().registerOrderFilter(this);
    // calculates the 10 bars SMA series
    sma = VOLUME_SERIES.SMA(10);

    // create two temporary series
    Series tr(bars().size());
    Series atr(bars().size());

    // these true range series are not useful in this case,
    // they are just calculated to show it can be done, and how
    // calculate the true range series values
    unsigned int bar = 1;
    for (; bar < bars().size(); bar++) {
      double h = high(bar);
      double l = low(bar);
      double c = close(bar);
      double pc = close(bar - 1);

      tr[bar] = max3(h - l, pc - h < 0 ? h - pc : pc - h,
                     pc - l < 0 ? l - pc : pc - l) /
                c;
    }

    // create a new series trsma - 3 bars SMA of the true range
    Series trsma = tr.SMA(3);

    // calculates the average true range
    atr[3] = trsma[3];
    for (bar = 3; bar < bars().size(); bar++) {
      double x = tr[bar];
      double prev = atr[bar - 1];
      atr[bar] = (prev * (3 - 1) + x) / 3;
    }

    // get the EMA of the close
    // the series of limit prices will be the close_ema * 0.95
    Series close_ema = closeSeries().EMA(4);

    // this is the series of limit prices (95% of the 4 bars ema of close).
    series = close_ema * 0.95;

    // install some exit condtions:
    // 5% profit target
    INSTALL_PROFIT_TARGET(_profitTarget);
    INSTALL_TIME_BASED_EXIT(_days);

    // signal that init was performed succesfully
    // a false return would signal some error and would stop the run
    return true;
  }

  // this is the buy at limit onBar - it is called before buy at limit is
  // submitted and gives a last chance to change it or even not send it at all
  unsigned int onBuyAtLimit(size_t bar, unsigned int shares,
                            double price) const {
    // in this case we are just doing some money maanagemt as well as some
    // filtering based on previous close prices

    // if it closed lower the previous two bars
    // and the total value traded was higher than $1.2mil
    // and the price of the order is lower than the last close
    // then buy at limit the number of shares calculated by getShares

    // IMPORTANT: since buyAtLimit is called on bar + 1 (see below), we have to
    // look at the last bar before that, so we use bar - 1, otherwise this will
    // trigger bar out of range error for the last bar available so instead bar,
    // we use bar - 1 (which is the last bar before the bar on which the buy
    // would happen) etc.
    if ((close(bar - 1) < close(bar - 2)) &&
        (close(bar - 2) < close(bar - 3)) && (price * sma[bar - 1] > 1200000) &&
        (price < close(bar - 1)))
      return getShares(bar - 1, price);
    else
      // otherwise don't place the order
      return 0;
  }

  // this implements the money management - calculates the number of shares
  // in this case, the position value should alwyas be $5000
  unsigned int getShares(size_t bar, double price) const {
    return (unsigned int)(5000 / price);
  }

  // called for each bar by forEachBar
  virtual void onBar(INDEX bar) {
    // apply auto stops - so check for profit target and holding period in our
    // case
    APPLY_AUTO_STOPS(bar);
    // get the limit price for the current bar
    double buyLimitPrice = series[bar];

    // buy at limit on the next bar (it should always be next bar or the system
    // would be peeking into the future!) use a default value of 1000 shares -
    // it will be re-calculated by onBuyAtLimit
    BUY_AT_LIMIT(bar + 1, buyLimitPrice, 1000, "buy at limit");
  }

  // the actual run method
  // it just does forEachBar which in turn calls onBar for all the available
  // bars
  virtual void run() { FOR_EACH_BAR(10); }
};

// WL code test1
/**
var t : integer;
t := getTickCount;
var Bar: integer;

for Bar := 0 to BarCount - 1 do
begin
var pos : integer;

for pos := 0 to positioncount - 1 do
if positionactive( pos ) then
sellAtLimit( bar, 25, pos, 'sell at limit' );

buyAtLimit( bar, PriceLow( bar ) * 0.95, 'buy at limit' );
end;
print( 'duration: ' + IntToStr( getTickCount - t ) );
*/
class SystemTest1 : public BarSystem<SystemTest1> {
 private:
  virtual bool onOpenPosition(tradery::Position pos, INDEX bar) {
    SELL_AT_LIMIT(bar + 1, pos, 25, "sell at limit");
    return true;
  }

 public:
  SystemTest1(const std::vector<std::string>* params = 0)
      : BarSystem<SystemTest1>(
            Info("C9D2B359-8DB4-4587-8D1D-B26B275ACEC5", "Test 1", "")) {}

  virtual void onBar(INDEX bar) {
    FOR_EACH_OPEN_POSITION(bar);
    BUY_AT_LIMIT(bar + 1, low(bar) * 1.05, 1000, "buy at limit");
  }

  virtual void run() { FOR_EACH_BAR(0); }
};

/** WL code Test2
var Bar: integer;
var t : integer;

t := getTickCount;

for Bar := 0 to BarCount - 1 do
begin
var pos : integer;

for pos := 0 to positioncount - 1 do
if positionactive( pos ) then
CoverAtLimit( bar, 25, pos, 'sell at limit' );

ShortAtLimit( bar, 2.97, 'buy at limit' );
end;

Print( IntToStr( GetTickCount - t ) );
*/
class SystemTest2 : public BarSystem<SystemTest2> {
 private:
  virtual bool onOpenPosition(tradery::Position pos, INDEX bar) {
    COVER_AT_LIMIT(bar + 1, pos, 25, "short exit");

    return true;
  }

 public:
  SystemTest2(const std::vector<std::string>* params = 0)
      : BarSystem<SystemTest2>(
            Info("115CEFEE-6C8B-4a02-8B7F-14FBBB617550", "Test 2", "")) {}

  void onBar(INDEX bar) {
    FOR_EACH_OPEN_POSITION(bar);
    SHORT_AT_LIMIT(bar + 1, high(bar) * 1.05, 1000, "short entry");
  }
  virtual void run() { FOR_EACH_BAR(0); }
};

/** WL code test3
var Bar: integer;

var t : integer;
t := getTickCount;

for Bar := 0 to BarCount - 1 do
begin
var pos : integer;

for pos := 0 to positioncount - 1 do
if positionactive( pos ) then
coverAtStop( bar, 25, pos, 'cover at stop' );

shortAtLimit( bar + 1, 2.97, 'buy at limit' );
end;

print( 'duration: ' + IntToStr( getTickCount - t ) );
*/
class SystemTest3 : public BarSystem<SystemTest3> {
 private:
  virtual bool onOpenPosition(tradery::Position pos, INDEX bar) {
    COVER_AT_STOP(bar + 1, pos, 25, "short exit");

    return true;
  }

 public:
  SystemTest3(const std::vector<std::string>* params = 0)
      : BarSystem<SystemTest3>(
            Info("50487369-8348-4094-8B99-B2517528F215", "Test 3", "Test 3")) {}

  void onBar(INDEX bar) {
    FOR_EACH_OPEN_POSITION(bar);
    SHORT_AT_LIMIT(bar + 1, high(bar) * 1.05, 1000, "short entry");
  }

  void run() { FOR_EACH_BAR(0); }
};

/** WL code test4
var t : integer;
t := getTickCount;
var Bar: integer;
InstallTimeBasedExit( 2 );
for Bar := 0 to BarCount - 1 do
begin
ApplyAutoStops( bar );
ShortAtLimit( bar + 1, PriceHigh * 1.05 'buy at limit' );
end;
print( 'duration: ' + IntToStr( getTickCount - t ) );
*/
class SystemTest4 : public BarSystem<SystemTest4> {
 public:
  SystemTest4(const std::vector<std::string>* params = 0)
      : BarSystem<SystemTest4>(
            Info("584D51A6-5E30-4038-9717-9EC71FF3E48F", "Test 4", "")) {}

  void run() {
    INSTALL_BREAK_EVEN_STOP(1);
    FOR_EACH_BAR(0);
  }

  void onBar(INDEX bar) {
    APPLY_AUTO_STOPS(bar);
    SHORT_AT_MARKET(bar + 1, 1000, "short entry");
  }
};

/** WL code test5
var t : integer;
t := getTickCount;
var Bar : integer;

InstallReverseBreakEvenStop ( 1 );

for Bar := 0 to barcount - 1 do
begin
ApplyAutoStops( Bar );
ShortAtLimit( Bar + 1, PriceHigh( Bar ), 'buy at limit' );
end;
print( 'duration: ' + IntToStr( getTickCount - t ) );
*/
class SystemTest5 : public BarSystem<SystemTest5> {
 public:
  SystemTest5(const std::vector<std::string>* params = 0)
      : BarSystem<SystemTest5>(
            Info("AD1231D9-3774-4ac3-86E6-587A5A2B6170", "Test 5", "")) {}

  virtual void onBar(INDEX bar) {
    APPLY_AUTO_STOPS(bar);
    SHORT_AT_LIMIT(bar + 1, high(bar) * 1.05, 1000, "short entry");
  }
  virtual void run() {
    INSTALL_REVERSE_BREAK_EVEN_STOP(5);
    FOR_EACH_BAR(0);
  }
};

/** WL code test6
var t : integer;
t := getTickCount;
var Bar : integer;

InstallStopLoss ( 1 );

for Bar := 0 to barcount - 1 do
begin
ApplyAutoStops( Bar );
ShortAtLimit( Bar + 1, PriceHigh( Bar ), 'buy at limit' );
end;
print( 'duration: ' + IntToStr( getTickCount - t ) );
*/
class SystemTest6 : public BarSystem<SystemTest6> {
 public:
  SystemTest6(const std::vector<std::string>* params = 0)
      : BarSystem<SystemTest6>(
            Info("B6BC9CEF-B269-40fd-B830-466A5FCA281F", "Test 6", "")) {}

  virtual void onBar(INDEX bar) {
    APPLY_AUTO_STOPS(bar);
    SHORT_AT_LIMIT(bar + 1, high(bar) * 1.05, 1000, "short entry");
  }

  virtual void run() {
    INSTALL_STOP_LOSS(1);
    FOR_EACH_BAR(0);
  }
};

/** WL code test7
var t : integer;
t := getTickCount;
var Bar : integer;

InstallProfitTarget ( 1 );

for Bar := 0 to barcount - 1 do
begin
ApplyAutoStops( Bar );
ShortAtLimit( Bar + 1, PriceHigh( Bar ) * 1.05, 'buy at limit' );
end;
print( 'duration: ' + IntToStr( getTickCount - t ) );
*/
class SystemTest7 : public BarSystem<SystemTest7> {
 public:
  SystemTest7(const std::vector<std::string>* params = 0)
      : BarSystem<SystemTest7>(
            Info("F224CB98-75D7-4e0b-BE28-044FD5CE187A", "Test 7", "")) {}

  virtual void onBar(INDEX bar) {
    APPLY_AUTO_STOPS(bar);
    SHORT_AT_LIMIT(bar + 1, high(bar) * 1.05, 1000, "short entry");
  }

  virtual void run() {
    INSTALL_PROFIT_TARGET(1);
    FOR_EACH_BAR(0);
  }
};

/** WL code test8
var t : integer;
t := getTickCount;
var Bar : integer;

InstallTrailingStop( 1, 1 );

for Bar := 0 to barcount - 1 do
begin
ApplyAutoStops( Bar );
BuyAtLimit( Bar + 1, PriceLow( Bar ) * 0.95, 'buy at limit' );
end;
print( 'duration: ' + IntToStr( getTickCount - t ) );
*/
class SystemTest8 : public BarSystem<SystemTest8> {
 public:
  SystemTest8(const std::vector<std::string>* params = 0)
      : BarSystem<SystemTest8>(
            Info("ECCBA9C6-E305-45e3-A5B6-7BA93C2FC34A", "Test 8", "")) {}

  virtual void onBar(INDEX bar) {
    APPLY_AUTO_STOPS(bar);
    BUY_AT_LIMIT(bar + 1, high(bar) * 0.95, 1000, "long entry");
  }
  virtual void run() {
    INSTALL_TRAILING_STOP(1, 1);
    FOR_EACH_BAR(0);
  }
};

/** WL code test9
var t : integer;
t := getTickCount;
var Bar : integer;

InstallTrailingStop( 1, 1 );

for Bar := 0 to barcount - 1 do
begin
ApplyAutoStops( Bar );
ShortAtLimit( Bar + 1, PriceHigh( Bar ) * 1.05, 'buy at limit' );
end;
print( 'duration: ' + IntToStr( getTickCount - t ) );
*/
class SystemTest9 : public BarSystem<SystemTest9> {
 public:
  SystemTest9(const std::vector<std::string>* params = 0)
      : BarSystem<SystemTest9>(
            Info("F8C76524-230E-4179-93CE-17339517688B", "Test 9", "")) {}

  virtual void onBar(INDEX bar) {
    APPLY_AUTO_STOPS(bar);
    SHORT_AT_LIMIT(bar + 1, high(bar) * 1.05, 1000, "short entry");
  }
  virtual void run() {
    INSTALL_TRAILING_STOP(1, 1);
    FOR_EACH_BAR(0);
  }
};

/** WL code test10
var t : integer;
t := getTickCount;
var Bar : integer;

InstallBreakEvenStop( 1, 1 );

for Bar := 0 to barcount - 1 do
begin
ApplyAutoStops( Bar );
ShortAtLimit( Bar + 1, PriceHigh( Bar ) * 1.05, 'buy at limit' );
end;
print( 'duration: ' + IntToStr( getTickCount - t ) );
*/
class SystemTest10 : public BarSystem<SystemTest10> {
 public:
  SystemTest10(const std::vector<std::string>* params = 0)
      : BarSystem<SystemTest10>(
            Info("B4DDF2E5-BC72-4db2-9970-99BC81315381", "Test 10", "")) {}

  virtual void onBar(INDEX bar) {
    APPLY_AUTO_STOPS(bar);
    SHORT_AT_LIMIT(bar + 1, high(bar) * 1.05, 10, "short");
  }
  virtual void run() {
    INSTALL_BREAK_EVEN_STOP(1);
    FOR_EACH_BAR(0);
  }
};

// implements an order filter
class SystemTest11 : public BarSystem<SystemTest11> {
 public:
  SystemTest11(const std::vector<std::string>* params = 0)
      : BarSystem<SystemTest11>(
            Info("F12EE574-D22B-41d0-8E07-3CEF61CE296F", "Test 11", "")) {}

  virtual bool onShortAtLimit(size_t barIndex) const {
    return (barIndex % 2) == 0;
  }

  virtual void onBar(INDEX bar) {
    APPLY_AUTO_STOPS(bar);
    SHORT_AT_LIMIT(bar + 1, high(bar) * 1.05, 10, "short");
  }
  virtual void run() {
    positions().registerOrderFilter(this);

    INSTALL_BREAK_EVEN_STOP(1);
    FOR_EACH_BAR(0);
  }
};

class SystemTest12 : public BarSystem<SystemTest12> {
 public:
  SystemTest12(const std::vector<std::string>* params = 0)
      : BarSystem<SystemTest12>(
            Info("E755E7BE-DF37-4b32-A43B-8F71E6F35433", "Test 12", "")) {}

  virtual void onBar(INDEX bar) {
    APPLY_AUTO_STOPS(bar);
    SHORT_AT_LIMIT(bar + 1, high(bar) * 1.01, 10, "short");
  }
  virtual void run() {
    INSTALL_BREAK_EVEN_STOP(5);
    INSTALL_REVERSE_BREAK_EVEN_STOP(5);
    INSTALL_PROFIT_TARGET(5);
    INSTALL_TRAILING_STOP(1, 1);
    INSTALL_STOP_LOSS(5);
    INSTALL_TIME_BASED_EXIT(10);

    FOR_EACH_BAR(0);
  }
};

// WL code test13
/**
var t : integer;
t := getTickCount;
var Bar: integer;

for Bar := 0 to BarCount - 1 do
begin
var pos : integer;

for pos := 0 to positioncount - 1 do
if positionactive( pos ) then
sellAtLimit( bar, 25, pos, 'sell at limit' );

buyAtLimit( bar, PriceLow( bar ) * 0.95, 'buy at limit' );
end;
print( 'duration: ' + IntToStr( getTickCount - t ) );
*/
class SystemTest13 : public BarSystem<SystemTest13> {
 private:
  virtual bool onOpenPosition(Position pos, INDEX bar) {
    SELL_AT_LIMIT(bar + 1, pos, 25, "sell at limit");
    return true;
  }

 public:
  SystemTest13(const std::vector<std::string>* params = 0)
      : BarSystem<SystemTest13>(
            Info("5DB400F4-B6F4-4e68-BADD-27927A45E350", "Test 13", "")) {}

  virtual void onBar(INDEX bar) {
    FOR_EACH_OPEN_POSITION(bar);

    if ((bar > 0) && CLOSE_SERIES.crossOver(bar, OPEN_SERIES))
      BUY_AT_MARKET(bar + 1, 1000, "buy at limit");
  }
  virtual void run() {
    Series smahigh5 = HIGH_SERIES.SMA(5);
    Series smahigh10 = HIGH_SERIES.SMA(10);

    FOR_EACH_BAR(0);
  }
};

template <class T>
class SeriesSystemTest : public BarSystem<T> {
 private:
  Series _newSeries;

 protected:
  void setNewSeries(Series series) { _newSeries = series; }

 public:
  ~SeriesSystemTest() {}
  SeriesSystemTest(const Info& info) : BarSystem<T>(info) {}

  virtual const Series getSeries1() const = 0;
};

class SystemTest14 : public SeriesSystemTest<SystemTest14> {
 public:
  SystemTest14(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest14>(
            Info("964280B6-C58A-4242-8B3D-F693882A9B67",
                 "Test 14 - series addValue 10000", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(VOLUME_SERIES + 10000); }
};

class SystemTest15 : public SeriesSystemTest<SystemTest15> {
 public:
  SystemTest15(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest15>(
            Info("9553DFF6-4DD1-432a-8F10-B55CA01E29EE",
                 "Test 15 - series subtractValue 10000", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(VOLUME_SERIES - 10000); }
};

class SystemTest16 : public SeriesSystemTest<SystemTest16> {
 public:
  SystemTest16(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest16>(
            Info("1EDCDF21-C1B1-412f-8EFA-57CEB3826004",
                 "Test 16 - series multiplyValue 0.001", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(VOLUME_SERIES * 0.001); }
};

class SystemTest17 : public SeriesSystemTest<SystemTest17> {
 public:
  SystemTest17(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest17>(
            Info("19A06D34-9020-484a-B29A-D4057FEA2C06",
                 "Test 17 - series divideValue 1000", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(VOLUME_SERIES / 1000); }
};

class SystemTest18 : public SeriesSystemTest<SystemTest18> {
 public:
  SystemTest18(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest18>(
            Info("2BEFBE56-E49C-435e-8AD0-C3A5C860E261",
                 "Test 18 - EMA period 10", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(VOLUME_SERIES.EMA(10)); }
};

class SystemTest19 : public SeriesSystemTest<SystemTest19> {
 public:
  SystemTest19(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest19>(
            Info("6EE34B3F-5216-4217-8ADE-AAF7A9DB7A3A",
                 "Test 19 - SMA period 10", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(VOLUME_SERIES.SMA(10)); }
};

/*var s : integer;
var e : integer;
s := createseries;

@s[ 0 ] := 0;
@s[ 1 ] := 1;
@s[ 2 ] := 2;
@s[ 3 ] := 3;
@s[ 4 ] := 4;
@s[ 5 ] := 5;
@s[ 6 ] := 6;
@s[ 7 ] := 7;
@s[ 8 ] := 8;

e := wmaseries( #volume, 7 );

var n : integer;

for n := 0 to barcount - 1 do
print( FloatToStr( @e[ n ] ) );

{for n := 0 to barcount - 1 do
print( floattostr( @e[ n ] ) );
}*/
class SystemTest20 : public SeriesSystemTest<SystemTest20> {
 public:
  SystemTest20(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest20>(
            Info("48A5CD94-E149-4a74-ABD0-CCB04E7973A6",
                 "Test 20 - WMA period 10", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(VOLUME_SERIES.WMA(10)); }
};

class SystemTest21 : public SeriesSystemTest<SystemTest21> {
 public:
  SystemTest21(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest21>(
            Info("6F2AB737-0DCF-4a2e-BDE3-82C4E6E667EE", "Test 21 - Acum/Dist",
                 "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(bars().AccumDist()); }
};

class SystemTest22 : public SeriesSystemTest<SystemTest22> {
 public:
  SystemTest22(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest22>(
            Info("2938A0E3-726B-4398-A293-EE38E7A88933", "Test 22 - True Range",
                 "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(bars().TrueRange()); }
};

class SystemTest23 : public SeriesSystemTest<SystemTest23> {
 public:
  SystemTest23(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest23>(
            Info("E50BF301-2278-4922-99D8-15A104F1C000",
                 "Test 23 - Aroon down Series", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(VOLUME_SERIES.AroonDown(10)); }
};

class SystemTest24 : public SeriesSystemTest<SystemTest24> {
 public:
  SystemTest24(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest24>(
            Info("E85CFA64-257E-48c3-AC6C-54E43FADBC1D",
                 "Test 24 - Aroon up Series", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(VOLUME_SERIES.AroonUp(10)); }
};

class SystemTest25 : public BarSystem<SystemTest25> {
 private:
  virtual bool onOpenPosition(Position pos, INDEX bar) {
    COVER_AT_LIMIT(bar + 1, pos, open(bar) * 0.9, "sell at limit");
    return true;
  }

  virtual void onBar(INDEX bar) {
    FOR_EACH_OPEN_POSITION(bar);
    SHORT_AT_CLOSE(bar + 1, 1000, "short at close");
  }

 public:
  SystemTest25(const std::vector<std::string>* params = 0)
      : BarSystem<SystemTest25>(Info("42F41904-E22E-4d89-870B-7CE57702DE4E",
                                     "Test 25 - testing slippage", "")) {}

  virtual void run() { FOR_EACH_BAR(0); }
};

class SystemTest26 : public SeriesSystemTest<SystemTest26> {
 public:
  SystemTest26(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest26>(
            Info("DE2589D8-AA18-4eed-A9B2-D2D6F01B2C5D", "Test 26 - RSI Series",
                 "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(VOLUME_SERIES.RSI(10)); }
};

class SystemTest27 : public SeriesSystemTest<SystemTest27> {
 public:
  SystemTest27(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest27>(
            Info("9B921656-C743-448a-BDC0-CDF3A955F130", "Test 27 - ROC Series",
                 "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(VOLUME_SERIES.ROC(10)); }
};

class SystemTest28 : public SeriesSystemTest<SystemTest28> {
 public:
  SystemTest28(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest28>(Info(
            "F55922EA-2BA5-4b2d-9FD4-F4F9341C7089",
            "Test 28 - MACD fixed values (12/26) Series - 10 periods", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(VOLUME_SERIES.MACDFix(10)); }
};

class SystemTest29 : public SeriesSystemTest<SystemTest29> {
 public:
  SystemTest29(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest29>(
            Info("CCEE4ACC-6244-420c-80E0-6DD985AEDB87",
                 "Test 29 - Bollinger Band Upper Series - 10 periods, 1 "
                 "standard dev",
                 "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(VOLUME_SERIES.BBandUpper(10, 1)); }
};

class SystemTest30 : public SeriesSystemTest<SystemTest30> {
 public:
  SystemTest30(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest30>(
            Info("05AB6AEE-20CB-41a0-B6A4-4BA06EA334D6",
                 "Test 30 - Bollinger Band Lower Series - 10 periods, 1 "
                 "standard dev",
                 "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(VOLUME_SERIES.BBandLower(10, 1)); }
};

class SystemTest31 : public SeriesSystemTest<SystemTest31> {
 public:
  SystemTest31(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest31>(
            Info("67FDB6F3-9FBA-4cf7-A936-52B6990C877F",
                 "Test 31 - ADX Series - 10 periods", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(bars().ADX(10)); }
};

class SystemTest32 : public SeriesSystemTest<SystemTest32> {
 public:
  SystemTest32(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest32>(Info(
            "26F029D3-802B-4c3e-B2A4-EA49C0B95679",
            "Test 32 - divide series by series of different size error", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() {
    Series s(100);
    setNewSeries(VOLUME_SERIES / s);
  }
};

class SystemTest33 : public SeriesSystemTest<SystemTest33> {
 public:
  SystemTest33(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest33>(
            Info("FAE039D3-B8EA-4b52-8C65-7F3A3B73978B",
                 "Test 33 - index out of range error", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { VOLUME_SERIES[VOLUME_SERIES.size() + 2]; }
};

class SystemTest34 : public BarSystem<SystemTest34> {
 private:
  virtual bool onOpenPosition(tradery::Position pos, INDEX bar) {
    SELL_AT_LIMIT(bar + 1, pos, 25, "short exit");

    return true;
  }

  virtual void onBar(INDEX bar) {
    FOR_EACH_OPEN_POSITION(bar);
    SHORT_AT_LIMIT(bar + 1, high(bar) * 1.05, 1000, "short entry");
  }

 public:
  SystemTest34(const std::vector<std::string>* params = 0)
      : BarSystem<SystemTest34>(Info("0AEDA323-5DED-4346-B5E9-699F7FAB83A5",
                                     "Test 34 - selling short position error",
                                     "")) {}

  virtual void run() {
    /*    AlertHandlerTest at;
    positions().registerAlertHandler( &at );
    */
    FOR_EACH_BAR(0);
  }
};

class SystemTest35 : public BarSystem<SystemTest35> {
 private:
  virtual bool onOpenPosition(tradery::Position pos, INDEX bar) {
    COVER_AT_LIMIT(bar + 1, pos, 25, "sell at limit");

    return true;
  }

 public:
  SystemTest35(const std::vector<std::string>* params = 0)
      : BarSystem<SystemTest35>(Info("F18AEF2E-33A3-458c-8C13-B553635ED243",
                                     "Test 35 - covering long position error",
                                     "")) {}

  virtual void onBar(INDEX bar) {
    FOR_EACH_OPEN_POSITION(bar);
    BUY_AT_LIMIT(bar + 1, low(bar) * 1.05, 1000, "buy at limit");
  }
  virtual void run() { FOR_EACH_BAR(0); }
};

class SystemTest36 : public BarSystem<SystemTest36> {
 private:
  virtual bool onOpenPosition(tradery::Position pos, INDEX bar) {
    SELL_AT_LIMIT(bar + 1, pos, 25, "sell at limit");
    // closing a closed position
    SELL_AT_LIMIT(bar + 1, pos, 25, "sell at limit");

    return true;
  }

 public:
  SystemTest36(const std::vector<std::string>* params = 0)
      : BarSystem<SystemTest36>(
            Info("92F5C0A4-04DB-4962-BDE5-3BB1E6649DF4",
                 "Test 36 - closing already closed position error", "")) {}

  virtual void onBar(INDEX bar) {
    FOR_EACH_OPEN_POSITION(bar);
    BUY_AT_LIMIT(bar + 1, low(bar) * 1.05, 1000, "buy at limit");
  }
  virtual void run() { FOR_EACH_BAR(0); }
};

class SystemTest37 : public SeriesSystemTest<SystemTest37> {
 public:
  SystemTest37(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest37>(
            Info("C0B377E9-3466-425e-A5C6-E2D40B0E0207",
                 "Test 37 - series divide by 0 error", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(VOLUME_SERIES / 0.0); }
};

class SystemTest38 : public SeriesSystemTest<SystemTest38> {
 public:
  SystemTest38(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest38>(
            Info("C7C8ED3F-189D-4ebe-9FF6-ED04CD9B6850",
                 "Test 38 - DEMA - 10 periods", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(VOLUME_SERIES.DEMA(10)); }
};

class SystemTest39 : public SeriesSystemTest<SystemTest39> {
 public:
  SystemTest39(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest39>(Info(
            "1862FD14-C515-4ae0-BA98-BC276B5523CB",
            "Test 39 - Hilbert Transform - Instantaneous trendline", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(VOLUME_SERIES.HTTrendline()); }
};

class SystemTest40 : public SeriesSystemTest<SystemTest40> {
 public:
  SystemTest40(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest40>(
            Info("070E32A1-98D2-4de9-9C56-CD64E9FBFEC9",
                 "Test 40 - KAMA of highs - 10 periods", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(HIGH_SERIES.KAMA(10)); }
};

class SystemTest41 : public SeriesSystemTest<SystemTest41> {
 public:
  SystemTest41(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest41>(
            Info("B2169105-048F-4031-8647-8408FB707FAB",
                 "Test 41 - MAMA of highs - 0.5, 0.05", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(HIGH_SERIES.MAMA(0.5, 0.05)); }
};

class SystemTest42 : public SeriesSystemTest<SystemTest42> {
 public:
  SystemTest42(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest42>(
            Info("044EB03C-1A85-4d5c-BAAB-B023C3F4970E",
                 "Test 42 - FAMA of highs - 0.5, 0.05", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(HIGH_SERIES.FAMA(0.5, 0.05)); }
};

class SystemTest43 : public SeriesSystemTest<SystemTest43> {
 public:
  SystemTest43(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest43>(
            Info("B9CB2048-FB26-44be-AAF4-3D456AEA70C7",
                 "Test 43 - MidPrice Series - 10 periods", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(bars().MidPrice(10)); }
};

class SystemTest44 : public SeriesSystemTest<SystemTest44> {
 public:
  SystemTest44(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest44>(
            Info("30AD5BCA-2ECB-419a-A880-FB9C977DAD06",
                 "Test 44 - Min Series - 10 periods", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(HIGH_SERIES.Min(10)); }
};

class SystemTest45 : public SeriesSystemTest<SystemTest45> {
 public:
  SystemTest45(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest45>(
            Info("3DDC366A-8268-4025-BCEB-00430FB2BA87",
                 "Test 45 - Max Series - 10 periods", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(HIGH_SERIES.Max(10)); }
};

class SystemTest46 : public SeriesSystemTest<SystemTest46> {
 public:
  SystemTest46(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest46>(
            Info("C3AF5284-45C4-4033-B5E0-5F53CEED9E68",
                 "Test 46 - MidPoint Series - 10 periods", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(HIGH_SERIES.MidPoint(10)); }
};

class SystemTest47 : public SeriesSystemTest<SystemTest47> {
 public:
  SystemTest47(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest47>(
            Info("FD5A2CCD-A084-4d23-B293-B74C5A4E340B",
                 "Test 47 - SAR - acc 1, max 1", "")) {}

  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(bars().SAR(1, 1)); }
};

class SystemTest48 : public SeriesSystemTest<SystemTest48> {
 public:
  SystemTest48(const std::vector<std::string>* params = 0)
      : SeriesSystemTest<SystemTest48>(
            Info("2A542031-96C8-4605-927C-18E2789226E8",
                 "Test 48 - MFI - period 10 ", "")) {}
  virtual const Series getSeries1() const { return VOLUME_SERIES; }

  virtual void run() { setNewSeries(bars().MFI(10)); }
};

class TestTickSystem : public TickSystem, public TickHandler {
 public:
  TestTickSystem()
      : TickSystem(Info("C9438BB3-AE3F-4167-8086-49AB13601AD0",
                        "Test tick system", "")) {}

  void onBar(const Ticks& bars, size_t index) {
    // the tick system receives the tick data here for each tick
  }

  void run() {
    // do for each tick
    defTicks()->forEach(*this);
  }
  virtual void cleanup() {}

  virtual bool init(const std::string& symbol) { return true; }
};

class SystemTest49 : public BarSystem<SystemTest49> {
 private:
  Series ma5;
  Series ma10;

 public:
  SystemTest49(const std::vector<std::string>* params = 0)
      : BarSystem<SystemTest49>(
            Info("2D1DB308-FD52-46be-9631-BDDA7A583B4A", "Test 48", "")) {}

  // this is called by the framework before running the system.
  // used to do one time initializations
  virtual bool init(const std::string& symbol) {
    ma5 = CLOSE_SERIES.EMA(5);
    ma10 = CLOSE_SERIES.EMA(10);

    // here you can install some exit conditions
    // such as:
    // 5% profit target
    // getDefPositions()->installProfitTarget( 5 );
    // or time based exit 2 bars
    // getDefPositions()->installTimeBasedExit( 2 );

    // signal that init was performed succesfully
    // a false return would signal some error and would stop the run
    return true;
  }

  // called for each bar by forEachBar
  virtual void onBar(INDEX bar) {
    if (ma5[bar - 1] < ma10[bar - 1])
      BUY_AT_MARKET(bar + 1, 90, "buy at market - test");
  }

  // it just calls forEachBar which in turn calls onBar for all the available
  // bars
  virtual void run() {
    // will call all bars starting at index 10 - we want to let EMA stabilize
    FOR_EACH_BAR(10);
  }
};
