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

#include <synchronizer.h>
#include <stats.h>
#include <boost\shared_array.hpp>

// this implements a dynamic array of arrays, used by the chart library
// the base class is the array itself, and the shared_array is just to make sure
// they get deleted at the end
class XLabels : public std::vector<const char*> {
public:
  std::vector<boost::shared_array<char> > _v;

public:
  XLabels(const EquityCurve& em) {
    for (auto v : em) {
      std::string str(v.first.to_simple_string());
      char* p = new char[str.length() + 1];
      strcpy(p, str.c_str());
      _v.push_back(boost::shared_array<char>(p));
      __super::push_back(p);
    }
  }

public:
  operator const char* const* () const {
    return &(front());
  }

  const char* operator[](size_t index) {
    return std::vector<const char*>::operator[](index);
  }
};


#if __has_include( <chartdir.h>)
#define HAS_CHART_DIR
#include <chartdir.h>
#include <financechart.h>
#pragma comment( lib, "chartdir60.lib")

class AllChart : public MultiChart {
public:
  AllChart(const EquityCurve& ec, const EquityCurve& bhec, const DrawdownCurve& totalDC, const DrawdownCurve& longDC,
    const DrawdownCurve& shortDC, const DrawdownCurve& bhDC, const std::string& fileNameBase);
};

#else

class AllChart{
public:
  AllChart(const EquityCurve& ec, const EquityCurve& bhec, const DrawdownCurve& totalDC, const DrawdownCurve& longDC,
    const DrawdownCurve& shortDC, const DrawdownCurve& bhDC, const std::string& fileNameBase){}
};

#endif



