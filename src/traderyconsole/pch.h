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

#include <sstream>
#include <string>

#include <stdio.h>
#include <process.h>
#include <userenv.h>

// TODO: reference additional headers your program requires here
#include <vector>
#include <queue>
#include <ctime>
#include <cassert>
#include <iomanip>

#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>


#include <boost\static_assert.hpp>
#include <boost\type_traits.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/filter_view.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/vector.hpp>


#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/static_assert.hpp>


#include <boost\program_options\cmdline.hpp>
#include <boost/program_options/environment_iterator.hpp>
#include <boost/program_options/eof_iterator.hpp>
#include <boost/program_options/errors.hpp>
#include <boost/program_options/option.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/version.hpp>
#include <boost/algorithm/string.hpp>
#include <boost\regex.hpp>
#include <boost/make_shared.hpp>
#include <boost/filesystem.hpp>
#include <boost/type_index.hpp>

#include <iostream>

#include <nlohmann\json.hpp>


#include <misc.h>
#include <miscwin.h>
#include <miscfile.h>
#include <thread.h>
#include <core.h>
#include <common.h>
#include <versionno.h>
#include <stats.h>
#include <log.h>
#include <resourcewrapper.h>

#include <plugin.h>

//#include <Tradery.h>
#include "..\plugin\plugin.h"
#include "..\plugin\plugintree.h"

#include "session.h"
