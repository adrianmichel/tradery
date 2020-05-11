/*
Copyright (C) 2018 Adrian Michel
http://www.amichel.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

void run() {
  installTimeBasedExit(2);

  Series SMAclose2 = closeSeries().SMA(2);
  Series SMAclose5 = closeSeries().SMA(5);
  Series SMAclose30 = closeSeries().SMA(30);
  double lc = 0.93;
  double sc = 0.93;

  for (Index bar = 1; bar < barsCount(); bar++) {
    //	  DebugBreak();
    applyAutoStops(bar);

    for (Position pos = getFirstOpenPosition(); pos;
         pos = getNextOpenPosition()) {
      double posPrice = pos.getEntryPrice();

      if (open(bar + 1) > posPrice AND pos.isLong()) {
        sellAtMarket(bar + 1, pos, "Sell at Market");
      }

      if (open(bar + 1) < posPrice AND pos.isShort()) {
        coverAtMarket(bar + 1, pos, "Cover at Market");
      }
    }

    //	DebugBreak();
    double d = SMAclose5[bar];
    double x = SMAclose30[bar];

    if (SMAclose5[bar] < lc * SMAclose30[bar]) {
      for (Index j = 1; j < 31; j++) {
        double lf = 0.75 - (j * 0.005);
        double lp = SMAclose2[bar] * lf;
        buyAtLimit(bar + 1, lp, 1000, "BuyAtLimit");
      }
    }

    if (SMAclose5[bar] > sc * SMAclose30[bar]) {
      for (Index j = 1; j < 31; j++) {
        double sf = 1.25 + (j * 0.005);
        double sp = SMAclose2[bar] * sf;
        shortAtLimit(bar + 1, sp, 1000, "ShortAtLimit");
      }
    }
  }
}
