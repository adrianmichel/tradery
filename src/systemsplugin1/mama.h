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

class MAMA_ttrcrep : public BarSystem<MAMA_ttrcrep> {
 public:
  //  constructor of the System1 class
  // does one time initialization of the class, which whill hold for the life of
  // the class, indicated by the "const" qualifier associated with the
  // variables. Per run intialization should be done in the method "init".
  MAMA_ttrcrep(const std::vector<std::string>* params = 0)
      // set the system name and the holding period to 2 days, profit target to
      // 5%
      : BarSystem<MAMA_ttrcrep>(
            Info("F7057B23-CBF6-4500-8166-8E1819CA306F", "MAMA_ttrcrep - ", ""))

  {}

 public:
  ~MAMA_ttrcrep(void) {}

  virtual void run() {
    double fastLimit = 0.4;
    double slowLimit = 0.05;

    Series priceSeries = (highSeries() + lowSeries()) / 2;

    double smooth = 0;
    double detrender = 0;
    double i1 = 0;
    double q1 = 0;
    double j1 = 0;
    double jq = 0;
    double i2 = 0;
    double q2 = 0;
    double re = 0;
    double im = 0;
    double x = 0;
    double period = 0;
    double smoothPeriod = 0;
    double phase = 0;
    double deltaPhase = 0;
    double alpha = 0;
    double mama = 0;
    double fama = 0;
    double period1 = 0;

    Series smoothSeries(size());
    Series periodSeries(size());
    Series detrenderSeries(size());
    Series phaseSeries(size());
    Series mamaSeries(size());
    Series famaSeries(size());
    Series i1Series(size());
    Series q1Series(size());
    Series i2Series(size());
    Series q2Series(size());
    Series reSeries(size());
    Series imSeries(size());

    for (Index bar = 6; bar < size() - 1; bar++) {
      double price = priceSeries[bar];
      double price1 = priceSeries[bar - 1];
      double price2 = priceSeries[bar - 2];
      double price3 = priceSeries[bar - 3];
      smooth = (4 * price + 3 * price1 + 2 * price2 + price3 / 10);
      smoothSeries[bar] = smooth;
      double smooth2 = smoothSeries[bar - 2];
      double smooth4 = smoothSeries[bar - 4];
      double smooth6 = smoothSeries[bar - 6];
      period1 = periodSeries[bar - 1];
      detrender = (0.0962 * smooth + 0.5769 * smooth2 - 0.5769 * smooth4 -
                   0.0962 * smooth6) *
                  (0.075 * period1 + 0.54);
      detrenderSeries[bar] = detrender;
      double detrender2 = detrenderSeries[bar - 2];
      double detrender3 = detrenderSeries[bar - 3];
      double detrender4 = detrenderSeries[bar - 4];
      double detrender6 = detrenderSeries[bar - 6];
      period1 = periodSeries[bar - 1];

      q1 = (0.0962 * detrender + 0.5769 * detrender2 - 0.5769 * detrender4 -
            0.0962 * detrender6) *
           (0.075 * period1 + 0.54);
      i1 = detrender3;
      i1Series[bar] = i1;
      q1Series[bar] = q1;

      double i12 = i1Series[bar - 2];
      double i14 = i1Series[bar - 4];
      double i16 = i1Series[bar - 6];

      q1 = q1Series[bar];
      double q12 = q1Series[bar - 2];
      double q14 = q1Series[bar - 4];
      double q16 = q1Series[bar - 6];

      period1 = periodSeries[bar - 1];

      double ji = (0.0962 * i1 + 0.5769 * i12 - 0.5769 * i14 - 0.0962 * i16) *
                  (0.075 * period1 + 0.54);
      jq = (0.0962 * q1 + 0.5769 * q12 - 0.5769 * q14 - 0.0962 * q16) *
           (0.075 * period1 + 0.54);

      i2 = i1 - jq;
      q2 = q1 + ji;

      i2Series[bar] = i2;
      q2Series[bar] = q2;

      i2 = i2Series[bar];
      double i21 = i2Series[bar - 1];

      q2 = q2Series[bar];
      double q21 = q2Series[bar - 1];

      i2 = 0.2 * i2 + 0.8 * i21;
      q2 = 0.2 * q2 + 0.8 * q21;

      re = i2 * i21 + q2 * q21;
      im = i2 * q21 - q2 * i21;

      reSeries[bar] = re;
      imSeries[bar] = im;
    }

    for (Index bar = 6; bar < size(); bar++) {
      re = reSeries[bar];
      double re1 = reSeries[bar - 1];

      im = imSeries[bar];
      double im1 = imSeries[bar - 1];

      period1 = periodSeries[bar - 1];

      re = 0.2 * re + 0.8 * re1;
      im = 0.2 * im + 0.8 * im1;
      reSeries[bar] = re;
      imSeries[bar] = im;

      if (im != 0 AND re != 0) {
        x = (im / re);
        double atValue = atan(x);
        // if bar > barCount() - 10 then print (FloatToStr(x) + ' ' +
        // FloatToStr(atValue));}
        period = 360 / atValue;
      }

      if (period > 1.5 * period1) period = 1.5 * period1;
      if (period < 0.67 * period1) period = 0.67 * period1;
      if (period < 6.0) period = 6.0;
      if (period > 50.0) period = 50.0;
      period = 0.2 * period + 0.8 * period1;
      periodSeries[bar] = period;

      i1 = i1Series[bar - 1];
      q1 = q1Series[bar - 1];
      if (i1 != 0) phase = atan(q1 / i1);
      phaseSeries[bar] = phase;

      phase = phaseSeries[bar];
      double phase1 = phaseSeries[bar - 1];
      double price = priceSeries[bar];
      double deltaphase = phase1 - phase;
      if (deltaphase < 1.0) deltaphase = 1.0;
      alpha = fastLimit / deltaphase;
      if (alpha < slowLimit) alpha = slowLimit;
      if (alpha > fastLimit) alpha = fastLimit;

      double mama1 = mamaSeries[bar - 1];
      double fama1 = famaSeries[bar - 1];
      mama = alpha * price + (1 - alpha) * mama1;
      mamaSeries[bar] = mama;
      fama = 0.5 * alpha * mama + (1 - 0.5 * alpha) * fama1;
      famaSeries[bar] = fama;
    }

    // PlotSeries( famaSeries, 0, #Blue, 0 );
    // PlotSeries( mamaSeries, 0, #fuchsia, 0 );

    for (Index bar = 7; bar < size(); bar++) {
      mama = mamaSeries[bar];
      double mama1 = mamaSeries[bar - 1];
      fama = famaSeries[bar];
      double fama1 = famaSeries[bar - 1];
      if (mama >= fama AND mama1 < fama1) {
        if (!hasOpenPositions())
          buyAtMarket(bar + 1, 1000, "");
        else
          coverAtMarket(bar + 1, getLastPosition(), "ShortExit");
      }

      if (mama < fama AND mama1 >= fama1) {
        if (!hasOpenPositions())
          shortAtMarket(bar + 1, 1000, "");
        else
          sellAtMarket(bar + 1, getLastPosition(), "LongExit");
      }
    }
  }
};

/*
void run()
{
        // set variables, series that will
        // be used in the bar loop to enter and exit positions

        // bar loop - for all bars from start index to the last index
        for( Index bar = start; bar < size(); bar++ )
        {
                // optional, in case any auto stops have been installed
                applyAutoStops( bar );

                // additional exit rules here
                // ...

                // entry rules - in this case
                // buy only if there are no open positions
                // if there are open positions, we know there is only
                // one (the last one), so close it (sellAtMarket)
        if (!hasOpenPositions() )
                        buyAtMarket(Bar + 1, "" );
            else
                    sellAtMarket(Bar + 1, LastPosition( ), "ShortExit");
        }
}
*/
