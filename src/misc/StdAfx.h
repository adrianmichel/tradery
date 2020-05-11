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

// Insert your headers here
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <process.h>

#pragma warning(disable : 4800 4275 4251 4244 4003)
#pragma warning(push)

#include <boost\date_time\posix_time\ptime.hpp>
#include <boost\date_time\posix_time\posix_time.hpp>
#include <boost\date_time\time_clock.hpp>
#include <boost\timer.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

//#define BOOST_THREAD_USE_LIB
//#define BOOST_ALL_NO_LIB
#include <boost\thread.hpp>
//#undef BOOST_THREAD_USE_LIB
//#undef BOOST_ALL_NO_LIB
#pragma warning(pop)

#include <tokenizer.h>

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before
// the previous line.
