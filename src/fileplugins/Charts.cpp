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
#include "Charts.h"


#if defined(HAS_CHART_DIR)

constexpr auto CHART_TITLE = "<*block,valign=absmiddle*>Equity Curve<*/*>";
constexpr auto CHART_WIDTH = 850;
constexpr auto CHART1_HEIGHT = 500;
constexpr auto LEFT_SPACE = 70;
constexpr auto RIGHT_SPACE = 3;
constexpr auto CHART1_TOP_SPACE = 0;
constexpr auto CHART1_BOTTOM_SPACE = 0;

constexpr auto CHART_PLOT_X = CHART_WIDTH - LEFT_SPACE - RIGHT_SPACE;
constexpr auto CHART1_PLOT_Y = CHART1_HEIGHT - CHART1_TOP_SPACE - CHART1_BOTTOM_SPACE;
constexpr auto CHART1_PLOT_BOTTOM = CHART1_HEIGHT - CHART1_BOTTOM_SPACE;
constexpr auto CHART_PLOT_RIGHT = CHART_WIDTH - RIGHT_SPACE;
constexpr auto CHART_PLOT_WIDTH = CHART_PLOT_RIGHT - LEFT_SPACE;
constexpr auto LEGEND_X = LEFT_SPACE;
constexpr auto LEGEND_Y = 0;
constexpr auto LEGEND_HEIGHT = 25;
constexpr auto EQUITY_START_COLOR = 0x8000FF00;
constexpr auto EQUITY_END_COLOR = 0x80FFFFFF;
constexpr auto LEGEND_TEXT_SHORT = "Short equity";
constexpr auto LEGEND_TEXT_LONG = "Long equity";
constexpr auto LEGEND_TEXT_CASH = "Cash";
constexpr auto LEGEND_TEXT_BH = "Buy & Hold equity";
constexpr auto LEGEND_TEXT_EQUITY = "Total equity";
constexpr auto LEGEND_TOTAL_DD = "Total";
constexpr auto LEGEND_SHORT_DD = "Short";
constexpr auto LEGEND_LONG_DD = "Long";
constexpr auto LEGEND_BH_DD = "Buy & Hold";
constexpr auto LEGEND_TOTAL_DD_DURATION = "Total";
constexpr auto LEGEND_LONG_DD_DURATION = "Long";
constexpr auto LEGEND_SHORT_DD_DURATION = "Short";
constexpr auto LEGEND_BH_DD_DURATION = "Buy & Hold";
constexpr auto X_LABEL_COUNT = 30;
constexpr auto X_LABEL_ANGLE = 45;
constexpr auto COPYRIGHT_TEXT = "© Tradery.com";
constexpr auto COPYRIGHT_X = CHART_PLOT_RIGHT - 80;
constexpr auto X_LABEL_GAP = 3;

constexpr auto STATS_X = CHART_PLOT_RIGHT - 200;
constexpr auto STATS_Y = LEGEND_Y + 10;

constexpr auto TREND_COLOR = 0X000000;
constexpr auto STATS_COLOR = 0X003F00;

constexpr auto CHART2_TOP_SPACE = 0;
constexpr auto CHART2_BOTTOM_SPACE = 0;
constexpr auto CHART2_HEIGHT = 150;
constexpr auto CHART2_PLOT_Y = CHART2_HEIGHT - CHART2_TOP_SPACE - CHART2_BOTTOM_SPACE;

constexpr auto CHART22_HEIGHT = CHART2_HEIGHT;

constexpr auto CHART3_HEIGHT = 220;
constexpr auto CHART3_TOP_SPACE = 0;
constexpr auto CHART3_BOTTOM_SPACE = 70;
constexpr auto CHART3_PLOT_Y = CHART3_HEIGHT - CHART3_TOP_SPACE - CHART3_BOTTOM_SPACE;

constexpr auto TITLE_X = LEFT_SPACE;
constexpr auto TITLE_Y = 0;
constexpr auto TITLE_WIDTH = CHART_WIDTH - LEFT_SPACE - RIGHT_SPACE;
constexpr auto TITLE_HEIGHT = 18;

constexpr auto TITLE_BACKGROUND_COLOR = 0Xc08080ff;

constexpr auto GAP = 0;
constexpr auto TITLE1_POS_Y = 0;
constexpr auto CHART1_POS_Y = TITLE_HEIGHT;

constexpr auto TITLE2_POS_Y = TITLE_HEIGHT + CHART1_HEIGHT + GAP;
constexpr auto CHART2_POS_Y = TITLE2_POS_Y + TITLE_HEIGHT;

constexpr auto TITLE22_POS_Y = CHART2_POS_Y + CHART2_HEIGHT + GAP;
constexpr auto CHART22_POS_Y = TITLE22_POS_Y + TITLE_HEIGHT;

constexpr auto TITLE3_POS_Y = CHART22_POS_Y + CHART22_HEIGHT + GAP;
constexpr auto CHART3_POS_Y = TITLE3_POS_Y + TITLE_HEIGHT;

constexpr auto SHORT_COLOR = 0x00c00000;
constexpr auto LONG_COLOR = 0x000000c0;
constexpr auto BH_COLOR = 0X00C000C0;
constexpr auto TOTAL_COLOR = 0x8080ff80;
constexpr auto CASH_COLOR = 0X00008000;

constexpr auto CHART1_BK_COLOR = 0x80fff0f0;
constexpr auto CHART21_BK_COLOR = 0xf0fff0;
constexpr auto CHART22_BK_COLOR = 0xf0ffff;
constexpr auto CHART3_BK_COLOR = 0xf0f0ff;

constexpr auto TITLE_TEXT_COLOR = 0xffffff;
#define TITLE_STYLE "<*font=bold,size=10,color=0X200000*>"

constexpr auto MULTICHART_HEIGHT = CHART3_POS_Y + CHART3_HEIGHT;

class Chart1 : public XYChart {
public:
  Chart1(const EquityCurve& ec, const EquityCurve& bhec, const XLabels& labels);
};

class Chart2 : public XYChart {
public:
  Chart2(const EquityCurve& em, const double* total, const double* longs, const double* shorts, const double* bh,
    const XLabels& labels, int bkcolor, const char* ytext, bool pct);
};

class Chart21 : public Chart2 {
public:
  Chart21(const EquityCurve& em, const double* total, const double* longs, const double* shorts, const double* bh, const XLabels& labels)
    : Chart2(em, total, longs, shorts, bh, labels, CHART21_BK_COLOR, "Drawdown pct", true) {}
};

class Chart22 : public Chart2 {
public:
  Chart22(const EquityCurve& em, const double* total, const double* longs, const double* shorts, const double* bh, const XLabels& labels)
    : Chart2(em, total, longs, shorts, bh, labels, CHART22_BK_COLOR, "Drawdown value", false) {}
};

class Chart3 : public XYChart {
public:
  Chart3(const EquityCurve& em, const DrawdownCurve& totalDC, const DrawdownCurve& longDC, const DrawdownCurve& shortDC,
    const DrawdownCurve& bhDC, const XLabels& labels);
};

class PriceChart : public FinanceChart {
public:
  PriceChart();
};


Chart1::Chart1(const EquityCurve& ec, const EquityCurve& bhec, const XLabels& labels)
  : XYChart(CHART_WIDTH, CHART1_HEIGHT, Chart::Transparent, Chart::Transparent, 0) {
  assert(ec.getSize() > 0);

  const double* total(ec.getTotal());
  const double* sh(ec.getShort());
  const double* lg(ec.getLong());
  const double* cash(ec.getCash());
  const double* bh(bhec.getTotal());

  // Set the plotarea at (50, 30) and of size 240 x 140 pixels. Use white
  // (0xffffff) background.
  PlotArea* pa = setPlotArea(LEFT_SPACE, CHART1_TOP_SPACE, CHART_PLOT_X, CHART1_PLOT_Y, CHART1_BK_COLOR, -1, -1, 0xcccccc, 0xcccccc);
  pa->setBackground("background.gif");

  xAxis()->setColors(0x000000, Chart::Transparent);

  // Add a legend box at (50, 185) (below of plot area) using horizontal
  // layout. Use 8 pts Arial font with Transparent background.
  LegendBox* legendBox = addLegend(80, 15, true, "", 8);
  legendBox->setReverse();
  legendBox->setBackground(Chart::Transparent, Chart::Transparent);

  // Add keys to the legend box to explain the color zones
  /*    legendBox->addKey( LEGEND_TEXT_SHORT, LONG_COLOR );
      legendBox->addKey( LEGEND_TEXT_LONG, SHORT_COLOR);
  */
  // Add a title box to the chart using 8 pts Arial Bold font, with yellow
  // (0xffff40) background and a black border (0x0)
  //    c->addTitle("Equity Curve", "arialbd.ttf",
  //    10)->setBackground(0xffff40, 0); c1.addTitle( CHART_TITLE, 0, 10,
  //    0xffffff )->setBackground(0xc00000, -1,
  //    Chart::softLighting(Chart::Right));

  // Set the y axis label format to US$nnnn

  TextBox* tb = addText(COPYRIGHT_X, CHART1_PLOT_Y - 20, COPYRIGHT_TEXT);

  std::ostringstream statsText;

  double start = total[0];
  double end = total[ec.getSize() - 1];
  double pctGain = total[0] != 0 ? (end - start) / start * 100 : 0;
  statsText << "Starting equity: " << std::fixed << std::setprecision(2) << start << std::endl;
  statsText << "Ending equity: " << std::fixed << std::setprecision(2) << total[ec.getSize() - 1];
  statsText << std::endl << "Gain: " << std::fixed << std::setprecision(2) << pctGain << " %";
  TextBox* stats = addText(STATS_X, STATS_Y, statsText.str().c_str());
  stats->setFontColor(STATS_COLOR);

  addTrendLayer(DoubleArray(total, (int)ec.getSize()), TREND_COLOR, "Trend Line")->setLineWidth(2);
  LineLayer* layer1 = addLineLayer();
  // Set the default line width to 2 pixels

  // Add the three data sets to the line layer. For demo purpose, we use a
  // dash line color for the last line
  layer1->addDataSet(DoubleArray(sh, (int)ec.size()), SHORT_COLOR, LEGEND_TEXT_SHORT);
  layer1->addDataSet(DoubleArray(lg, (int)ec.size()), LONG_COLOR, LEGEND_TEXT_LONG);
  //    layer1->addDataSet(DoubleArray( cash, (int)ec.size() ), CASH_COLOR,
  //    LEGEND_TEXT_CASH );
  DataSet* ds = layer1->addDataSet(DoubleArray(bh, (int)bhec.size()), dashLineColor(BH_COLOR, Chart::DotLine), LEGEND_TEXT_BH);

  AreaLayer* layer = addAreaLayer();
  layer->addDataSet(DoubleArray(total, (int)ec.getSize()), TOTAL_COLOR, LEGEND_TEXT_EQUITY);
  layer->setLineWidth(1);

  AreaLayer* layerCash = addAreaLayer();
  layerCash->addDataSet(DoubleArray(cash, (int)ec.getSize()), CASH_COLOR, LEGEND_TEXT_CASH);
  layerCash->setLineWidth(1);

  //    layer1->addDataSet(DoubleArray(cash, ec.size() ), 0x00ff00, "Cash");

  xAxis()->setLabels(StringArray(labels, (int)ec.getSize()))->setFontAngle(X_LABEL_ANGLE);

  xAxis()->setLabelOffset(-5);
  xAxis()->setLabelStep((int)(ec.getSize() / X_LABEL_COUNT));
  xAxis()->setLabelGap(X_LABEL_GAP);
  xAxis()->setLabelStep(2, 1);
  yAxis()->setTitle("Equity value");
}

Chart2::Chart2(const EquityCurve& em, const double* total, const double* longs, const double* shorts, const double* bh,
  const XLabels& labels, int bkcolor, const char* ytext, bool pct)
  : XYChart(CHART_WIDTH, CHART2_HEIGHT, Chart::Transparent, Chart::Transparent, 0) {
  setPlotArea(LEFT_SPACE, CHART2_TOP_SPACE, CHART_PLOT_X, CHART2_PLOT_Y, bkcolor, -1, -1, 0xcccccc, 0xcccccc);

  // Set the labels on the x axis.
  xAxis()->setLabels(StringArray(labels, (int)em.getSize()))->setFontAngle(X_LABEL_ANGLE);

  xAxis()->setLabelOffset(-5);
  xAxis()->setLabelStep((int)(em.getSize() / X_LABEL_COUNT));
  xAxis()->setLabelGap(X_LABEL_GAP);
  xAxis()->setLabelStep(2, 1);
  yAxis()->addZone(CHART2_PLOT_Y - 10, CHART2_PLOT_Y, 0x99ff99);
  yAxis()->setTitle(ytext);
  if (pct) {
    yAxis()->setLabelFormat("{value}%");
  }
  else {
    yAxis()->setLabelFormat("{value}");
  }

  LineLayer* layer1 = addLineLayer();
  // Set the default line width to 2 pixels

  // Add the three data sets to the line layer. For demo purpose, we use a
  // dash line color for the last line

  if (longs != 0) {
    layer1->addDataSet(DoubleArray(longs, (int)em.size()), LONG_COLOR, LEGEND_LONG_DD);
  }

  if (shorts != 0) {
    layer1->addDataSet(DoubleArray(shorts, (int)em.size()), SHORT_COLOR, LEGEND_SHORT_DD);
  }

  if (bh != 0) {
    layer1->addDataSet(DoubleArray(bh, (int)em.size()), dashLineColor(BH_COLOR, Chart::DotLine), LEGEND_BH_DD);
  }

  AreaLayer* layer = addAreaLayer();
  //    layer->setLineWidth( 0 );

  if (total != 0) {
    layer->addDataSet(DoubleArray(total, (int)em.size()), TOTAL_COLOR, LEGEND_TOTAL_DD);
  }

  LegendBox* legendBox = addLegend(80, 70, true, "", 8);
  legendBox->setReverse();
  legendBox->setBackground(Chart::Transparent, Chart::Transparent);
  TextBox* tb = addText(COPYRIGHT_X, CHART2_PLOT_Y - 20, COPYRIGHT_TEXT);
}

Chart3::Chart3(const EquityCurve& em, const DrawdownCurve& totalDC, const DrawdownCurve& longDC, const DrawdownCurve& shortDC,
  const DrawdownCurve& bhDC, const XLabels& labels)
  : XYChart(CHART_WIDTH, CHART3_HEIGHT, Chart::Transparent, Chart::Transparent, 0) {
  setPlotArea(LEFT_SPACE, CHART3_TOP_SPACE, CHART_PLOT_X, CHART3_PLOT_Y, CHART3_BK_COLOR, -1, -1, 0xcccccc, 0xcccccc);

  LineLayer* layer1 = addLineLayer();
  // Set the default line width to 2 pixels

  // Add the three data sets to the line layer. For demo purpose, we use a
  // dash line color for the last line

  if (longDC.getBarsArray() != 0) {
    layer1->addDataSet(DoubleArray(longDC.getBarsArray(), (int)em.size()), LONG_COLOR, LEGEND_LONG_DD_DURATION);
  }

  if (shortDC.getBarsArray() != 0) {
    layer1->addDataSet(DoubleArray(shortDC.getBarsArray(), (int)em.size()), SHORT_COLOR, LEGEND_SHORT_DD_DURATION);
  }

  if (bhDC.getBarsArray() != 0) {
    layer1->addDataSet(DoubleArray(bhDC.getBarsArray(), (int)em.size()), dashLineColor(BH_COLOR, Chart::DotLine), LEGEND_BH_DD_DURATION);
  }

  AreaLayer* layer = addAreaLayer();
  if (totalDC.getBarsArray() != 0) {
    layer->addDataSet(DoubleArray(totalDC.getBarsArray(), (int)em.size()), TOTAL_COLOR, LEGEND_TOTAL_DD_DURATION);
  }
  //    layer->setLineWidth( 0 );
  yAxis()->setLabelFormat("{value}");
  yAxis()->setTitle("Drawdown duration (days)");

  // Set the labels on the x axis.
  xAxis()->setLabels(StringArray(labels, (int)em.getSize()))->setFontAngle(X_LABEL_ANGLE);

  xAxis()->setLabelOffset(-5);
  xAxis()->setLabelStep((int)(em.getSize() / X_LABEL_COUNT));
  xAxis()->setLabelGap(X_LABEL_GAP);
  xAxis()->setLabelStep(2, 1);

  LegendBox* legendBox = addLegend(80, 15, true, "", 8);
  legendBox->setReverse();
  legendBox->setBackground(Chart::Transparent, Chart::Transparent);

  TextBox* tb = addText(COPYRIGHT_X, CHART3_TOP_SPACE + 7, COPYRIGHT_TEXT);
}

AllChart::AllChart(const EquityCurve& ec, const EquityCurve& bhec, const DrawdownCurve& totalDC, const DrawdownCurve& longDC,
  const DrawdownCurve& shortDC, const DrawdownCurve& bhDC, const std::string& fileNameBase)
  : MultiChart(CHART_WIDTH, MULTICHART_HEIGHT) {
  /*
  Set ChartDirector license code here
  */
  //    Chart::setLicenseCode("XXXX-XXXX-XXXX-XXXX-XXXX-XXXX");

  const XLabels labels(ec);

  Chart1 c1(ec, bhec, labels);
  Chart21 c21(ec, totalDC.getDDPercentArray(), longDC.getDDPercentArray(), shortDC.getDDPercentArray(), bhDC.getDDPercentArray(), labels);
  Chart22 c22(ec, totalDC.getDDArray(), longDC.getDDArray(), shortDC.getDDArray(), bhDC.getDDArray(), labels);
  Chart3 c3(ec, totalDC, longDC, shortDC, bhDC, labels);

  TextBox* title = addText(TITLE_X, TITLE1_POS_Y, TITLE_STYLE "Equity", 0, 10, TITLE_TEXT_COLOR, Center);
  title->setBackground(TITLE_BACKGROUND_COLOR, 0);
  title->setSize(TITLE_WIDTH, TITLE_HEIGHT);

  title = addText(TITLE_X, TITLE2_POS_Y, TITLE_STYLE "Drawdown pct", 0, 10, TITLE_TEXT_COLOR, Center);
  title->setBackground(TITLE_BACKGROUND_COLOR, 0);
  title->setSize(TITLE_WIDTH, TITLE_HEIGHT);

  title = addText(TITLE_X, TITLE22_POS_Y, TITLE_STYLE "Drawdown value", 0, 10, TITLE_TEXT_COLOR, Center);
  title->setBackground(TITLE_BACKGROUND_COLOR, 0);
  title->setSize(TITLE_WIDTH, TITLE_HEIGHT);

  title = addText(TITLE_X, TITLE3_POS_Y, TITLE_STYLE "Drawdown duration", 0, 10, TITLE_TEXT_COLOR, Center);
  title->setBackground(TITLE_BACKGROUND_COLOR, 0);
  title->setSize(TITLE_WIDTH, TITLE_HEIGHT);

  addChart(0, CHART1_POS_Y, &c1);
  addChart(0, CHART2_POS_Y, &c21);
  addChart(0, CHART22_POS_Y, &c22);
  addChart(0, CHART3_POS_Y, &c3);

  // output the chart
  makeChart((fileNameBase + ".png").c_str());
}

PriceChart::PriceChart()
  : FinanceChart(600)

{
  // Create a finance chart demo containing 100 days of data
  int noOfDays = 1000;

  // To compute moving averages starting from the first day, we need to get
  // extra data points before the first day
  int extraDays = 30;

  // In this exammple, we use a random number generator utility to simulate
  // the data. We set up the random table to create 6 cols x (noOfDays +
  // extraDays) rows, using 9 as the seed.
  std::shared_ptr< RanTable > rantable(std::make_shared< RanTable >(9, 6, noOfDays + extraDays));

  // Set the 1st col to be the timeStamp, starting from Sep 4, 2002, with each
  // row representing one day, and counting week days only (jump over Sat and
  // Sun)
  rantable->setDateCol(0, chartTime(2002, 9, 4), 86400, true);

  // Set the 2nd, 3rd, 4th and 5th columns to be high, low, open and close
  // data. The open value starts from 100, and the daily change is random from
  // -5 to 5.
  rantable->setHLOCCols(1, 100, -5, 5);

  // Set the 6th column as the vol data from 5 to 25 million
  rantable->setCol(5, 50000000, 250000000);

  // Now we read the data from the table into arrays
  DoubleArray timeStamps = rantable->getCol(0);
  DoubleArray highData = rantable->getCol(1);
  DoubleArray lowData = rantable->getCol(2);
  DoubleArray openData = rantable->getCol(3);
  DoubleArray closeData = rantable->getCol(4);
  DoubleArray volData = rantable->getCol(5);

  // Create a FinanceChart object of width 600 pixels

  // Add a title to the chart
  addTitle("Finance Chart Demonstration");

  // Set the data into the finance chart object
  setData(timeStamps, highData, lowData, openData, closeData, volData,
    extraDays);

  // Add the main chart with 210 pixels in height
  addMainChart(210);

  // Add a 5 period simple moving average to the main chart, using brown color
  addSimpleMovingAvg(5, 0x663300);

  // Add a 20 period simple moving average to the main chart, using purple
  // color
  addSimpleMovingAvg(20, 0x9900ff);

  // Add an HLOC symbols to the main chart, using green/red for up/down days
  addHLOC(0x008000, 0xcc0000);

  // Add 20 days bollinger band to the main chart, using light blue (9999ff)
  // as the border and semi-transparent blue (c06666ff) as the fill color
  addBollingerBand(20, 2, 0x9999ff, 0xC06666FF);

  // Add a 70 pixels volume bars sub-chart to the bottom of the main chart,
  // using green/red/grey for up/down/flat days
  addVolBars(70, 0x99ff99, 0xff9999, 0x808080);

  // Append a 14-days RSI indicator chart (70 pixels height) after the main
  // chart. The main RSI line is purple (800080). Set threshold region to +/-
  // 20 (that is, RSI = 50 +/- 25). The upper/lower threshold regions will be
  // filled with red (ff0000)/blue (0000ff).
  addRSI(70, 14, 0x800080, 20, 0xff0000, 0x0000ff);

  // Append a 12-days momentum indicator chart (70 pixels height) using blue
  // (0000ff) color.
  addMomentum(70, 12, 0x0000ff);

  // output the chart
  makeChart("c:\\finance.png");
}

#endif
