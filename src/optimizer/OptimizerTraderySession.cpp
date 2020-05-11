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

#include "StdAfx.h"
#include "OptimizerTraderySession.h"

OptimizerTraderySession::~OptimizerTraderySession(void) {}

char* PerformanceStats::StatsX[] = {TRADERY_SCORE,    TOTAL_PCT_GAIN_LOSS,
                                    TOTAL_GAIN_LOSS,  ANNUALIZED_PCT_GAIN,
                                    EXPOSURE,         EXPECTANCY,
                                    ULCER_INDEX,      MAX_DRAWDOWN,
                                    MAX_DRAWDOWN_PCT, MAX_DRAWDOWN_DURATION};

double PerformanceStats::getStatValue(size_t index,
                                      StatsValue::ValueType type) const {
  if (index < sizeof(StatsX) / sizeof(char*))
    return getValue(StatsX[index], type);
  else
    throw StatsException();
}
