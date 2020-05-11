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

#include "Configuration.h"

class RunnablePluginBuilderException : public std::exception{
public:
  RunnablePluginBuilderException( const std::string& message ) :
    std::exception( message.c_str() ){}
};

class RunnablePluginBuilder {
 private:
  DWORD _exitCode;

 public:
  RunnablePluginBuilder(const Configuration& config, bool& _cancel);

  bool operator!() const { return _exitCode != 0; }

  bool success() const { return _exitCode == 0; }

  DWORD exitCode() const { return _exitCode; }
};
