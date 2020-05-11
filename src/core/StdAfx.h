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

#pragma warning(disable : 4786)
#pragma warning(disable : 4251)

// Insert your headers here
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers

#ifdef _UNICODE
#if !defined UNICODE
#define UNICODE
#endif
#endif  //_UNICODE

#ifdef UNICODE
#if !defined _UNICODE
#define _UNICODE
#endif
#endif  // UNICODE

#include <windows.h>
#include <process.h>
#include <typeinfo>
#include <iostream>
#include <vector>
#include <string>
#include <list>
#include <queue>
#include <assert.h>
#include <set>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <sys/timeb.h>
#include <iomanip>
#include <fcntl.h>
#include <io.h>
#include <functional>
#include <float.h>
#include <math.h>

// C includes
#include <assert.h>

#pragma warning(disable : 4786)
#pragma warning(disable : 4275)
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#pragma warning(default : 4275)

// TA-LIB includes
#include <ta_func.h>

#include <nlohmann/json.hpp>

#include <exception>
#include <core.h>
#include <common.h>
#include <misc.h>
#include <stats.h>

//{{AFX_INSERT_LOCATION}}
