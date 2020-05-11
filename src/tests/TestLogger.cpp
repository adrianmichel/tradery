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

#include "pch.h"
#include "TestLogger.h"

void TestLogger::log(Level level, const std::string& str)const {
  // first remove any eol characters from the end of the string, as Logger adds its own end of line
  // characters
  std::string s(str);

  while (!s.empty() && (s.back() == L'\n' || s.back() == L'\r')) {
    s.pop_back();
  }

  Microsoft::VisualStudio::CppUnitTestFramework::Logger::WriteMessage(s.c_str());
}
